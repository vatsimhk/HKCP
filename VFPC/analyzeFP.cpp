#include "stdafx.h"
#include "analyzeFP.hpp"
#include "EuroScopePlugIn.h"
#include <time.h>
#include <boost/algorithm/string.hpp>

extern "C" IMAGE_DOS_HEADER __ImageBase;

bool blink;
bool debugMode, initialSidLoad;

int disCount;

ifstream sidDatei;
char DllPathFile[_MAX_PATH];
string pfad;

vector<string> sidName;
vector<string> sidEven;
vector<int> sidMin;
vector<int> sidMax;
vector<string> AircraftIgnore;

using namespace std;
using namespace EuroScopePlugIn;

	// Run on Plugin Initialization
CVFPCPlugin::CVFPCPlugin(void) :CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT)
{

	// Register Tag Item "VFPC"
	RegisterTagItemType("VFPC", TAG_ITEM_FPCHECK);
	RegisterTagItemType("VFPC (if failed)", TAG_ITEM_FPCHECK_IF_FAILED);
	// RegisterTagItemType("VFPC (if failed, static)", TAG_ITEM_FPCHECK_IF_FAILED_STATIC);
	RegisterTagItemFunction("Check FP Menu", TAG_FUNC_CHECKFP_MENU);
	RegisterTagItemFunction("Modify RFL Menu", TAG_FUNC_MODRFL_MENU);
	RegisterTagItemFunction("Set SID", TAG_FUNC_SET_SID);

	// Get Path of the Sid.txt
	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	pfad = DllPathFile;
	pfad.resize(pfad.size() - strlen("HKCP.dll"));
	pfad += "Sid.json";

	debugMode = false;
	initialSidLoad = false;

	try {
		fstream inputFile(pfad);
		sidData = json::parse(inputFile);
	}
	catch (...) {
		sendMessage("Error loading sid.json!");
	}

	UpdateActiveDepRunways();
}

// Run on Plugin destruction, Ie. Closing EuroScope or unloading plugin
CVFPCPlugin::~CVFPCPlugin()
{
}

void CVFPCPlugin::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid()) {
		return;
	}

	bool IsVFPCItem = true;
	string tagOutput;

	switch (ItemCode) {
	case TAG_ITEM_FPCHECK:
		ValidateFlightPlan(FlightPlan, sidData);
		tagOutput = VFPCFPData[FlightPlan.GetCallsign()].errorCode;
		break;
	default:
		IsVFPCItem = false;
	}


	if (!IsVFPCItem) {
		return;
	}

	*pColorCode = TAG_COLOR_RGB_DEFINED;
	if (tagOutput == "OK") {
		*pRGB = TAG_GREEN;
	}
	else if (tagOutput == "CHK") {
		*pRGB = TAG_RED;
	}
	else if (tagOutput == "VFR" || tagOutput == "LIFR") {
		*pRGB = TAG_CYAN;
	}
	else {
		*pRGB = TAG_YELLOW;
	}
	strcpy_s(sItemString, 16, tagOutput.substr(0, 15).c_str());
}

void CVFPCPlugin::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	CFlightPlan FlightPlan = FlightPlanSelectASEL();
	if (!FlightPlan.IsValid()) {
		return;
	}

	string callsign = FlightPlan.GetCallsign();

	switch(FunctionId) {
	case TAG_FUNC_CHECKFP_MENU:
		OpenPopupList(Area, "Check FP Menu", 1);
		AddPopupListElement("Check FLAS", "", TAG_FUNC_CHECKFP_FLAS, false, 2, false);
		break;
	case TAG_FUNC_CHECKFP_FLAS:
		sendMessage("Valid FLs for " + callsign, VFPCFPData[callsign].FLASMessage);
		break;
	default:
		break;
	}
}

void CVFPCPlugin::OnAirportRunwayActivityChanged()
{
	UpdateActiveDepRunways();
}

/*
	Custom Functions
*/

void CVFPCPlugin::sendMessage(string type, string message) {
	// Show a message
	DisplayUserMessage("VFPC", type.c_str(), message.c_str(), true, true, true, true, false);
}

void CVFPCPlugin::sendMessage(string message) {
	DisplayUserMessage("Message", "VFPC", message.c_str(), true, true, true, false, false);
}

bool CVFPCPlugin::IsStringInList(const string& target, const vector<string>& list)
{
	return any_of(list.begin(), list.end(), [&target](const string& item) {
		return target.find(item) != string::npos;
	});
}

bool CVFPCPlugin::IsDestinationMatch(const string& destination, const json& rule)
{
	if (!rule.contains("destinations")) {
		return true; // No destination restriction
	}
	for (const auto& dest : rule["destinations"]) {
		if (destination.find(dest.get<string>()) != string::npos) {
			return true;
		}
	}
	return false;
}

