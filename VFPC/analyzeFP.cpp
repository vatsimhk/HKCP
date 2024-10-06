#include "stdafx.h"
#include "analyzeFP.hpp"
#include "EuroScopePlugIn.h"
#include <time.h> 

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
		tagOutput = ValidateFlightPlan(FlightPlan, sidData);
		break;
	default:
		IsVFPCItem = false;
	}

	if (IsVFPCItem) {
		strcpy_s(sItemString, 16, tagOutput.substr(0, 15).c_str());
	}
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
	
	if (rule["airways"].is_string()) {
		airways.push_back(rule["airways"].get<string>());
	}
	else {
		airways = rule["airways"].get<vector<string>>();
	}
	return IsStringInList(route, airways);
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
		bool isOdd = rfl % 200 == 100;
		string direction = rules["direction"].get<string>();
		if ((direction == "ODD" && !isOdd) || (direction == "EVEN" && isOdd)) {
			return "FLR";
		}
	}

	return "OK"; // If no altitude restrictions are violated
}

string CVFPCPlugin::ValidateRules(const json& rule, const string& destination, const string& route, int rfl)
{
	sendMessage("Processing Rule " + rule.dump());
	
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

string CVFPCPlugin::ValidateFlightPlan(CFlightPlan& flightPlan, const json& sidData)
{
	string departureAirport = flightPlan.GetFlightPlanData().GetOrigin();
	string flightRoute = flightPlan.GetFlightPlanData().GetRoute();
	string destination = flightPlan.GetFlightPlanData().GetDestination();
	int rfl = flightPlan.GetFlightPlanData().GetFinalAltitude();

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
						return result; // Return result immediately if found
					}
				}
			}
		}
	}

	return "CHK"; // No matching route or SID found
}


