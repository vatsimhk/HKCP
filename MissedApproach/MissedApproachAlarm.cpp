#include "stdafx.h"
#include "MissedApproachAlarm.hpp"
#include "MissedApproachPlugin.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include <time.h>

using namespace EuroScopePlugIn;

//Initialize static variables
RECT MissedApproachAlarm::m_Area = { 190, 500, 490, 740 };
RECT MissedApproachAlarm::c_Area = { 600, 900, 750, 1150 };
RECT MissedApproachAlarm::c_Area_Min = { 600, 900, 750, 960 };
RECT MissedApproachAlarm::i_Area = { 200, 500, 480, 650 };
RECT MissedApproachAlarm::i_Area_Min = { 200, 500, 480, 570 };
POINT MissedApproachAlarm::m_Offset = { 300, 240 };
POINT MissedApproachAlarm::c_Offset = { 150, 250 };
POINT MissedApproachAlarm::i_Offset = { 280, 150 };

int MissedApproachAlarm::ackButtonState = 0; //0 off, 1 flashing, 2 green ack
int MissedApproachAlarm::actButtonState = 0; //-1 disabled, 0 off, 1 act flashing, 2 on
int MissedApproachAlarm::actButtonHold = 50;
int MissedApproachAlarm::resetButtonState = 0; //-1 red on (no ack), 0 off, 1 green on (ack received)
int MissedApproachAlarm::windowVisibility = 2; // 0 = hidden, 1 = minimised, 2 = full
vector<string> MissedApproachAlarm::missedAcftData = {};
vector<string> MissedApproachAlarm::activeMAPPRunways = {};
vector<string> MissedApproachAlarm::selectedAcftData = {};
string MissedApproachAlarm::ackStation = "???";

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
	if ((p_value = GetDataFromAsr(string("ConfigTopLeftX").c_str())) != NULL) {
		c_Area.left = atoi(p_value);
		c_Area.right = c_Area.left + c_Offset.x;
	}
	if ((p_value = GetDataFromAsr(string("ConfigTopLeftY").c_str())) != NULL) {
		c_Area.top = atoi(p_value);
		c_Area.bottom = c_Area.top + c_Offset.y;
	}
	if ((p_value = GetDataFromAsr(string("IndicatorTopLeftX").c_str())) != NULL) {
		i_Area.left = atoi(p_value);
		i_Area.right = i_Area.left + i_Offset.x;
	}
	if ((p_value = GetDataFromAsr(string("IndicatorTopLeftY").c_str())) != NULL) {
		i_Area.top = atoi(p_value);
		i_Area.bottom = i_Area.top + i_Offset.y;
	}
}

//---OnAsrContentToBeSaved------------------------------------------

