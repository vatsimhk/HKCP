#include "stdafx.h"
#include "MissedApproachAlarm.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include <time.h>

using namespace EuroScopePlugIn;

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

	MissedApproachAlarmLogic ma;
	vector<string> missedAircraftData = ma.getMissedApproaches("VHHH");
	if (missedAircraftData.size() == 0) {
		return;
	}

	string callsignText = missedAircraftData[0];
	string destText = missedAircraftData[1];
	string rwyText = missedAircraftData[2];

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
	CRect title(windowAreaCRect.left, windowAreaCRect.top+20, windowAreaCRect.left + 400, windowAreaCRect.top + 60);
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

void MissedApproachAlarm::flashButton(HDC hDC, CRect button) {
	CDC dc;
	dc.Attach(hDC);
	if (ackButtonState == 1) {
		dc.FillSolidRect(button, ACK_BUTTON_GREEN);
		dc.Detach();
		return;
	}

	// Play Sound
	char DllPathFile[_MAX_PATH];
	string pfad;
	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	pfad = DllPathFile;
	pfad.resize(pfad.size() - strlen("VFPC.dll"));
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
	if (ObjectType == ACK_BUTTON && Button == BUTTON_LEFT) {
		if (ackButtonState == 0) {
			ackButtonState = 1;
		}
		else {
			MissedApproachAlarmLogic ma;
			ma.ackMissedApproach();
			ackButtonState = 0;
		}
	}

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
}

//---OnButtonDownScreenObject---------------------------------------------

void MissedApproachAlarm::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
}

//---OnButtonDownScreenObject---------------------------------------------

void MissedApproachAlarm::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {

}

//PLUGIN BACKEND (Logic)

MissedApproachAlarmLogic::MissedApproachAlarmLogic(): CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT) {

}

vector<string> MissedApproachAlarmLogic::getMissedApproaches(const char * dest) {
	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	vector<string> acftData;
	for (CFlightPlan fpl = FlightPlanSelectFirst(); fpl.IsValid();  fpl = FlightPlanSelectNext(fpl)) {
		float distanceToDest = fpl.GetDistanceToDestination();
		// if (distanceToDest > 20.0) continue;

		data = fpl.GetFlightPlanData();
		controllerData = fpl.GetControllerAssignedData();
		if (strcmp(controllerData.GetScratchPadString(), "MISAP_") == 0) {
			acftData.push_back(fpl.GetCallsign());
			acftData.push_back(data.GetDestination());
			acftData.push_back(data.GetArrivalRwy());
			return acftData;
		}
	}
	return acftData;
}

void MissedApproachAlarmLogic::ackMissedApproach() {
	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	for (CFlightPlan fpl = FlightPlanSelectFirst(); fpl.IsValid(); fpl = FlightPlanSelectNext(fpl)) {
		data = fpl.GetFlightPlanData();
		controllerData = fpl.GetControllerAssignedData();
		if (strcmp(controllerData.GetScratchPadString(), "MISAP_") == 0) {
			controllerData.SetScratchPadString("");
		}
	}
	//couldn't find it, handle error
}