#include "stdafx.h"
#include "analyzeFP.hpp"
#include "EuroScopePlugIn.h"

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
	string loadingMessage = "Version: ";
	loadingMessage += MY_PLUGIN_VERSION;
	loadingMessage += " loaded.";
	sendMessage(loadingMessage);

	// Register Tag Item "VFPC"
	RegisterTagItemType("VFPC", TAG_ITEM_FPCHECK);
	RegisterTagItemType("VFPC (if failed)", TAG_ITEM_FPCHECK_IF_FAILED);
	RegisterTagItemType("VFPC (if failed, static)", TAG_ITEM_FPCHECK_IF_FAILED_STATIC);
	RegisterTagItemFunction("Check FP", TAG_FUNC_CHECKFP_MENU);

	// Get Path of the Sid.txt
	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	pfad = DllPathFile;
	pfad.resize(pfad.size() - strlen("VFPC.dll"));
	pfad += "Sid.json";

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

void CVFPCPlugin::debugMessage(string type, string message) {
	// Display Debug Message if debugMode = true
	if (debugMode) {
		DisplayUserMessage("VFPC", type.c_str(), message.c_str(), true, true, true, false, false);
	}
}

void CVFPCPlugin::sendMessage(string type, string message) {
	// Show a message
	DisplayUserMessage("VFPC", type.c_str(), message.c_str(), true, true, true, true, false);
}

void CVFPCPlugin::sendMessage(string message) {
	DisplayUserMessage("Message", "VFPC", message.c_str(), true, true, true, false, false);
}

void CVFPCPlugin::getSids() {
	stringstream ss;
	ifstream ifs;
	ifs.open(pfad.c_str(), ios::binary);
	ss << ifs.rdbuf();
	ifs.close();

	if (config.Parse<0>(ss.str().c_str()).HasParseError()) {
		string msg = str(boost::format("An error parsing VFPC configuration occurred. Error: %s (Offset: %i)\nOnce fixed, reload the config by typing '.vfpc reload'") % config.GetParseError() % config.GetErrorOffset());
		sendMessage(msg);

		config.Parse<0>("[{\"icao\": \"XXXX\"}]");
	}
	
	airports.clear();

	for (SizeType i = 0; i < config.Size(); i++) {
		const Value& airport = config[i];
		string airport_icao = airport["icao"].GetString();

		airports.insert(pair<string, SizeType>(airport_icao, i));
	}
}

