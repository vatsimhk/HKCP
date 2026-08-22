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
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace EuroScopePlugIn;
using json = nlohmann::json;

class AT3Tags :
	public EuroScopePlugIn::CPlugIn

{
public:
	AT3Tags(COLORREF colorA, COLORREF colorNA, COLORREF colorR, COLORREF colorV);

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

	string GetFormattedAltitude(CRadarTarget& RadarTarget);

	string GetFormattedAltitudedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget);

	string GetFormattedTrack(CRadarTarget& RadarTarget);

	string GetFormattedHeadingAssigned(CFlightPlan& FlightPlan);

	string GetFormattedGroundspeed(CRadarTarget& RadarTarget);

	string GetFormattedSpeedAssigned(CFlightPlan& FlightPlan);

	void GetRouteCode(CFlightPlan& FlightPlan);

	string GetRouteCodeLine4(CFlightPlan& FlightPlan);

	void GetAssignedAPP(CFlightPlan& FlightPlan);

	string GetAPPDEPLine4(CFlightPlan& FlightPlan);

	string GetAMCLine4 (CFlightPlan& FlightPlan);

	string GetFormattedETA(CFlightPlan& FlightPlan, int minutes);

	string GetAMANDelay(CFlightPlan& FlightPlan);

	string GetCallsign(CFlightPlan& FlightPlan);

	string GetADSBCallsign(CRadarTarget& RadarTarget);

	string GetATYPWTC(CFlightPlan& FlightPlan);

	string GetVSIndicator(CRadarTarget& RadarTarget);

	string GetFormattedArrivalRwy(CFlightPlan& FlightPlan);

	static void SplitCallsign(const std::string& callsign, std::string& prefix, std::string& number);

	static bool isSimilarCallsign(const std::string& CurrentPrefix, std::string& CurrentNum, std::string& otherPrefix, std::string& otherNum);

	static bool isCorrelateCorrect(CFlightPlan FlightPlan, string CurrentCallsign);

	string GetALRT(CFlightPlan& FlightPlan);

	string GetWTG(CFlightPlan& FlightPlan);

protected:
	int minu;
	json appsJson;
	json rteJson;
	unordered_map<string, string> wtgMap;
	set<string> arptSet;
	unordered_map<string, int> callsignToHandoffTimer;

	COLORREF colorAssumed;
	COLORREF colorNotAssumed;
	COLORREF colorRedundant;
	COLORREF colorVFR;
	static const int HOW_WARNING_TIME = 30;

private:
	template <typename Out>
	void split(const string& s, char delim, Out result) {
		istringstream iss(s);
		string item;
		while (getline(iss, item, delim)) {
			*result++ = item;
		}
	}

	vector<string> split(const string& s, char delim) {
		vector<string> elems;
		split(s, delim, back_inserter(elems));
		return elems;
	}
};
