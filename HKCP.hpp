#pragma once
#include "EuroScopePlugIn.h"
#include <sstream>
#include <iostream>
#include <string>
#include "Constant.hpp"

using namespace std;
using namespace EuroScopePlugIn;

class HKCPPlugin :
	public EuroScopePlugIn::CPlugIn
{
public:
	HKCPPlugin();

	~HKCPPlugin();

	COLORREF GetTopSkyColorSettings(string settingName, COLORREF defaultColor);

	virtual void OnFunctionCall(int FunctionId, const char* ItemString, POINT Pt, RECT Area);

	virtual CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

	virtual void OnGetTagItem(CFlightPlan FlightPlan,
		CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int* pColorCode,
		COLORREF* pRGB,
		double* pFontSize);

	virtual void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);

	virtual void OnFlightPlanDisconnect(CFlightPlan FlightPlan);

	virtual bool OnCompileCommand(const char* sCommandLine);

	virtual void OnTimer(int Count);

	virtual void OnCompilePrivateChat(const char* sSenderCallsign, const char* sReceiverCallsign, const char* sChatMessage);

	virtual void OnAirportRunwayActivityChanged();

	COLORREF colorAssumed;
	COLORREF colorNotAssumed;
	COLORREF colorRedundant;
};
