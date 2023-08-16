#include "stdafx.h"
#include "MissedApproachAlarm.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include <time.h>

using namespace EuroScopePlugIn;

//Initialize static variables
RECT MissedApproachAlarm::m_Area = { 190, 523, 590, 863 };
RECT MissedApproachAlarm::b_Area = { 9, 7, 30, 30 };
RECT MissedApproachAlarm::c_Area = { 600, 900, 800, 1300 };
RECT MissedApproachAlarm::c_Area_Min = { 600, 900, 800, 980 };
RECT MissedApproachAlarm::i_Area = { 190, 523, 690, 823 };
RECT MissedApproachAlarm::i_Area_Min = { 190, 523, 690, 600 };
POINT MissedApproachAlarm::m_Offset = { 400, 340 };

int MissedApproachAlarm::ackButtonState = 0; //0 off, 1 flashing, 2 green ack
int MissedApproachAlarm::actButtonState = 0; //-1 disabled, 0 off, 1 act flashing, 2 on
int MissedApproachAlarm::resetButtonState = 0; //-1 red on (no ack), 0 off, 1 green on (ack received)
int MissedApproachAlarm::windowVisibility = 2; // 0 = hidden, 1 = minimised, 2 = full
vector<string> MissedApproachAlarm::missedAcftData = {};
vector<string> MissedApproachAlarm::activeMAPPRunways = {};
vector<string> MissedApproachAlarm::selectedAcftData = {};
string MissedApproachAlarm::ackStation = "???";

vector<string> MissedApproachPlugin::activeArrRunways = {};

MissedApproachAlarm::MissedApproachAlarm() {

}

MissedApproachAlarm::~MissedApproachAlarm() {
	try {
		this->OnAsrContentToBeSaved();
		//this->EuroScopePlugInExitCustom();
	}
	catch (exception& e) {
		stringstream s;
		s << e.what() << endl;	
	}
}

//---OnAsrContentLoaded--------------------------------------------

void MissedApproachAlarm::OnAsrContentLoaded(bool Loaded)
{
	const char* p_value;
	if ((p_value = GetDataFromAsr(string("AlarmTopLeftX").c_str())) != NULL) {
		m_Area.left = atoi(p_value);
		m_Area.right = m_Area.left + m_Offset.x;
	}
	if ((p_value = GetDataFromAsr(string("AlarmTopLeftY").c_str())) != NULL) {
		m_Area.top = atoi(p_value);
		m_Area.bottom = m_Area.top + m_Offset.y;
	}
}

//---OnAsrContentToBeSaved------------------------------------------

void MissedApproachAlarm::OnAsrContentToBeSaved()
{
	SaveDataToAsr(string("AlarmTopLeftX").c_str(), "Alarm position", to_string(m_Area.left).c_str());
	SaveDataToAsr(string("AlarmTopLeftY").c_str(), "Alarm position", to_string(m_Area.top).c_str());
}

//---OnRefresh------------------------------------------------------

void MissedApproachAlarm::OnRefresh(HDC hDC, int Phase)
{
	if (Phase != REFRESH_PHASE_AFTER_LISTS)
		return;

	MissedApproachPlugin ma;
	int position = ma.getPositionType();

	if (position <= 3) {
		//Logged in as GND or DEL
		activeMAPPRunways.clear();
		selectedAcftData.clear();
		missedAcftData.clear();
		return;
	}

	if (position == 4) {
		//Logged in as TWR

		drawIndicatorUnit(hDC);
		return;
	}

	//Logged in as APP or CTR
	drawConfigWindow(hDC);

	if (missedAcftData.size() != 0) {
		drawAlarmWindow(hDC);
	}

}

