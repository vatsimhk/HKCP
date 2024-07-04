#pragma once
#include "EuroScopePlugIn.h"
#include "stdafx.h"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;
using namespace EuroScopePlugIn;

class AT3Tags :
	public EuroScopePlugIn::CPlugIn

{
public:
	AT3Tags();

	string pfad;
	char DllPathFile[_MAX_PATH];

	virtual void OnGetTagItem(CFlightPlan FlightPlan,
		CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int* pColorCode,
		COLORREF* pRGB,
		double* pFontSize);

	virtual void    OnFunctionCall(int FunctionId,
		const char* sItemString,
		POINT Pt,
		RECT Area);

	void InitialiseApps();

	vector<string> GetAvailableApps(string airport, string runway);

	string GetFormattedAltitude(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedAltitudedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedTrack(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedHeadingAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedGroundspeed(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedSpeedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetRouteCode(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetAPPDEPLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetAMCLine4 (CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedSlot(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetCallsign(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetATYPWTC(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetVSIndicator(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetArrivalRwy(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

protected:
	Document appsDoc;
};