// Does the checking and magic stuff, so everything will be alright, when this is finished! Or not. Who knows?
map<string, string> CVFPCPlugin::validizeSid(CFlightPlan flightPlan) {
	/*	CS = Callsign,
		AIRPORT = Origin
		SEARCH = SID search error
		SID = SID,
		DESTINATION = Destination,
		AIRWAYS = Airway,
		NOISE = Engine Type,
		DIRECTION = Even / Odd,
		MIN_FL = Minimum Flight Level,
		MAX_FL = Maximum Flight Level,
		ALLOWED_FL = Allowed Flight Level,
		NAVIGATION = Navigation restriction,
		STATUS = Passed
	*/
	map<string, string> returnValid;

	returnValid["CS"] = flightPlan.GetCallsign();
	returnValid["STATUS"] = "Passed";
	bool valid{ false };

	string origin = flightPlan.GetFlightPlanData().GetOrigin(); boost::to_upper(origin);
	string destination = flightPlan.GetFlightPlanData().GetDestination(); boost::to_upper(destination);
	SizeType origin_int;
	int RFL = flightPlan.GetFlightPlanData().GetFinalAltitude();
	
	vector<string> route = split(flightPlan.GetFlightPlanData().GetRoute(), ' ');
	for (std::size_t i = 0; i < route.size(); i++) {
		boost::to_upper(route[i]);
	}

	string sid = flightPlan.GetFlightPlanData().GetSidName(); boost::to_upper(sid);

	// Flightplan has SID
	if (!sid.length()) {
		returnValid["SEARCH"] = "Flightplan doesn't have SID set!";
		returnValid["STATUS"] = "Failed";
		return returnValid;
	}

	string first_wp = sid.substr(0, sid.find_first_of("0123456789"));
	if (0 != first_wp.length())
		boost::to_upper(first_wp);
	string sid_suffix;
	if (first_wp.length() != sid.length()) {
		sid_suffix = sid.substr(sid.find_first_of("0123456789"), sid.length());
		boost::to_upper(sid_suffix);
	}
	string first_airway;

	// Did not find a valid SID
	if (sid_suffix.length() == 0 && "VCT" != first_wp) {
		returnValid["SEARCH"] = "Flightplan doesn't have SID set!";
		returnValid["STATUS"] = "Failed";
		return returnValid;
	}

	vector<string>::iterator it = find(route.begin(), route.end(), first_wp);
	if (it != route.end() && (it - route.begin()) != route.size() - 1) {
		first_airway = route[(it - route.begin()) + 1];
		boost::to_upper(first_airway);
	}

	// Airport defined
	if (airports.find(origin) == airports.end()) {
		returnValid["SEARCH"] = "No valid Airport found!";
		returnValid["STATUS"] = "Failed";
		return returnValid;
	}
	else
		origin_int = airports[origin];

	// Any SIDs defined
	if (!config[origin_int].HasMember("sids") || config[origin_int]["sids"].IsArray()) {
		returnValid["SEARCH"] = "No SIDs defined!";
		returnValid["STATUS"] = "Failed";
		return returnValid;
	}

	// Needed SID defined
	if (!config[origin_int]["sids"].HasMember(first_wp.c_str()) || !config[origin_int]["sids"][first_wp.c_str()].IsArray()) {
		returnValid["SEARCH"] = "No valid SID found!";
		returnValid["STATUS"] = "Failed";
		return returnValid;
	}

	const Value& conditions = config[origin_int]["sids"][first_wp.c_str()];
	for (SizeType i = 0; i < conditions.Size(); i++) {
		returnValid.clear();
		returnValid["CS"] = flightPlan.GetCallsign();
		bool passed[8]{ false };
		valid = false;

		// Skip SID if the check is suffix-related
		if (conditions[i]["suffix"].IsString() && conditions[i]["suffix"].GetString() != sid_suffix) {
			continue;
		}

		// Does Condition contain our destination if it's limited
		if (conditions[i]["destinations"].IsArray() && conditions[i]["destinations"].Size()) {
			string dest;
			if ((dest = destArrayContains(conditions[i]["destinations"], destination.c_str())).size()) {
				if (dest.size() < 4)
					dest += string(4 - dest.size(), '*');
				returnValid["DESTINATION"] = "For Destination (" + dest + "): ";
				passed[0] = true;
			}
			else {
				continue;
			}
		}
		else {
			returnValid["DESTINATION"] = "No Destination restr";
			passed[0] = true;
		}

		// Does Condition contain our first airway if it's limited
		if (conditions[i]["airways"].IsArray() && conditions[i]["airways"].Size()) {
			string rte = flightPlan.GetFlightPlanData().GetRoute();
			if (routeContains(rte, conditions[i]["airways"])) {
				returnValid["AIRWAYS"] = "Passed Transition Route";
				passed[1] = true;
			}
			else {
				continue;
			}
		}
		else {
			returnValid["AIRWAYS"] = "";
			passed[1] = true;
		}

		// Noise Abatement test
		string noise = conditions[i]["noise"].GetString();
		if (conditions[i]["noise"].IsString() == true) {
			if (noise == "Y") {
				returnValid["NOISE"] = "Noise abatement procedure";
			}
			else {
				returnValid["NOISE"] = "Regular procedure";
				passed[2] = true;
			}
		}
		else {
			returnValid["NOISE"] = "";
			passed[2] = true;
		}


		valid = true;
		returnValid["SID"] = first_wp;

		// Direction of condition (EVEN, ODD, ANY)
		string direction = conditions[i]["direction"].GetString();
		boost::to_upper(direction);

		if (direction == "EVEN") {
			if ((RFL / 1000) % 2 == 0) {
				returnValid["DIRECTION"] = "Even FLs (Passed)";
				passed[3] = true;
			}
			else {
				returnValid["DIRECTION"] = "Even FLs (Failed)";
			}
		}
		else if (direction == "ODD") {
			if ((RFL / 1000) % 2 != 0) {
				returnValid["DIRECTION"] = "Odd FLs (Passed)";
				passed[3] = true;
			}
			else {
				returnValid["DIRECTION"] = "ODD FLs (Failed)";
			}
		}
		else if (direction == "ANY") {
			returnValid["DIRECTION"] = "";
			passed[3] = true;
		}
		else {
			string errorText{ "Config Error for Even/Odd on SID: " };
			errorText += first_wp;
			sendMessage("Error", errorText);
			returnValid["DIRECTION"] = "Config Error for Even/Odd on this SID!";
		}
		
		// Flight level (min_fl, max_fl)
		int min_fl, max_fl;
		if (conditions[i].HasMember("min_fl") && (min_fl = conditions[i]["min_fl"].GetInt()) > 0) {
			if ((RFL / 100) >= min_fl) {
				returnValid["MIN_FL"] = "at or above FL" + to_string(conditions[i]["min_fl"].GetInt()) + " (Passed)";
				passed[4] = true;
			}
			else {
				returnValid["MIN_FL"] = "at or above FL" + to_string(min_fl) + " (Failed)";
			}
		}
		else {
			returnValid["MIN_FL"] = "No Minimum FL";
			passed[4] = true;
		}

		if (conditions[i].HasMember("max_fl") && (max_fl = conditions[i]["max_fl"].GetInt()) > 0) {
			if ((RFL / 100) <= max_fl) {
				returnValid["MAX_FL"] = "at or below FL" + to_string(conditions[i]["max_fl"].GetInt()) + " (Passed)";
				passed[5] = true;
			}
			else {
				returnValid["MAX_FL"] = "at or below FL" + to_string(max_fl) + " (Failed)";
			}
		}
		else {
			returnValid["MAX_FL"] = "No Maximum FL";
			passed[5] = true;
		}

		// Flight level allocation scheme
		// Does Condition contain our first airway if it's limited
		string allowed_fls = conditions[i]["FLAS"].GetString();
		if (conditions[i]["allowed_fls"].IsArray() && conditions[i]["allowed_fls"].Size()) {
			if (routeContains(to_string(RFL / 100), conditions[i]["allowed_fls"])) {
				returnValid["ALLOWED_FL"] =  allowed_fls + " (Passed)";
				passed[6] = true;
			}
			else {
				returnValid["ALLOWED_FL"] = allowed_fls + " (Failed)";
			}
		}
		else {
			returnValid["ALLOWED_FL"] = "No FLAS";
			passed[6] = true;
		}

		// Special navigation requirements needed
		if (conditions[i]["navigation"].IsString()) {
			string navigation_constraints(conditions[i]["navigation"].GetString());
			if (string::npos == navigation_constraints.find_first_of(flightPlan.GetFlightPlanData().GetCapibilities())) {
				returnValid["NAVIGATION"] = "Failed navigation capability restr. Needed: " + navigation_constraints;
				passed[7] = false;
			}
			else {
				returnValid["NAVIGATION"] = "No navigation capability restr";
				passed[7] = true;
			}
		}
		else {
			returnValid["NAVIGATION"] = "";
			passed[7] = true;
		}

		bool passedVeri{ false };
		for (int i = 0; i < 8; i++) {
			if (passed[i])
			{
				passedVeri = true;
			}
			else {
				passedVeri = false;
				break;
			}
		}
		if (passedVeri) {
			returnValid["STATUS"] = "Passed";
			break;
		}
		else {
			returnValid["STATUS"] = "Failed";
			if (!passed[0] || !passed[1])
				continue;
			else
				break;
		}
		
	}
	
	if (!valid) {
		returnValid["SID"] = "No valid SID found!";
		returnValid["STATUS"] = "Failed";
	}
	
	return returnValid;
}

