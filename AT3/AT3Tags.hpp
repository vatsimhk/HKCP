#pragma once
#include "EuroScopePlugIn.h"
#include "stdafx.h"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace EuroScopePlugIn;

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

};
