#include "stdafx.h"
#include "analyzeFP.hpp"
#include "EuroScopePlugIn.h"
#include <time.h>
#include <boost/algorithm/string.hpp>

#define AUTO 0
#define FORCE_3RS 1
#define FORCE_NAP 2

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
	RegisterTagItemFunction("Assign SID Menu", TAG_FUNC_ASSIGN_SID_MENU);
	RegisterTagItemFunction("Auto Assign SID", TAG_FUNC_ASSIGN_SID_AUTO);
	RegisterTagItemFunction("Auto Assign SID (Force 3RS)", TAG_FUNC_ASSIGN_SID_3RS);
	RegisterTagItemFunction("Auto Assign SID (Force NAP)", TAG_FUNC_ASSIGN_SID_NAP);
	//RegisterTagItemFunction("Manually Assign SID", TAG_FUNC_ASSIGN_SID_MANUAL);

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
		if (VFPCFPData[FlightPlan.GetCallsign()].active) {
			tagOutput = "-";
			break;
		}

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
	if (tagOutput == "-") {
		*pRGB = TAG_GREY;
	} else if (tagOutput == "OK") {
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
	string origin = FlightPlan.GetFlightPlanData().GetOrigin();
	string rect_coords = to_string(Area.left) + " " + to_string(Area.right) + " " + to_string(Area.top) + " " + to_string(Area.bottom);
	string FLASMessage;

	switch(FunctionId) {
	case TAG_FUNC_CHECKFP_MENU:
		OpenPopupList(Area, "VFPC Menu", 1);
		if (VFPCFPData[callsign].active) {
			AddPopupListElement("Enable Checks", "", TAG_FUNC_VFPC_ON_OFF, false, POPUP_ELEMENT_UNCHECKED, false);
		}
		else {
			AddPopupListElement("Enable Checks", "", TAG_FUNC_VFPC_ON_OFF, false, POPUP_ELEMENT_CHECKED, false);
		}
		AddPopupListElement("Check FLAS", "", TAG_FUNC_CHECKFP_FLAS, false, POPUP_ELEMENT_NO_CHECKBOX, false);
		AddPopupListElement("Assign SID", "", TAG_FUNC_ASSIGN_SID_AUTO, false, POPUP_ELEMENT_NO_CHECKBOX, false);
		if (origin == "VHHH") {
			AddPopupListElement("Assign SID (Force 3RS)", "", TAG_FUNC_ASSIGN_SID_3RS, false, POPUP_ELEMENT_NO_CHECKBOX, false);
			AddPopupListElement("Assign SID (Force NAP)", "", TAG_FUNC_ASSIGN_SID_NAP, false, POPUP_ELEMENT_NO_CHECKBOX, false);
		}
		break;
	case TAG_FUNC_VFPC_ON_OFF:
		VFPCFPData[callsign].active = !VFPCFPData[callsign].active;
		break;
	case TAG_FUNC_CHECKFP_FLAS:
		FLASMessage = VFPCFPData[callsign].FLASMessage;
		if (VFPCFPData[callsign].errorCode == "FLR") {
			FLASMessage += " (Failed)";
		}
		else {
			FLASMessage += " (Passed)";
		}
		sendMessage("Valid FLs for " + callsign, FLASMessage);
		break;
	case TAG_FUNC_ASSIGN_SID_MENU:
		OpenPopupList(Area, "VFPC Assign SID", 1);
		AddPopupListElement("Auto            ", "", TAG_FUNC_ASSIGN_SID_AUTO, false, POPUP_ELEMENT_NO_CHECKBOX, false);
		if (origin == "VHHH") {
			AddPopupListElement("Auto (Force 3RS)", "", TAG_FUNC_ASSIGN_SID_3RS, false, POPUP_ELEMENT_NO_CHECKBOX, false);
			AddPopupListElement("Auto (Force NAP)", "", TAG_FUNC_ASSIGN_SID_NAP, false, POPUP_ELEMENT_NO_CHECKBOX, false);
		}
		//AddPopupListElement("Manual", "", TAG_FUNC_ASSIGN_SID_MANUAL, false, POPUP_ELEMENT_NO_CHECKBOX, false);
		break;
	case TAG_FUNC_ASSIGN_SID_AUTO:
		AutoAssignSid(FlightPlan, sidData, AUTO);
		break;
	case TAG_FUNC_ASSIGN_SID_3RS:
		AutoAssignSid(FlightPlan, sidData, FORCE_3RS);
		break;
	case TAG_FUNC_ASSIGN_SID_NAP:
		AutoAssignSid(FlightPlan, sidData, FORCE_NAP);
		break;
	case TAG_FUNC_ASSIGN_SID_MANUAL:
		//sendMessage(rect_coords);
		DisplayPtr->StartTagFunction(callsign.c_str(), NULL, TAG_ITEM_TYPE_ASSIGNED_SID, FlightPlan.GetFlightPlanData().GetSidName(), NULL, TAG_ITEM_FUNCTION_ASSIGNED_SID, Pt, Area);
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

	if (rule.contains("sid_07") || rule.contains("sid_25") || rule.contains("sid_all")) {
		return "CHK";
	}
	
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
		VFPCFPData[callsign].active = true;
	}

	string flightRules = flightPlan.GetFlightPlanData().GetPlanType();

	if (flightRules == "V") {
		VFPCFPData[callsign].errorCode = "VFR";
		VFPCFPData[callsign].FLASMessage = "No altitude checks are run for VFR Traffic.";
		return;
	}

	if (destination == departureAirport) {
		VFPCFPData[callsign].errorCode = "LIFR";
		VFPCFPData[callsign].FLASMessage = "No altitude checks are run for Local IFR Traffic.";
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

void CVFPCPlugin::AutoAssignSid(CFlightPlan& flightPlan, const json& sidData, int config)
{
	string departureAirport = flightPlan.GetFlightPlanData().GetOrigin();
	string flightRoute = flightPlan.GetFlightPlanData().GetRoute();
	string destination = flightPlan.GetFlightPlanData().GetDestination();
	string callsign = flightPlan.GetCallsign();

	for (const auto& airportEntry : sidData) {
		if (airportEntry["icao"].get<string>() != departureAirport) {
			continue; // Skip if the departure airport doesn't match
		}


		for (const auto& sidEntry : airportEntry["sids"].items()) {
			string sidWaypoint = sidEntry.key();
			if (flightRoute.find(sidWaypoint) == string::npos) {
				continue;
			}

			const auto rule = sidEntry.value()[0];
			time_t curr_time;
			curr_time = time(NULL);
			tm* tm_gmt = gmtime(&curr_time);

			vector<string> sidList;
			if (activeDepRunways.find("07R") != activeDepRunways.end() || 
			    activeDepRunways.find("07C") != activeDepRunways.end() || 
			    activeDepRunways.find("07L") != activeDepRunways.end()) {
				if (rule.contains("sid_noise") && (tm_gmt->tm_hour >= 15 && tm_gmt->tm_hour <= 23 && config != FORCE_3RS) || config == FORCE_NAP) {
					sidList = rule["sid_noise"].get<vector<string>>();
				}
				else if (rule.contains("sid_07")) {
					sidList = rule["sid_07"].get<vector<string>>();
				}
			}
			else if ( (activeDepRunways.find("25R") != activeDepRunways.end() ||
				activeDepRunways.find("25C") != activeDepRunways.end() ||
				activeDepRunways.find("25L") != activeDepRunways.end() ) &&
				rule.contains("sid_25")) {
				sidList = rule["sid_25"].get<vector<string>>();
			}
			
			if (rule.contains("sid_all")) {
				sidList = rule["sid_all"].get<vector<string>>();
			}

			if (sidList.empty()) {
				continue;
			}

			for (const string& sid : sidList) {
				// Extract runway from SID and check if that runway is active
				string rwy;
				if (sid.length() > 8) {
					rwy = sid.substr(8, 3);
				}
				else {
					rwy = sid.substr(6, 2);
				}
				if (find(activeDepRunways.begin(), activeDepRunways.end(), rwy) != activeDepRunways.end()) {
					// Insert first SID that matches departure ruinway
					InsertSidFlightPlan(flightPlan, sid, sidWaypoint);
					break;
				}
			}

			//sendMessage(rule.dump());
			return;
		}
	}

	sendMessage(callsign, "Unable to auto assign SID");
}

void CVFPCPlugin::InsertSidFlightPlan(CFlightPlan& flightPlan, string sid, string sidWaypoint)
{
	string flightRoute = flightPlan.GetFlightPlanData().GetRoute();
	size_t pos = flightRoute.find(sidWaypoint + " ");

	if (pos != string::npos) {
		flightRoute.erase(0, pos);
	}

	if (sidWaypoint.length() >= 5 || sid.length() <= 8) {
		// Insert SID/RWY
		flightRoute.insert(0, sid + " ");
	}
	else {
		// Insert SID/RWY WAYPOINT, e.g. DALOL1X/07R DALOL
		flightRoute.insert(0, sid + " " + sid.substr(0, 5) + " ");
	}

	flightPlan.GetFlightPlanData().SetRoute(flightRoute.c_str());
	flightPlan.GetFlightPlanData().AmendFlightPlan();
}

void CVFPCPlugin::UpdateActiveDepRunways()
{
	CSectorElement runway;
	set<string> activeRunways;
	for (runway = SectorFileElementSelectFirst(SECTOR_ELEMENT_RUNWAY); runway.IsValid(); runway = SectorFileElementSelectNext(runway, SECTOR_ELEMENT_RUNWAY)) {
		if (strcmp(runway.GetRunwayName(0), "NAP") == 0) {
			continue;
		}
		if (runway.IsElementActive(true, 0)) {
			activeRunways.insert(runway.GetRunwayName(0));
		}
		if (runway.IsElementActive(true, 1)) {
			activeRunways.insert(runway.GetRunwayName(1));
		}
	}

	activeDepRunways = activeRunways;
}