//
void CVFPCPlugin::OnFunctionCall(int FunctionId, const char * ItemString, POINT Pt, RECT Area) {
	CFlightPlan fp = FlightPlanSelectASEL();
	
	if (FunctionId == TAG_FUNC_CHECKFP_MENU) {
		OpenPopupList(Area, "Check FP", 1);
		//AddPopupListElement("Show All Checks", "", TAG_FUNC_CHECKFP_CHECK, false, 2, false);
		AddPopupListElement("Show FLAS", "", TAG_FUNC_CHECKFP_FLAS, false, 2, false);

		if (find(AircraftIgnore.begin(), AircraftIgnore.end(), fp.GetCallsign()) != AircraftIgnore.end())
			AddPopupListElement("Enable", "", TAG_FUNC_ON_OFF, false, 2, false);
		else
			AddPopupListElement("Disable", "", TAG_FUNC_ON_OFF, false, 2, false);
	}
	if (FunctionId == TAG_FUNC_CHECKFP_CHECK) {
		checkFPDetail();
	}
	if (FunctionId == TAG_FUNC_CHECKFP_FLAS) {
		checkFLAS();
	}
	if (FunctionId == TAG_FUNC_ON_OFF) {
		if (find(AircraftIgnore.begin(), AircraftIgnore.end(), fp.GetCallsign()) != AircraftIgnore.end())
			AircraftIgnore.erase(remove(AircraftIgnore.begin(), AircraftIgnore.end(), fp.GetCallsign()), AircraftIgnore.end());
		else
			AircraftIgnore.emplace_back(fp.GetCallsign());

	}
}

