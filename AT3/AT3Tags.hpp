#pragma once
#include "EuroScopePlugIn.h"
#include "stdafx.h"
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace EuroScopePlugIn;
using json = nlohmann::json;

class AT3Tags :
	public EuroScopePlugIn::CPlugIn

{
public:
	AT3Tags();

	string path;
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

	virtual void    OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan,
		int DataType);

	vector<string> GetAvailableApps(string airport, string runway);

	vector<string> GetAvailableRtes(string airport, string runway);

	string GetFormattedAltitude(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedAltitudedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedTrack(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedHeadingAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedGroundspeed(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedSpeedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	void GetRouteCode(CFlightPlan& FlightPlan);

	string GetRouteCodeLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	void GetAssignedAPP(CFlightPlan& FlightPlan);

	string GetAPPDEPLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetAMCLine4 (CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedSlot(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetCallsign(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetATYPWTC(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetVSIndicator(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetArrivalRwy(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

protected:
	json appsJson;
	json rteJson;
	set<string> arptSet;
};
