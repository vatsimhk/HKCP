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
POINT MissedApproachAlarm::m_Offset = { 400, 340 };

int MissedApproachAlarm::ackButtonState = 0;
int MissedApproachAlarm::configWindowState = 2; // 0 = hidden, 1 = minimised, 2 = full
vector<string> MissedApproachAlarm::missedAcftData = {};
vector<string> MissedApproachAlarm::activeMAPPRunways = {};

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
		return;
	}

	if (position == 4) {
		//Logged in as TWR

		//TODO: draw tower window

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
	CRect northACK(windowAreaCRect.left + 150, windowAreaCRect.top + 99, windowAreaCRect.left + 250, windowAreaCRect.top + 194);
	northACK.NormalizeRect();

	flashButton(hDC, northACK);
	dc.DrawText("ACK", strlen("ACK"), northACK, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	AddScreenObject(ACK_BUTTON, "", northACK, false, "");

	dc.SelectObject(oldFont);
	dc.Detach();
}

void MissedApproachAlarm::drawConfigWindow(HDC hDC) {
	if (configWindowState == 0) {
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


	if (configWindowState == 1) {
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
			dc.FillSolidRect(checkBox, BUTTON_RED);
		}
		string buttonNo = "runway_button_" + currentRunway;
		AddScreenObject(RWY_ENABLE_BUTTON, buttonNo.c_str(), checkBox, true, "");
		offset += 50;
	}
	dc.Detach();
}

void MissedApproachAlarm::flashButton(HDC hDC, CRect button) {
	CDC dc;
	dc.Attach(hDC);
	if (ackButtonState == 1) {
		dc.FillSolidRect(button, BUTTON_GREEN);
		dc.Detach();
		return;
	}

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
		dc.FillSolidRect(button, ACK_BUTTON_ON);
	}
	else {
		dc.FillSolidRect(button, ACK_BUTTON_OFF);
	}
	dc.Detach();
}

void MissedApproachAlarm::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
	if (Button != BUTTON_LEFT) return;

	MissedApproachPlugin ma;
	if (ObjectType == ACK_BUTTON) {
		if (ackButtonState == 0) {
			ackButtonState = 1;
		}
		else {
			ma.ackMissedApproach(missedAcftData[0].c_str());
			missedAcftData.clear();
			ackButtonState = 0;
		}
	}

	if (ObjectType == WINDOW_TITLE_BAR) {
		if (strcmp(sObjectId, "config_minimised") == 0) {
			configWindowState = 2;
		}
		if (strcmp(sObjectId, "config_full") == 0) {
			configWindowState = 1;
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
	CFlightPlanData data = FlightPlan.GetFlightPlanData();
	CFlightPlanControllerAssignedData controllerData = FlightPlan.GetControllerAssignedData();

	if (strcmp(controllerData.GetScratchPadString(), "MISAP_") != 0) return;

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
		configWindowState = 2;
		return true;
	}
	if (strcmp(sCommandLine, ".mapphide") == 0) {
		configWindowState = 0;
		return true;
	}
	return false;
}

//PLUGIN BACKEND (Logic)

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

void MissedApproachPlugin::ackMissedApproach(const char * callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();
	if (strcmp(controllerData.GetScratchPadString(), "MISAP_") == 0) {
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