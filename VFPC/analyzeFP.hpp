#pragma once
#include "EuroScopePlugIn.h"
#include <sstream>
#include <iostream>
#include <string>
#include "Constant.hpp"
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <HKCPDisplay.hpp>

using namespace std;
using namespace EuroScopePlugIn;
using json = nlohmann::json;

struct ValidationInfo {
	string errorCode;
	string FLASMessage;
	string errorMessage;
};

class CVFPCPlugin :
	public EuroScopePlugIn::CPlugIn
{
public:
	CVFPCPlugin();
	virtual ~CVFPCPlugin();

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

	virtual void OnAirportRunwayActivityChanged();

	void sendMessage(string type, string message);

	void sendMessage(string message);

	bool IsStringInList(const string& target, const vector<string>& list);

	bool IsDestinationMatch(const string& destination, const json& rule);

	bool IsAirwayMatch(const string& route, const json& rule);

	string CheckAltitude(int rfl, const json& rules);

	string ValidateRules(const json& rule, const string& destination, const string& route, int rfl);

	void ValidateFlightPlan(CFlightPlan& flightPlan, const json& sidData);

	void AutoAssignSid(CFlightPlan& flightPlan, const json& sidData, int config);

	void InsertSidFlightPlan(CFlightPlan& flightPlan, string sid, string sidWaypoint);

	void UpdateActiveDepRunways();

	HKCPDisplay* DisplayPtr;
protected:
	json sidData;
	unordered_map<string, ValidationInfo> VFPCFPData;
	set<string> activeDepRunways;
	string hongKongConf;
};

