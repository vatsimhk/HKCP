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
	pfad += "HKCP-VFPC.json";

	debugMode = false;
	initialSidLoad = false;
}

// Run on Plugin destruction, Ie. Closing EuroScope or unloading plugin
CVFPCPlugin::~CVFPCPlugin()
{
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
	return std::any_of(list.begin(), list.end(), [&target](const std::string& item) {
		return target.find(item) != std::string::npos;
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
	vector<string> airways = rule["airways"].get<vector<string>>();
	return IsStringInList(route, airways);
}

string CVFPCPlugin::CheckAltitude(int rfl, const json& rules)
{
	// Check allowed flight levels
	if (rules.contains("allowed_fls")) {
		for (const auto& fl : rules["allowed_fls"]) {
			if (rfl == fl.get<int>()) {
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

string CVFPCPlugin::ValidateFlightPlan(CFlightPlan& flightPlan, const json& sidData)
{
	return "CHK";
}