void MissedApproachAlarm::drawAlarmWindow(HDC hDC) {
	string callsignText = missedAcftData[0];
	string destText = missedAcftData[1];
	string rwyText = missedAcftData[2];

	// Basics
	CDC dc;
	dc.Attach(hDC);

	dc.SetTextColor(qTextColor);
	CBrush textBorderBrush(qTextColor);
	CBrush borderBrush(qBorderColor);
	CBrush backgroundBrush(qBackgroundColor);
	CBrush alrtButtonBrush(TAG_RED);

	CFont fontTitle, fontLabel;
	fontTitle.CreateFont(36, 0, 0, 0, 3, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabel.CreateFont(30, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));

	// Base Rect
	CRect windowAreaCRect(m_Area);
	windowAreaCRect.NormalizeRect();

	dc.FillSolidRect(windowAreaCRect, qBackgroundColor);
	dc.FrameRect(windowAreaCRect, &borderBrush);
	AddScreenObject(DRAWING_APPWINDOW, "window", windowAreaCRect, true, "");

	CFont* oldFont = dc.SelectObject(&fontTitle);
	CRect title(windowAreaCRect.left, windowAreaCRect.top + 20, windowAreaCRect.left + 400, windowAreaCRect.top + 60);
	dc.DrawText("MISSED APPROACH", strlen("MISSED APPROACH"), title, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	CFont* newFont = dc.SelectObject(&fontLabel);

	CRect runwayAirport(windowAreaCRect.left, windowAreaCRect.top + 285, windowAreaCRect.left + 400, windowAreaCRect.top + 317);
	string runwayAirportText = destText + " / " + rwyText;
	dc.DrawText(runwayAirportText.c_str(), runwayAirportText.length(), runwayAirport, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	CRect callsign(windowAreaCRect.left, windowAreaCRect.top + 208, windowAreaCRect.left + 400, windowAreaCRect.top + 265);
	dc.DrawText(callsignText.c_str(), callsignText.length(), callsign, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	dc.SetTextColor(BLACK);
	dc.SelectObject(&fontTitle);
	CRect ackButton(windowAreaCRect.left + 150, windowAreaCRect.top + 99, windowAreaCRect.left + 250, windowAreaCRect.top + 194);
	ackButton.NormalizeRect();

	if (ackButtonState == 1) {
		dc.FillSolidRect(ackButton, BUTTON_GREEN);
	}
	else {
		flashButton(hDC, ackButton);
	}
	dc.DrawText("ACK", strlen("ACK"), ackButton, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(ACK_BUTTON, "", ackButton, false, "");

	dc.SelectObject(oldFont);
	dc.Detach();
}

void MissedApproachAlarm::drawConfigWindow(HDC hDC) {
	if (windowVisibility == 0) {
		return;
	}
	vector<string> activeRunways = MissedApproachPlugin::activeArrRunways;

	CDC dc;
	dc.Attach(hDC);
	CFont fontTitle, fontLabel;
	fontTitle.CreateFont(28, 0, 0, 0, 3, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabel.CreateFont(24, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	dc.SetTextColor(qTextColor);


	if (windowVisibility == 1) {
		//Draw minimised window
		CRect configWindowRect(c_Area_Min);
		configWindowRect.NormalizeRect();
		dc.FillSolidRect(configWindowRect, qBackgroundColor);
		AddScreenObject(DRAWING_APPWINDOW, "config_window", configWindowRect, true, "");

		CRect titleRect(configWindowRect.left, configWindowRect.top + 20, configWindowRect.right, configWindowRect.top + 50);
		dc.SelectObject(&fontTitle);
		dc.DrawText("MAPP Config", strlen("MAPP Config"), titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		AddScreenObject(WINDOW_TITLE_BAR, "config_minimised", titleRect, true, "");

		dc.Detach();
		return;
	}

	//Draw full window
	CRect configWindowRect(c_Area);
	configWindowRect.NormalizeRect();
	dc.FillSolidRect(configWindowRect, qBackgroundColor);
	AddScreenObject(DRAWING_APPWINDOW, "config_window", configWindowRect, true, "");

	//Draw title
	CRect titleRect(configWindowRect.left, configWindowRect.top + 20, configWindowRect.right, configWindowRect.top + 50);
	dc.SelectObject(&fontTitle);
	dc.DrawText("MAPP Config", strlen("MAPP Config"), titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(WINDOW_TITLE_BAR, "config_full", titleRect, true, "");

	//Loop, iterate through arrival runways, create buttons and text for each one
	dc.SelectObject(&fontLabel);
	int offset = 0;
	for (auto& currentRunway : activeRunways) {
		CRect runwayText(configWindowRect.left + 60, configWindowRect.top + 80 + offset, configWindowRect.left + 100, configWindowRect.top + 110 + offset);
		dc.DrawText(currentRunway.c_str(), currentRunway.length(), runwayText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

		CRect checkBox(configWindowRect.left + 20, configWindowRect.top + 80 + offset, configWindowRect.left + 50, configWindowRect.top + 110 + offset);
		if (find(activeMAPPRunways.begin(), activeMAPPRunways.end(), currentRunway) != activeMAPPRunways.end()) {
			dc.FillSolidRect(checkBox, BUTTON_GREEN);
		}
		else {
			dc.FillSolidRect(checkBox, BUTTON_RED_ON);
		}
		string buttonNo = "runway_button_" + currentRunway;
		AddScreenObject(RWY_ENABLE_BUTTON, buttonNo.c_str(), checkBox, true, "");
		offset += 50;
	}
	dc.Detach();
}

void MissedApproachAlarm::drawIndicatorUnit(HDC hDC) {
	if (windowVisibility == 0) {
		return;
	}

	CDC dc;
	dc.Attach(hDC);
	MissedApproachPlugin ma;

	string ackLabel = "ACK";

	CFont fontTitle, fontLabel, fontLabelSmall;
	dc.SetTextColor(qTextColor);
	fontTitle.CreateFont(36, 0, 0, 0, 3, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabel.CreateFont(30, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabelSmall.CreateFont(24, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));

	if (windowVisibility == 1) {
		//Draw minimised window
		CRect indicatorWindowRect(i_Area_Min);
		indicatorWindowRect.NormalizeRect();
		dc.FillSolidRect(indicatorWindowRect, qBackgroundColor);
		AddScreenObject(DRAWING_APPWINDOW, "indicator_window", indicatorWindowRect, true, "");

		CRect titleRect(indicatorWindowRect.left, indicatorWindowRect.top + 20, indicatorWindowRect.right, indicatorWindowRect.top + 60);
		dc.SelectObject(&fontTitle);
		dc.DrawText("Missed Approach Light Indicator", strlen("Missed Approach Light Indicator"), titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		AddScreenObject(WINDOW_TITLE_BAR, "indicator_minimised", titleRect, true, "");
		dc.Detach();
		return;
	}
	//Draw Full Window
	CRect indicatorWindowRect(i_Area);
	indicatorWindowRect.NormalizeRect();
	dc.FillSolidRect(indicatorWindowRect, qBackgroundColor);
	AddScreenObject(DRAWING_APPWINDOW, "indicator_window", indicatorWindowRect, true, "");

	CRect titleRect(indicatorWindowRect.left, indicatorWindowRect.top + 20, indicatorWindowRect.right, indicatorWindowRect.top + 60);
	dc.SelectObject(&fontTitle);
	dc.DrawText("Missed Approach Light Indicator", strlen("Missed Approach Light Indicator"), titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(WINDOW_TITLE_BAR, "indicator_full", titleRect, true, "");

	//Draw ACT Button
	CRect buttonACT(indicatorWindowRect.left + 300, indicatorWindowRect.top + 100, indicatorWindowRect.left + 450, indicatorWindowRect.top + 200);
	CRect selectedAcftRect(indicatorWindowRect.left + 300, indicatorWindowRect.top + 200, indicatorWindowRect.left + 450, indicatorWindowRect.top + 300);
	string selectedAcftText;

	// Draw selected aircraft label below
	dc.SelectObject(&fontLabelSmall);
	if (actButtonState == 0 || actButtonState == -1) {
		dc.SetTextColor(qTextColor);
		dc.FillSolidRect(buttonACT, BUTTON_ORANGE_OFF);
		selectedAcftData = ma.getASELAircraftData();
		dc.SetTextColor(qTextColor);
		if (!selectedAcftData.empty() && ma.matchArrivalAirport(selectedAcftData[1].c_str())) {
			string selectedAcftText = "SEL: " + selectedAcftData[0] + " / " + selectedAcftData[2];
			dc.DrawText(selectedAcftText.c_str(), selectedAcftText.length(), selectedAcftRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			actButtonState = 0;
		}
		else {
			dc.DrawText("- / -", strlen("- / -"), selectedAcftRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			actButtonState = -1;
		}
	}
	else if (actButtonState == 1) {
		flashButton(hDC, buttonACT);
		dc.SetTextColor(BUTTON_ORANGE_ON);
		selectedAcftText = "ACT: " + selectedAcftData[0] + " / " + selectedAcftData[2];
		dc.DrawText(selectedAcftText.c_str(), selectedAcftText.length(), selectedAcftRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	}
	else {
		dc.SetTextColor(qTextColor);
		dc.FillSolidRect(buttonACT, BUTTON_ORANGE_ON);
		selectedAcftText = "ACK: " + selectedAcftData[0] + " / " + selectedAcftData[2];
		dc.DrawText(selectedAcftText.c_str(), selectedAcftText.length(), selectedAcftRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	}

	dc.SelectObject(&fontLabel);
	dc.SetTextColor(BLACK);
	dc.DrawText("ACT", strlen("ACT"), buttonACT, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(ACT_BUTTON, "act", buttonACT, true, "");

	//Draw ACK Reset Button
	CRect buttonReset(indicatorWindowRect.left + 50, indicatorWindowRect.top + 100, indicatorWindowRect.left + 150, indicatorWindowRect.top + 200);
	if (resetButtonState == 0) {
		dc.FillSolidRect(buttonReset, BUTTON_RED_OFF);
	}
	else if (resetButtonState == 1) {
		dc.FillSolidRect(buttonReset, BUTTON_GREEN);
		ackLabel.append(": ");
		ackLabel.append(ackStation);
	}
	else {
		dc.FillSolidRect(buttonReset, BUTTON_RED_ON);
		//Check for acknowledgement
		if (!selectedAcftData.empty() && ma.checkForAck(selectedAcftData[0].c_str()) != NULL) {
			actButtonState = 2;
			resetButtonState = 1;
			ackStation = ma.checkForAck(selectedAcftData[0].c_str());
		}
	}
	dc.SelectObject(&fontLabel);
	dc.SetTextColor(BLACK);
	dc.DrawText("RESET", strlen("RESET"), buttonReset, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(RESET_BUTTON, "reset", buttonReset, true, "");

	CRect ackText(indicatorWindowRect.left + 50, indicatorWindowRect.top + 200, indicatorWindowRect.left + 150, indicatorWindowRect.top + 300);
	dc.SelectObject(&fontLabelSmall);
	dc.SetTextColor(qTextColor);
	dc.DrawText(ackLabel.c_str(), ackLabel.length(), ackText, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	dc.Detach();
}

void MissedApproachAlarm::flashButton(HDC hDC, CRect button) {
	CDC dc;
	dc.Attach(hDC);

	// Play Sound
	char DllPathFile[_MAX_PATH];
	string pfad;
	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	pfad = DllPathFile;
	pfad.resize(pfad.size() - strlen("HKCP.dll"));
	pfad += "missed_alarm.wav";
	PlaySound(TEXT(pfad.c_str()), NULL, SND_NOSTOP | SND_FILENAME | SND_ASYNC);

	time_t rawtime = time(NULL);
	struct tm timeinfo;
	gmtime_s(&timeinfo, &rawtime);
	char buffer[3];
	strftime(buffer, 3, "%S", &timeinfo);
	int sec = atoi(buffer);

	if (sec % 2 == 0) {
		dc.FillSolidRect(button, BUTTON_ORANGE_ON);
	}
	else {
		dc.FillSolidRect(button, BUTTON_ORANGE_OFF);
	}
	dc.Detach();
}

void MissedApproachAlarm::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
	if (Button != BUTTON_LEFT) return;

	MissedApproachPlugin ma;
	if (ObjectType == ACK_BUTTON) {
		if (ackButtonState == 0) {
			ackButtonState = 1;
			ma.ackMissedApproach(missedAcftData[0].c_str());
		}
		else {
			missedAcftData.clear();
			ackButtonState = 0;
		}
	}

	if (ObjectType == ACT_BUTTON) {
		if (actButtonState == 0) {
			actButtonState = 1;
			resetButtonState = -1;
			ma.initMissedApproach(selectedAcftData[0].c_str());
		}
	}

	if (ObjectType == RESET_BUTTON) {
		if (resetButtonState == 1 || resetButtonState == -1) {
			resetButtonState = 0;
			ma.resetMissedApproach(selectedAcftData[0].c_str());
			actButtonState = 0;
		}
	}

	if (ObjectType == WINDOW_TITLE_BAR) {
		if (strstr(sObjectId, "minimised") != NULL) {
			windowVisibility = 2;
		}
		if (strstr(sObjectId, "full") != NULL) {
			windowVisibility = 1;
		}
	}

	//Handle buttons for enable/disabling other runways

	if (strstr(sObjectId, "runway_button_") != NULL) {
		vector<string> runways = MissedApproachPlugin::activeArrRunways;
		string currentRunway = (sObjectId + 14);
		auto it = find(activeMAPPRunways.begin(), activeMAPPRunways.end(), currentRunway);
		if (it != activeMAPPRunways.end()) {
			activeMAPPRunways.erase(it);
		}
		else {
			activeMAPPRunways.push_back(currentRunway);
		}
		
	}
}

void MissedApproachAlarm::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType) {
	if (DataType != CTR_DATA_TYPE_SCRATCH_PAD_STRING) return;
	MissedApproachPlugin ma;
	CFlightPlanData data = FlightPlan.GetFlightPlanData();
	CFlightPlanControllerAssignedData controllerData = FlightPlan.GetControllerAssignedData();

	//Filter from scratchpad message
	if (strcmp(controllerData.GetScratchPadString(), "MISAP_") != 0) return;

	//Handle tower case first
	if (!selectedAcftData.empty()) {
		if (ma.matchArrivalAirport(selectedAcftData[1].c_str())) {
			actButtonState = 1;
		}
		return;
	}
	//Don't add to vector unless runway is selected and active
	if (find(activeMAPPRunways.begin(), activeMAPPRunways.end(), data.GetArrivalRwy()) == activeMAPPRunways.end()) return;

	missedAcftData.push_back(FlightPlan.GetCallsign());
	missedAcftData.push_back(data.GetDestination());
	missedAcftData.push_back(data.GetArrivalRwy());
}

//---OnMoveScreenObject---------------------------------------------

void MissedApproachAlarm::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released) {
	if (strcmp(sObjectId, "window") == 0) {

		CRect appWindowRect(m_Area);
		appWindowRect.NormalizeRect();

		POINT TopLeft = { Area.left, Area.top };
		POINT BottomRight = { TopLeft.x + appWindowRect.Width(), TopLeft.y + appWindowRect.Height() };
		CRect newPos(TopLeft, BottomRight);
		newPos.NormalizeRect();

		m_Area = newPos;
	}
	if (strcmp(sObjectId, "indicator_window") == 0 || strcmp(sObjectId, "indicator_minimised") == 0 || strcmp(sObjectId, "indicator_full") == 0) {
		CRect appWindowRect(i_Area);
		appWindowRect.NormalizeRect();

		POINT TopLeft = { Area.left, Area.top };
		POINT BottomRight = { TopLeft.x + appWindowRect.Width(), TopLeft.y + appWindowRect.Height() };
		CRect newPos(TopLeft, BottomRight);
		newPos.NormalizeRect();
		
		CRect appWindowMinRect(i_Area_Min);
		BottomRight = { TopLeft.x + appWindowMinRect.Width(), TopLeft.y + appWindowMinRect.Height() };
		CRect newMinPos(TopLeft, BottomRight);
		newMinPos.NormalizeRect();

		i_Area = newPos;
		i_Area_Min = newMinPos;
	}
	if (strcmp(sObjectId, "config_window") == 0 || strcmp(sObjectId, "config_minimised") == 0 || strcmp(sObjectId, "config_full") == 0) {

		CRect configWindowRect(c_Area);
		configWindowRect.NormalizeRect();

		POINT TopLeft = { Area.left, Area.top };
		POINT BottomRight = { TopLeft.x + configWindowRect.Width(), TopLeft.y + configWindowRect.Height() };
		CRect newPos(TopLeft, BottomRight);
		newPos.NormalizeRect();

		CRect configWindowMinRect(c_Area_Min);
		BottomRight = { TopLeft.x + configWindowMinRect.Width(), TopLeft.y + configWindowMinRect.Height() };
		CRect newMinPos(TopLeft, BottomRight);
		newMinPos.NormalizeRect();

		c_Area = newPos;
		c_Area_Min = newMinPos;
	}
}

//---OnButtonDownScreenObject---------------------------------------------

void MissedApproachAlarm::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
}

//---OnButtonDownScreenObject---------------------------------------------

void MissedApproachAlarm::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {

}

bool MissedApproachAlarm::OnCompileCommand(const char* sCommandLine) {
	if (strcmp(sCommandLine, ".mappshow") == 0) {
		windowVisibility = 2;
		return true;
	}
	if (strcmp(sCommandLine, ".mapphide") == 0) {
		windowVisibility = 0;
		return true;
	}
	return false;
}

//PLUGIN Helper Functions

MissedApproachPlugin::MissedApproachPlugin(): CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT) {
	activeArrRunways = getArrivalRunways();
}

vector<string> MissedApproachPlugin::getArrivalRunways() {
	CSectorElement runway;
	vector<string> activeRunways;
	for (runway = SectorFileElementSelectFirst(SECTOR_ELEMENT_RUNWAY); runway.IsValid(); runway = SectorFileElementSelectNext(runway, SECTOR_ELEMENT_RUNWAY)) {
		if (runway.IsElementActive(false, 0)) {
			activeRunways.push_back(runway.GetRunwayName(0));
		}
		if (runway.IsElementActive(false, 1)) {
			activeRunways.push_back(runway.GetRunwayName(1));
		}
	}
	return activeRunways;
}    

void MissedApproachPlugin::initMissedApproach(const char * callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	if (!fpl.IsValid()) return;

	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();
	controllerData.SetScratchPadString("MISAP_");
}

void MissedApproachPlugin::ackMissedApproach(const char * callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CController myself = ControllerMyself();
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();

	string ackMessage = "MISAP_ACK_";
	ackMessage.append(myself.GetPositionId());
	if (strcmp(controllerData.GetScratchPadString(), "MISAP_") == 0) {
		controllerData.SetScratchPadString(ackMessage.c_str());
	}
	//couldn't find it, handle error
}

void MissedApproachPlugin::resetMissedApproach(const char* callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();
	if (strstr(controllerData.GetScratchPadString(), "MISAP_") != NULL) {
		controllerData.SetScratchPadString("");
	}
	//couldn't find it, handle error
}

void MissedApproachPlugin::OnAirportRunwayActivityChanged() {
	activeArrRunways = getArrivalRunways();
}

int MissedApproachPlugin::getPositionType() {
	CController myself = ControllerMyself();
	return myself.GetFacility();
}

vector<string> MissedApproachPlugin::getASELAircraftData() {
	vector<string> acftData = {};
	CFlightPlan fpl = FlightPlanSelectASEL();
	activeArrRunways = getArrivalRunways();
	CRadarTargetPositionData rdr = fpl.GetCorrelatedRadarTarget().GetPosition();
	if (!fpl.IsValid() || rdr.GetPressureAltitude() < 150 || fpl.GetTrackingControllerId() == "") {
		//return empty if FPL is invalid, on the ground, or tracked by another controller
		return acftData;
	}
	else {
		CFlightPlanData data = fpl.GetFlightPlanData();
		acftData.push_back(fpl.GetCallsign());
		acftData.push_back(data.GetDestination());
		acftData.push_back(data.GetArrivalRwy());
	}
	
	return acftData;
}

bool MissedApproachPlugin::matchArrivalAirport(const char* arrivalArpt) {
	const char * myself = ControllerMyself().GetCallsign();
	if (strstr(myself, arrivalArpt) == NULL) {
		return false;
	}
	return true;
}

const char * MissedApproachPlugin::checkForAck(const char* callsign) {
	CFlightPlanControllerAssignedData controllerData = FlightPlanSelect(callsign).GetControllerAssignedData();
	const char* ptr = strstr(controllerData.GetScratchPadString(), "MISAP_ACK_");
	if (ptr != NULL) {
		ptr = ptr + strlen("MISAP_ACK_");
		return (ptr != NULL && strlen(ptr) == 3) ? ptr : "???";
	}
	return NULL;
}