void MissedApproachAlarm::OnAsrContentToBeSaved()
{
	SaveDataToAsr(string("AlarmTopLeftX").c_str(), "Alarm position", to_string(m_Area.left).c_str());
	SaveDataToAsr(string("AlarmTopLeftY").c_str(), "Alarm position", to_string(m_Area.top).c_str());

	SaveDataToAsr(string("ConfigTopLeftX").c_str(), "Config position", to_string(c_Area.left).c_str());
	SaveDataToAsr(string("ConfigTopLeftY").c_str(), "Config position", to_string(c_Area.top).c_str());

	SaveDataToAsr(string("IndTopLeftX").c_str(), "Indicator position", to_string(i_Area.left).c_str());
	SaveDataToAsr(string("IndTopLeftY").c_str(), "Indicator position", to_string(i_Area.top).c_str());
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
	fontTitle.CreateFont(26, 0, 0, 0, 3, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabel.CreateFont(20, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));

	// Base Rect
	CRect windowAreaCRect(m_Area);
	windowAreaCRect.NormalizeRect();

	dc.FillSolidRect(windowAreaCRect, qBackgroundColor);
	dc.FrameRect(windowAreaCRect, &borderBrush);
	AddScreenObject(DRAWING_APPWINDOW, "window", windowAreaCRect, true, "");

	CFont* oldFont = dc.SelectObject(&fontTitle);
	CRect title(windowAreaCRect.left, windowAreaCRect.top + 20, windowAreaCRect.right, windowAreaCRect.top + 40);
	dc.DrawText("MISSED APPROACH", strlen("MISSED APPROACH"), title, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	CFont* newFont = dc.SelectObject(&fontLabel);

	CRect runwayAirport(windowAreaCRect.left, windowAreaCRect.top + 180, windowAreaCRect.right, windowAreaCRect.top + 200);
	string runwayAirportText = destText + " / " + rwyText;
	dc.DrawText(runwayAirportText.c_str(), runwayAirportText.length(), runwayAirport, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	CRect callsign(windowAreaCRect.left, windowAreaCRect.top + 200, windowAreaCRect.right, windowAreaCRect.top + 220);
	dc.DrawText(callsignText.c_str(), callsignText.length(), callsign, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	dc.SetTextColor(BLACK);
	dc.SelectObject(&fontTitle);
	CRect ackButton(windowAreaCRect.left + 100, windowAreaCRect.top + 60, windowAreaCRect.left + 200, windowAreaCRect.top + 160);
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
	fontTitle.CreateFont(24, 0, 0, 0, 3, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabel.CreateFont(18, 0, 0, 0, 0, false, false,
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
		CRect runwayText(configWindowRect.left + 60, configWindowRect.top + 70 + offset, configWindowRect.left + 100, configWindowRect.top + 90 + offset);
		dc.DrawText(currentRunway.c_str(), currentRunway.length(), runwayText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

		CRect checkBox(configWindowRect.left + 20, configWindowRect.top + 70 + offset, configWindowRect.left + 40, configWindowRect.top + 90 + offset);
		if (find(activeMAPPRunways.begin(), activeMAPPRunways.end(), currentRunway) != activeMAPPRunways.end()) {
			dc.FillSolidRect(checkBox, BUTTON_GREEN);
		}
		else {
			dc.FillSolidRect(checkBox, BUTTON_RED_ON);
		}
		string buttonNo = "runway_button_" + currentRunway;
		AddScreenObject(RWY_ENABLE_BUTTON, buttonNo.c_str(), checkBox, true, "");
		offset += 30;
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
	fontTitle.CreateFont(24, 0, 0, 0, 3, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabel.CreateFont(18, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontLabelSmall.CreateFont(16, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));

	if (windowVisibility == 1) {
		//Draw minimised window
		CRect indicatorWindowRect(i_Area_Min);
		indicatorWindowRect.NormalizeRect();
		dc.FillSolidRect(indicatorWindowRect, qBackgroundColor);
		AddScreenObject(DRAWING_APPWINDOW, "indicator_window", indicatorWindowRect, true, "");

		CRect titleRect(indicatorWindowRect.left, indicatorWindowRect.top + 15, indicatorWindowRect.right, indicatorWindowRect.top + 40);
		dc.SelectObject(&fontTitle);
		dc.DrawText("Missed Approach Indicator", strlen("Missed Approach Indicator"), titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		AddScreenObject(WINDOW_TITLE_BAR, "indicator_minimised", titleRect, true, "");
		dc.Detach();
		return;
	}
	//Draw Full Window
	CRect indicatorWindowRect(i_Area);
	indicatorWindowRect.NormalizeRect();
	dc.FillSolidRect(indicatorWindowRect, qBackgroundColor);
	AddScreenObject(DRAWING_APPWINDOW, "indicator_window", indicatorWindowRect, true, "");

	CRect titleRect(indicatorWindowRect.left, indicatorWindowRect.top + 15, indicatorWindowRect.right, indicatorWindowRect.top + 40);
	dc.SelectObject(&fontTitle);
	dc.DrawText("Missed Approach Indicator", strlen("Missed Approach Indicator"), titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(WINDOW_TITLE_BAR, "indicator_full", titleRect, true, "");

	//Draw ACT Button
	CRect buttonACT(indicatorWindowRect.left + 170, indicatorWindowRect.top + 60, indicatorWindowRect.left + 245, indicatorWindowRect.top + 110);
	CRect selectedAcftRect(indicatorWindowRect.left + 170, indicatorWindowRect.top + 120, indicatorWindowRect.left + 245, indicatorWindowRect.top + 140);
	string selectedAcftText;

	// Draw selected aircraft label below
	dc.SelectObject(&fontLabelSmall);
	if (actButtonState == 0 || actButtonState == -1) {
		dc.SetTextColor(qTextColor);
		selectedAcftData = ma.getASELAircraftData();
		dc.SetTextColor(qTextColor);
		if (!selectedAcftData.empty() && ma.matchArrivalAirport(selectedAcftData[1].c_str())) {
			string selectedAcftText = "SEL: " + selectedAcftData[0] + " / " + selectedAcftData[2];
			dc.DrawText(selectedAcftText.c_str(), selectedAcftText.length(), selectedAcftRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			dc.FillSolidRect(buttonACT, BUTTON_ORANGE_OFF);

			if (actButtonHold < 50) {
				CRect buttonActPartial(indicatorWindowRect.left + 170, indicatorWindowRect.top + 60 + actButtonHold, indicatorWindowRect.left + 245, indicatorWindowRect.top + 110);
				dc.FillSolidRect(buttonActPartial, BUTTON_ORANGE_ON);
				actButtonHold = actButtonHold > 0 ? actButtonHold-1 : 0;
				RequestRefresh();
			}
			actButtonState = 0;
		}
		else {
			dc.DrawText("- / -", strlen("- / -"), selectedAcftRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			dc.FillSolidRect(buttonACT, BUTTON_GREY);
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
	CRect buttonReset(indicatorWindowRect.left + 35, indicatorWindowRect.top + 60, indicatorWindowRect.left + 85, indicatorWindowRect.top + 110);
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
	dc.SelectObject(&fontLabelSmall);
	dc.SetTextColor(BLACK);
	dc.DrawText("RESET", strlen("RESET"), buttonReset, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(RESET_BUTTON, "reset", buttonReset, true, "");

	CRect ackText(indicatorWindowRect.left + 35, indicatorWindowRect.top + 120, indicatorWindowRect.left + 85, indicatorWindowRect.top + 140);
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
		return;
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
			resetButtonState = -1;
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
	if (Button != BUTTON_LEFT) {
		return;
	}
	if (ObjectType == ACT_BUTTON && actButtonHold > 0) {
		actButtonHold--;
	}
}

//---OnButtonDownScreenObject---------------------------------------------

void MissedApproachAlarm::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
	if (Button != BUTTON_LEFT) {
		return;
	}

	if (ObjectType == ACT_BUTTON) {
		if (actButtonHold == 0 && actButtonState == 0) {
			actButtonState = 1;
			resetButtonState = -1;
			MissedApproachPlugin ma;
			ma.initMissedApproach(selectedAcftData[0].c_str());
		}
		actButtonHold = 50;
	}
}

void MissedApproachAlarm::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
	if (Button != BUTTON_LEFT) {
		return;
	}

	if (ObjectType == ACT_BUTTON) {
		if (actButtonState == 0) {
			actButtonState = 1;
			resetButtonState = -1;
			MissedApproachPlugin ma;
			ma.initMissedApproach(selectedAcftData[0].c_str());
		}
		actButtonHold = 50;
	}
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