// Get FlightPlan, and therefore get the first waypoint of the flightplan (ie. SID). Check if the (RFL/1000) corresponds to the SID Min FL and report output "OK" or "FPL"
void CVFPCPlugin::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	*pColorCode = TAG_COLOR_RGB_DEFINED;
	if (ItemCode == TAG_ITEM_FPCHECK)
	{
		if (strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V") > -1) {
			*pRGB = TAG_CYAN;
			strcpy_s(sItemString, 16, "VFR");
		}
		else {
			map<string, string> messageBuffer = validizeSid(FlightPlan);

			if (find(AircraftIgnore.begin(), AircraftIgnore.end(), FlightPlan.GetCallsign()) != AircraftIgnore.end()) {
				*pRGB = TAG_GREY;
				strcpy_s(sItemString, 16, "-");
			}
			else if (messageBuffer["STATUS"] == "Passed") {
				*pRGB = TAG_GREEN;
				strcpy_s(sItemString, 16, "OK!");
			}
			else {
				string code; int count;
				tie(code, count) = getFails(messageBuffer);

				if (count == 100)
					*pRGB = TAG_RED;
				else if (count != 1) {
					*pRGB = TAG_YELLOW;
				}
				else
				*pRGB = TAG_GREEN;
				strcpy_s(sItemString, 16, code.c_str());
			}
			
		}
	}
	else if ((ItemCode == TAG_ITEM_FPCHECK_IF_FAILED || ItemCode == TAG_ITEM_FPCHECK_IF_FAILED_STATIC) && FlightPlan.GetFlightPlanData().GetPlanType() != "V")
	{
		map<string, string> messageBuffer = validizeSid(FlightPlan);

		if (find(AircraftIgnore.begin(), AircraftIgnore.end(), FlightPlan.GetCallsign()) == AircraftIgnore.end() &&
			messageBuffer["STATUS"] != "Passed") {
			*pRGB = TAG_RED;

			if (ItemCode == TAG_ITEM_FPCHECK_IF_FAILED) {
				string code; int count;
				tie(code, count) = getFails(messageBuffer);
				strcpy_s(sItemString, 16, code.c_str());
			}
			else
				strcpy_s(sItemString, 16, "E");
		}
	}
}

//
void CVFPCPlugin::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	AircraftIgnore.erase(remove(AircraftIgnore.begin(), AircraftIgnore.end(), FlightPlan.GetCallsign()), AircraftIgnore.end());	
}

//
bool CVFPCPlugin::OnCompileCommand(const char * sCommandLine) {
	if (startsWith(".vfpc reload", sCommandLine))
	{
		sendMessage("Unloading all loaded SIDs...");
		sidName.clear();
		sidEven.clear();
		sidMin.clear();
		sidMax.clear();
		initialSidLoad = false;
		return true;
	}
	if (startsWith(".vfpc debug", sCommandLine)) {
		if (debugMode) {
			debugMessage("DebugMode", "Deactivating Debug Mode!");
			debugMode = false;
		} else {
			debugMode = true;
			debugMessage("DebugMode", "Activating Debug Mode!");
		}
		return true;
	}
	if (startsWith(".vfpc load", sCommandLine)) {
		locale loc;
		string buffer{ sCommandLine };
		buffer.erase(0, 11);
		getSids();
		return true;
	}
	if (startsWith(".vfpc check", sCommandLine))
	{
		checkFPDetail();
		return true;
	}
	return false;
}

