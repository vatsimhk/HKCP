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
#include "MAESTROapi.h"
#include <chrono>
#include <ctime>

using namespace std;
using namespace EuroScopePlugIn;
using json = nlohmann::json;

class AT3Tags :
	public EuroScopePlugIn::CPlugIn

{
public:
	AT3Tags();

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

	virtual void    OnTimer(int Counter);

	void SetApp(int index, CFlightPlan FlightPlan, vector<string> appsVec);

	void SetRte(int index, CFlightPlan FlightPlan, vector<string> rteVec, string dest, string destRunway);

	string GetActiveArrRwy(string airport);

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

	string GetFormattedETA(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget, int minutes);

	string GetAMANDelay(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetCallsign(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetATYPWTC(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetVSIndicator(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedArrivalRwy(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

protected:
	int minu;
	json appsJson;
	json rteJson;
	set<string> arptSet;
};