bool CVFPCPlugin::IsAirwayMatch(const string& route, const json& rule)
{
	if (!rule.contains("airways")) {
		return true; // No airway restriction
	}

	vector<string> airways;

	vector<string> routeVec;
	boost::split(routeVec, route, boost::is_any_of(" "));
	string newRoute = "";
	for (std::size_t i = 0; i < routeVec.size(); i++) {
		if (i != 0 && i != routeVec.size()) {
			routeVec[i] = routeVec[i].substr(0, routeVec[i].find_first_of('/'));
		}
		boost::to_upper(routeVec[i]);
		newRoute = newRoute + routeVec[i] + " ";
	}
	
	if (rule["airways"].is_string()) {
		airways.push_back(rule["airways"].get<string>());
	}
	else {
		airways = rule["airways"].get<vector<string>>();
	}
	return IsStringInList(newRoute, airways);
}

string CVFPCPlugin::CheckAltitude(int rfl, const json& rules)
{
	// Check allowed flight levels
	rfl = rfl / 100;
	if (rules.contains("allowed_fls")) {
		for (const auto& fl : rules["allowed_fls"]) {
			if (rfl == stoi(fl.get<string>())) {
				return "OK";
			}
		}
		return "FLR"; // If RFL does not match allowed_fls
	}

	// Check max/min flight levels
	if (rules.contains("max_fl") && rfl > rules["max_fl"].get<int>()) {
		return "FLR";
	}
	if (rules.contains("min_fl") && rfl < rules["min_fl"].get<int>()) {
		return "FLR";
	}

	// Check for directional restrictions (ODD/EVEN)
	if (rules.contains("direction")) {
		bool isOdd = rfl % 20;
		string direction = rules["direction"].get<string>();
		if ((direction == "ODD" && !isOdd) || (direction == "EVEN" && isOdd)) {
			return "FLR";
		}
	}

	return "OK"; // If no altitude restrictions are violated
}

string CVFPCPlugin::ValidateRules(const json& rule, const string& destination, const string& route, int rfl)
{
	//sendMessage("Processing Rule " + rule.dump());
	
	try {
		if (!IsDestinationMatch(destination, rule)) {
			return "CHK"; // Destination mismatch
		}

		if (!IsAirwayMatch(route, rule)) {
			return "CHK"; // Airway mismatch
		}

		return CheckAltitude(rfl, rule); // Check altitude restrictions
	}
	catch (...) {
		return "CHK";
	}
}

void CVFPCPlugin::ValidateFlightPlan(CFlightPlan& flightPlan, const json& sidData)
{
	string departureAirport = flightPlan.GetFlightPlanData().GetOrigin();
	string flightRoute = flightPlan.GetFlightPlanData().GetRoute();
	string destination = flightPlan.GetFlightPlanData().GetDestination();
	string callsign = flightPlan.GetCallsign();
	int rfl = flightPlan.GetFlightPlanData().GetFinalAltitude();

	if (VFPCFPData.find(callsign) == VFPCFPData.end()) {
		ValidationInfo info;
		VFPCFPData[callsign] = info;
		VFPCFPData[callsign].errorCode = "CHK";
		VFPCFPData[callsign].FLASMessage = "No valid altitudes found";
	}

	string flightRules = flightPlan.GetFlightPlanData().GetPlanType();

	if (flightRules == "V") {
		VFPCFPData[callsign].errorCode = "VFR";
		return;
	}

	if (destination == departureAirport) {
		VFPCFPData[callsign].errorCode = "LIFR";
		return;
	}

	// Step 1: Match the departure airport
	for (const auto& airportEntry : sidData) {
		if (airportEntry["icao"].get<string>() != departureAirport) {
			continue; // Skip if the departure airport doesn't match
		}


		// Step 2: Check if the flight plan route contains a SID waypoint
		for (const auto& sidEntry : airportEntry["sids"].items()) {
			string sidWaypoint = sidEntry.key();
			if (flightRoute.find(sidWaypoint) != string::npos) {
				// Step 3: Iterate through the rules for the matched SID
				for (const auto& rule : sidEntry.value()) {
					string result = ValidateRules(rule, destination, flightRoute, rfl);
					if (result == "OK" || result == "FLR") {
						VFPCFPData[callsign].errorCode = result;
						if (rule.contains("FLAS")) {
							VFPCFPData[callsign].FLASMessage = rule["FLAS"].get<string>();
						}
						else {
							VFPCFPData[callsign].FLASMessage = "No valid FLAS message found";
						}
						return; // Return result immediately if found
					}
				}
			}
		}
	}

	// If we get here then no valid check was found
	VFPCFPData[callsign].errorCode = "CHK";
	VFPCFPData[callsign].FLASMessage = "No valid altitudes found";
}

void CVFPCPlugin::UpdateActiveDepRunways()
{
	CSectorElement runway;
	vector<string> activeRunways;
	for (runway = SectorFileElementSelectFirst(SECTOR_ELEMENT_RUNWAY); runway.IsValid(); runway = SectorFileElementSelectNext(runway, SECTOR_ELEMENT_RUNWAY)) {
		if (strcmp(runway.GetRunwayName(0), "NAP") == 0) {
			continue;
		}
		if (runway.IsElementActive(true, 0)) {
			activeRunways.push_back(runway.GetRunwayName(0));
		}
		if (runway.IsElementActive(true, 1)) {
			activeRunways.push_back(runway.GetRunwayName(1));
		}
	}

	activeDepRunways = activeRunways;
}