// the original checking algorithm, unused in hk
void CVFPCPlugin::checkFPDetail() {	
	map<string, string> messageBuffer = validizeSid(FlightPlanSelectASEL());
	string buffer{};

	if (messageBuffer.find("SEARCH") == messageBuffer.end()) {
		buffer += messageBuffer["STATUS"] + " SID " + messageBuffer["SID"] + ": ";

		map<string, string>::iterator it;
		for (it = messageBuffer.begin(); it != messageBuffer.end(); it++)
		{
			if (it->first == "CS" || it->first == "STATUS" || it->first == "SID")
				continue;
			buffer += it->second + ", ";
		}
	} else {
		buffer = "Failed: " + messageBuffer["SEARCH"];
	}

	sendMessage(messageBuffer["CS"], buffer);
}

//sends valid FLs for route
void CVFPCPlugin::checkFLAS() {
	map<string, string> messageBuffer = validizeSid(FlightPlanSelectASEL());
	string buffer{};

	buffer = messageBuffer["ALLOWED_FL"];
	if (buffer == "No FLAS") {
		if (messageBuffer["DIRECTION"] != "") {
			buffer = messageBuffer["DIRECTION"] + ", ";
		}
		if (messageBuffer["MIN_FL"] != "No Minimum FL") {
			buffer += messageBuffer["MIN_FL"] + ", ";
		}
		if (messageBuffer["MAX_FL"] != "No Maximum FL") {
			buffer += messageBuffer["MAX_FL"];
		}
	}
	else if (buffer == "") {
		buffer = "Unable to find altitude restriction for route! Please check manually.";
	}
	sendMessage("Valid FLs for " + messageBuffer["CS"], buffer);
}

pair<string, int> CVFPCPlugin::getFails(map<string, string> messageBuffer) {
	vector<string> fails;
	int failCode = 0;
	
	//fails.push_back("FPL");

	/*if (messageBuffer.find("STATUS") != messageBuffer.end()) {
		fails.push_back("SID");
	}
	if (messageBuffer["DESTINATION"].find_first_of("Failed") == 0) {
		fails.push_back("DST");
	}
	if (messageBuffer["AIRWAYS"].find_first_of("Failed") == 0) {
		fails.push_back("AWY");
	}*/

	if (messageBuffer["DIRECTION"].find("Failed") != std::string::npos) {
		fails.push_back("E/O");
		failCode = 2;
	}
	if (messageBuffer["MIN_FL"].find("Failed") != std::string::npos) {
		fails.push_back("MIN");
		failCode = 3;
	}
	if (messageBuffer["MAX_FL"].find("Failed") != std::string::npos) {
		fails.push_back("MAX");
		failCode = 4;
	}
	if (messageBuffer["ALLOWED_FL"].find("Failed") != std::string::npos) {
		fails.push_back("FLR");
		failCode = 5;
	}

	/*if (messageBuffer["NAVIGATION"].find_first_of("Failed") == 0) {
		fails.push_back("NAV");
		failCode++;
	}*/
	if (failCode == 0) {
		if (messageBuffer["NOISE"].find("Noise") != std::string::npos) {
			fails.push_back("NAP");
			failCode = 1;
		} else {
			fails.push_back("CHK");
			failCode = 100;
		}
	}

	std::size_t couldnt = disCount;
	while (couldnt >= fails.size())
		couldnt -= fails.size();
	return pair<string, int>(fails[couldnt], failCode);
}

void CVFPCPlugin::OnTimer(int Counter) {
	blink = !blink;

	if (blink) {
		if (disCount < 3) {
			disCount++;
		}
		else {
			disCount = 0;
		}
	}

	// Loading proper Sids, when logged in
	if (GetConnectionType() != CONNECTION_TYPE_NO && !initialSidLoad) {
		string callsign{ ControllerMyself().GetCallsign() };
		getSids();
		initialSidLoad = true;
	} else if (GetConnectionType() == CONNECTION_TYPE_NO && initialSidLoad) {
		sidName.clear();
		sidEven.clear();
		sidMin.clear();
		sidMax.clear();
		initialSidLoad = false;
		sendMessage("Unloading", "All loaded SIDs");
	}
}