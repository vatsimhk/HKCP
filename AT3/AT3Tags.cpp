#include "stdafx.h"
#include "AT3Tags.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include "MAESTROapi.h"
#include <string>
#include <array>
#include <chrono>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace EuroScopePlugIn;

AT3Tags::AT3Tags(COLORREF colorA, COLORREF colorNA, COLORREF colorR, COLORREF colorV) : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT)
{
	RegisterTagItemType("AT3 Altitude", TAG_ITEM_AT3_ALTITUDE);
	RegisterTagItemType("AT3 Assigned Altitude", TAG_ITEM_AT3_ALTITUDE_ASSIGNED);
	RegisterTagItemType("AT3 Track", TAG_ITEM_AT3_TRACK);
	RegisterTagItemType("AT3 Assigned Heading", TAG_ITEM_AT3_HEADING_ASSIGNED);
	RegisterTagItemType("AT3 Ground Speed", TAG_ITEM_AT3_SPEED);
	RegisterTagItemType("AT3 Assigned Speed (IAS)", TAG_ITEM_AT3_SPEED_ASSIGNED);
	RegisterTagItemType("AT3 Route Code", TAG_ITEM_AT3_ROUTE_CODE);
	RegisterTagItemType("AT3 APP/DEP Line 4", TAG_ITEM_AT3_APPDEP_LINE4);
	RegisterTagItemType("AT3 AMC Line 4", TAG_ITEM_AT3_AMC_LINE4);
	RegisterTagItemType("AT3 ETA", TAG_ITEM_AT3_ETA);
	RegisterTagItemType("AT3 Callsign", TAG_ITEM_AT3_CALLSIGN);
	RegisterTagItemType("AT3 ADS-B Callsign", TAG_ITEM_AT3_ADSB_CALLSIGN);
	RegisterTagItemType("AT3 ATYP + WTC", TAG_ITEM_AT3_ATYPWTC);
	RegisterTagItemType("AT3 VS Indicator", TAG_ITEM_AT3_VS_INDICATOR);
	RegisterTagItemType("AT3 Arrival Runway", TAG_ITEM_AT3_ARRIVAL_RWY);
	RegisterTagItemType("AT3 AMAN Delay", TAG_ITEM_AT3_DELAY);
	RegisterTagItemType("AT3 ALRT", TAG_ITEM_AT3_ALRT);
	RegisterTagItemType("AT3 WTG", TAG_ITEM_AT3_WTG);

	RegisterTagItemFunction("AT3 Approach Selection Menu", TAG_FUNC_APP_SEL_MENU);
	RegisterTagItemFunction("AT3 Route Selection Menu", TAG_FUNC_RTE_SEL_MENU);

	colorAssumed = colorA;
	colorNotAssumed = colorNA;
	colorRedundant = colorR;
	colorVFR = colorV;

	char DllPathFile[_MAX_PATH];

	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	string path = DllPathFile;
	path.resize(path.size() - strlen("HKCP.dll"));
	string appPath = path + "HKCPApproaches.json";
	string rtePath = path + "HKCPRoutes.json";
	path.resize(path.size() - strlen("HKCP/"));
	string acftPath = path + "TopSky/ICAO_Aircraft.json";

	try {
		fstream appsFile(appPath);
		appsJson = json::parse(appsFile);
		appsFile.close();
		appsFile.clear();

		fstream rteFile(rtePath);
		rteJson = json::parse(rteFile);
		rteFile.close();
		rteFile.clear();

		fstream acftFile(acftPath);
		if (!acftFile) {
			DisplayUserMessage("HKCP", "HKCP", "Unable to find TopSky/ICAO_Aircraft.json for WTG data", true, true, false, false, false);
		}
		else {
			json acftJson = json::parse(acftFile);
			acftFile.close();
			acftFile.clear();

			for (auto& acft : acftJson) {
				if (!acft.contains("WTG") || !acft.contains("ICAO"))
					continue;

				wtgMap[acft["ICAO"]] = acft["WTG"];
			}

			acftJson = json(); // clear memory
		}

		for (auto& arpt : appsJson.items()) {
			arptSet.insert(arpt.key());
		}
	}
	catch (...) {} //do nothing, other functions will catch it and return BAD DATA
}

string AT3Tags::GetActiveArrRwy(string airport) {
	CSectorElement runway;
	string dest;
	for (runway = SectorFileElementSelectFirst(SECTOR_ELEMENT_RUNWAY); runway.IsValid(); runway = SectorFileElementSelectNext(runway, SECTOR_ELEMENT_RUNWAY)) {
		dest = runway.GetAirportName();
		if (strcmp(runway.GetRunwayName(0), "NAP") == 0) {
			continue;
		}
		else if (dest.substr(0, 4) == airport) {
			if (runway.IsElementActive(false, 0)) {
				string runwayName = runway.GetRunwayName(0);
				return runwayName;
			}
			if (runway.IsElementActive(false, 1)) {
				string runwayName = runway.GetRunwayName(1);
				return runwayName;
			}
		}
	}
	return "";
}

vector<string> AT3Tags::GetAvailableApps(string airport, string runway) {
	vector<string> appsVec;

	try {
		for (auto& app : appsJson[airport][runway]) {
			appsVec.push_back(app[0]);
		}
	}
	catch (...) {}

	return appsVec;
}

vector<string> AT3Tags::GetAvailableRtes(string airport, string runway) {
	vector<string> rteVec;

	CFlightPlan FlightPlan = FlightPlanSelectASEL();

	string fpRteStr = FlightPlan.GetFlightPlanData().GetRoute();
	string rwyConfig = runway.substr(0, 2);
	try {
		for (string code : rteJson[airport][rwyConfig]["defaults"]) {
			rteVec.push_back(rteJson[airport][rwyConfig]["routes"][code]["string"]); //all acft for the airport will have these routes regardless of flight plan
		}

		for (auto& config : rteJson[airport][rwyConfig]["points"].items()) { 
			if (fpRteStr.find(config.key()) != string::npos) { 
				for (string code : config.value()) { 
					rteVec.push_back(rteJson[airport][rwyConfig]["routes"][code]["string"]);
				}	
			}
		}
	}
	catch (...) {}

	return rteVec;
}

void AT3Tags::SetApp(int index, CFlightPlan FlightPlan, vector<string> appsVec) {
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
	string spadItem = "/A/" + appsVec[index] + "/A//";
	spadCurrent.append(spadItem);
	FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
}

void AT3Tags::SetRte(int index, CFlightPlan FlightPlan, vector<string> rteVec, string dest, string destRunway) {
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
	string spadItem = "/R/" + rteVec[index] + "/R//";

	spadCurrent.append(spadItem);
	FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

	try {
		string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[index].substr(0, rteVec[index].find("_"))]["star"];

		if (starStr.length() > 0) {
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			string esAssignedStar = FlightPlan.GetFlightPlanData().GetStarName();

			if (esAssignedStar.length() > 0) { // remove original star
				size_t fpIndex = fpRoute.find(esAssignedStar);
				if (fpIndex != string::npos) {
					fpRoute = fpRoute.substr(0, fpIndex - 1);
				}
				
			}

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				CFlightPlanData flightplan_data = FlightPlan.GetFlightPlanData();
				flightplan_data.SetRoute(fpRoute.c_str()); // assign STAR if not already assigned
				flightplan_data.AmendFlightPlan();
			}
		}
	}
	catch (...) {}
}

void AT3Tags::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType) {
	if (!FlightPlan.IsValid()) {
		return;
	}

	switch (DataType) {
		case CTR_DATA_TYPE_SCRATCH_PAD_STRING:
			GetAssignedAPP(FlightPlan);
			GetRouteCode(FlightPlan);
	}
}

void AT3Tags::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!RadarTarget.IsValid()) {
		return;
	}

	bool isAT3Item = true;

	*pColorCode = TAG_COLOR_RGB_DEFINED;
	*pRGB = colorNotAssumed;

	if (FlightPlan.IsValid()) {
		switch (FlightPlan.GetState()) {
		case FLIGHT_PLAN_STATE_NON_CONCERNED:
			*pRGB = colorNotAssumed;
			break;
		case FLIGHT_PLAN_STATE_NOTIFIED:
			*pRGB = colorNotAssumed;
			break;
		case FLIGHT_PLAN_STATE_COORDINATED:
			*pRGB = colorNotAssumed;
			break;
		case FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED:
			*pRGB = colorRedundant;
			break;
		case FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED:
			*pRGB = colorAssumed;
			break;
		case FLIGHT_PLAN_STATE_ASSUMED:
			*pRGB = colorAssumed;
			break;
		case FLIGHT_PLAN_STATE_REDUNDANT:
			*pRGB = colorRedundant;
			break;
		}
	}
	
	string tagOutput = "";

	switch (ItemCode) {
		case TAG_ITEM_AT3_ALTITUDE:
			tagOutput = GetFormattedAltitude(RadarTarget);
			break;
		case TAG_ITEM_AT3_TRACK:
			tagOutput = GetFormattedTrack(RadarTarget);
			break;
		case TAG_ITEM_AT3_SPEED:
			tagOutput = GetFormattedGroundspeed(RadarTarget);
			break;
		case TAG_ITEM_AT3_VS_INDICATOR:
			tagOutput = GetVSIndicator(RadarTarget);
			break;
		case TAG_ITEM_AT3_ADSB_CALLSIGN:
			tagOutput = GetADSBCallsign(RadarTarget);
			break;
		default:
			tagOutput = "";
			isAT3Item = false;
	}

	if (isAT3Item) {
		strcpy_s(sItemString, 16, tagOutput.substr(0, 15).c_str()); 
		return;
	}

	if (FlightPlan.IsValid()) {
		isAT3Item = true;

		switch (ItemCode) {
			case TAG_ITEM_AT3_ALTITUDE_ASSIGNED:
				tagOutput = GetFormattedAltitudedAssigned(FlightPlan, RadarTarget);
				break;
			case TAG_ITEM_AT3_HEADING_ASSIGNED:
				tagOutput = GetFormattedHeadingAssigned(FlightPlan);
				break;
			case TAG_ITEM_AT3_SPEED_ASSIGNED:
				tagOutput = GetFormattedSpeedAssigned(FlightPlan);
				break;
			case TAG_ITEM_AT3_ROUTE_CODE:
				tagOutput = GetRouteCodeLine4(FlightPlan);
				break;
			case TAG_ITEM_AT3_APPDEP_LINE4:
				tagOutput = GetAPPDEPLine4(FlightPlan);
				break;
			case TAG_ITEM_AT3_AMC_LINE4:
				tagOutput = GetAMCLine4(FlightPlan);
				break;
			case TAG_ITEM_AT3_ETA:
				tagOutput = GetFormattedETA(FlightPlan, minu);
				break;
			case TAG_ITEM_AT3_DELAY:
				tagOutput = GetAMANDelay(FlightPlan);
				break;
			case TAG_ITEM_AT3_CALLSIGN:
				tagOutput = GetCallsign(FlightPlan);
				if (string(FlightPlan.GetFlightPlanData().GetPlanType()) == "V") {
					*pRGB = colorVFR;
				}
				break;
			case TAG_ITEM_AT3_ATYPWTC:
				tagOutput = GetATYPWTC(FlightPlan);
				if (string(FlightPlan.GetFlightPlanData().GetPlanType()) == "V") {
					*pRGB = colorVFR;
				}
				break;
			case TAG_ITEM_AT3_ARRIVAL_RWY:
				tagOutput = GetFormattedArrivalRwy(FlightPlan);
				break;
			case TAG_ITEM_AT3_ALRT:
				tagOutput = GetALRT(FlightPlan);
				*pRGB = colorRedundant;
				break;
			case TAG_ITEM_AT3_WTG:
				tagOutput = GetWTG(FlightPlan);
				break;
			default:
				tagOutput = "";
				isAT3Item = false;
		}
	}

	// Convert string output to character array
	if (isAT3Item) {
		strcpy_s(sItemString, 16, tagOutput.substr(0, 15).c_str());
	}
}

void AT3Tags::OnTimer(int Counter) {
	for (auto& pair : callsignToHandoffTimer) {
		pair.second++;
	}

	if (Counter % 10 != 0) {
		return;
	} else {
		time_t nowt = time(0);
		struct tm* tmp = gmtime(&nowt);

		minu = tmp->tm_min;
	}
}

void AT3Tags::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	CFlightPlan FlightPlan = FlightPlanSelectASEL();
	if (!FlightPlan.IsValid()) {
		return;
	}

	string dest = FlightPlan.GetFlightPlanData().GetDestination();
	string destRunway = FlightPlan.GetFlightPlanData().GetArrivalRwy();
	string STAR = FlightPlan.GetFlightPlanData().GetStarName();

	string APPMenuName = dest + destRunway + " APP RTES";
	string RteMenuName = dest + " RTES";
	vector<string> appsVec;
	vector<string> rteVec;

	if (!dest.empty() && !destRunway.empty()) {
		appsVec = GetAvailableApps(dest, destRunway);
		rteVec = GetAvailableRtes(dest, destRunway);
	} else if (!dest.empty()) {
		destRunway = GetActiveArrRwy(dest);
		appsVec = GetAvailableApps(dest, destRunway);
		rteVec = GetAvailableRtes(dest, destRunway);
	}

	switch (FunctionId) {
	case TAG_FUNC_APP_SEL_MENU:
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) {
			OpenPopupList(Area, APPMenuName.c_str(), 1);
			if (appsVec.size() > 0) {
				AddPopupListElement(appsVec[0].c_str(), "", TAG_FUNC_APP_SEL_ITEM_1, false, 2, false);
			}
			else {
				AddPopupListElement("BAD DATA     ", "", TAG_FUNC_APP_SEL_DUMMY, false, 2, false);
				break;
			}

			if (appsVec.size() > 1) {
				AddPopupListElement(appsVec[1].c_str(), "", TAG_FUNC_APP_SEL_ITEM_2, false, 2, false);
			}
			else {
				break;
			}

			if (appsVec.size() > 2) {
				AddPopupListElement(appsVec[2].c_str(), "", TAG_FUNC_APP_SEL_ITEM_3, false, 2, false);
			}
			else {
				break;
			}

			if (appsVec.size() > 3) {
				AddPopupListElement(appsVec[3].c_str(), "", TAG_FUNC_APP_SEL_ITEM_4, false, 2, false);
			}
			else {
				break;
			}

			if (appsVec.size() > 4) {
				AddPopupListElement(appsVec[4].c_str(), "", TAG_FUNC_APP_SEL_ITEM_5, false, 2, false);
			}
			else {
				break;
			}

			if (appsVec.size() > 5) {
				AddPopupListElement(appsVec[5].c_str(), "", TAG_FUNC_APP_SEL_ITEM_6, false, 2, false);
			}
			else {
				break;
			}

			if (appsVec.size() > 6) {
				AddPopupListElement(appsVec[6].c_str(), "", TAG_FUNC_APP_SEL_ITEM_7, false, 2, false);
			}
			else {
				break;
			}

			if (appsVec.size() > 7) {
				AddPopupListElement(appsVec[7].c_str(), "", TAG_FUNC_APP_SEL_ITEM_8, false, 2, false);
			}
			else {
				break;
			}
		}
		else {
			break;
		}

	case TAG_FUNC_APP_SEL_ITEM_1: {
		SetApp(0, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_2: {
		SetApp(1, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_3: {
		SetApp(2, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_4: {
		SetApp(3, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_5: {
		SetApp(4, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_6: {
		SetApp(5, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_7: {
		SetApp(6, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_APP_SEL_ITEM_8: {
		SetApp(7, FlightPlan, appsVec);
		break;
	}
	case TAG_FUNC_RTE_SEL_MENU:
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) {
			OpenPopupList(Area, RteMenuName.c_str(), 1);
			if (rteVec.size() > 0) {
				AddPopupListElement(rteVec[0].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_1, false, 2, false);
			}
			else {
				AddPopupListElement("BAD DATA     ", "", TAG_FUNC_RTE_SEL_DUMMY, false, 2, false);
				break;
			}

			if (rteVec.size() > 1) {
				AddPopupListElement(rteVec[1].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_2, false, 2, false);
			}
			else {
				break;
			}

			if (rteVec.size() > 2) {
				AddPopupListElement(rteVec[2].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_3, false, 2, false);
			}
			else {
				break;
			}

			if (rteVec.size() > 3) {
				AddPopupListElement(rteVec[3].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_4, false, 2, false);
			}
			else {
				break;
			}

			if (rteVec.size() > 4) {
				AddPopupListElement(rteVec[4].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_5, false, 2, false);
			}
			else {
				break;
			}

			if (rteVec.size() > 5) {
				AddPopupListElement(rteVec[5].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_6, false, 2, false);
			}
			else {
				break;
			}

			if (rteVec.size() > 6) {
				AddPopupListElement(rteVec[6].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_7, false, 2, false);
			}
			else {
				break;
			}

			if (rteVec.size() > 7) {
				AddPopupListElement(rteVec[7].c_str(), "", TAG_FUNC_RTE_SEL_ITEM_8, false, 2, false);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_1: {
			SetRte(0, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_2: {
			SetRte(1, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_3: {
			SetRte(2, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_4: {
			SetRte(3, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_5: {
			SetRte(4, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_6: {
			SetRte(5, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_7: {
			SetRte(6, FlightPlan, rteVec, dest, destRunway);
			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_8: {
			SetRte(7, FlightPlan, rteVec, dest, destRunway);
			break;
		}
	}
}

void AT3Tags::SplitCallsign(const std::string& callsign, std::string& prefix, std::string& number) {
	prefix = "";
	number = "";
	for (char c : callsign) {
		// All letters belongs to prefix
		if (std::isalpha(c) && number.empty()) {
			prefix += c;
		}
		// Rest belongs to number
		else {
			number += c;
		}
	}
}

bool AT3Tags::isSimilarCallsign(const std::string & CurrentPrefix, std::string & CurrentNum, std::string & otherPrefix, std::string & otherNum) {
	int matchCount = 0;
	string tempNum2 = otherNum;
	for (char c : CurrentNum) {
		size_t pos = tempNum2.find(c);
		if (pos != std::string::npos) {
			matchCount++;
			tempNum2.erase(pos, 1);
		}
	}
	float longerNumLength = max(CurrentNum.length(), otherNum.length());
	float numSimilarity = matchCount / longerNumLength;

	if (CurrentNum == otherNum) {
		return true;
	}
	else if (CurrentPrefix == otherPrefix && numSimilarity > 0.5) {
		return true;
	}
	else {
		return false;
	}
}

bool AT3Tags::isCorrelateCorrect(CFlightPlan FlightPlan, string CurrentCallsign) {
	string correlatedCallsign = FlightPlan.GetCorrelatedRadarTarget().GetCallsign();
	if (CurrentCallsign != correlatedCallsign) {
		return false;
	}
}

string AT3Tags::GetFormattedAltitude(CRadarTarget& RadarTarget)
{
	int altitude = RadarTarget.GetPosition().GetPressureAltitude();
	int transAlt = GetTransitionAltitude();
	string formattedAlt;

	if (altitude > transAlt) {
		int flightLevel = RadarTarget.GetPosition().GetFlightLevel();
		formattedAlt = to_string((flightLevel + 50) / 100); //rough rounding, +50 to force round up
		if (formattedAlt.length() < 3)
		{
			formattedAlt.insert(0, 3 - formattedAlt.length(), '0');
		}
		formattedAlt.insert(0, "F");
	} else {
		formattedAlt = to_string((altitude + 50) / 100);
		if (formattedAlt.length() < 3)
		{
			formattedAlt.insert(0, 3 - formattedAlt.length(), '0');
		}
		formattedAlt.insert(0, "A");
	}

	return formattedAlt;
}

string AT3Tags::GetFormattedAltitudedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	int altAssigned = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
	int transAlt = GetTransitionAltitude();
	int finalAltAssigned = FlightPlan.GetFinalAltitude();
	string formattedAltAssigned;

	if (altAssigned > 2) {
		if (altAssigned > transAlt) {
			formattedAltAssigned = to_string((altAssigned + 50) / 100);
			if (to_string((RadarTarget.GetPosition().GetFlightLevel() + 50) / 100) == formattedAltAssigned) {
				formattedAltAssigned = "    ";
			}
			else if (formattedAltAssigned.length() <= 3 && formattedAltAssigned != "") {
				formattedAltAssigned.insert(0, 3 - formattedAltAssigned.length(), '0');
				formattedAltAssigned.insert(0, "F");
			}
		}
		else {
			formattedAltAssigned = to_string((altAssigned + 50) / 100);
			if (to_string((RadarTarget.GetPosition().GetPressureAltitude() + 50) / 100) == formattedAltAssigned) {
				formattedAltAssigned = "    ";
			}
			else if (formattedAltAssigned.length() <= 3 && formattedAltAssigned != "") {
				formattedAltAssigned.insert(0, 3 - formattedAltAssigned.length(), '0');
				formattedAltAssigned.insert(0, "A");
			}
		}
	}
	else if (altAssigned >= 0 && altAssigned <= 2) {
		switch (altAssigned) {
			case 0: //ES defaults to CFL 0 when CFL = RFL
				formattedAltAssigned = to_string((finalAltAssigned + 50) / 100);
				if (finalAltAssigned != 0 && !(to_string((RadarTarget.GetPosition().GetPressureAltitude() + 50) / 100) == formattedAltAssigned || to_string((RadarTarget.GetPosition().GetFlightLevel() + 50) / 100) == formattedAltAssigned)) {
					if (formattedAltAssigned.length() <= 3 && formattedAltAssigned != "") {
						formattedAltAssigned.insert(0, 3 - formattedAltAssigned.length(), '0');
					}
					if (finalAltAssigned > transAlt) {
						formattedAltAssigned.insert(0, "F");
					}
					else {
						formattedAltAssigned.insert(0, "A");
					}
				}
				else {
					formattedAltAssigned = "    ";
				}
				break;
			case 1:
				formattedAltAssigned = "A   ";
				break;
			case 2:
				formattedAltAssigned = "V   ";
				break;

		}
	}
	else {
		formattedAltAssigned = "9999";
	}

	return formattedAltAssigned;
}

string AT3Tags::GetFormattedTrack(CRadarTarget& RadarTarget)
{
	string track = to_string(static_cast<int>(trunc(RadarTarget.GetTrackHeading())));

	if (track == "0") {
		track = "360";
	}

	if (track.length() <= 3) {
		track.insert(0, 3 - track.length(), '0');
	}
	return track;
}

string AT3Tags::GetFormattedHeadingAssigned(CFlightPlan& FlightPlan)
{
	string headingAssigned = to_string(FlightPlan.GetControllerAssignedData().GetAssignedHeading());
	if (headingAssigned == "0") {
		headingAssigned = "    ";
	} else if (headingAssigned.length() <= 3) {
		headingAssigned.insert(0, 3 - headingAssigned.length(), '0');
		headingAssigned.insert(0, "H");
	} else {
		headingAssigned = "999";
	}
	return headingAssigned;
}

string AT3Tags::GetFormattedGroundspeed(CRadarTarget& RadarTarget)
{
	string groundSpeed = to_string((RadarTarget.GetGS() + 5) / 10);
	if (groundSpeed.length() <= 2) {
		groundSpeed.insert(0, 2 - groundSpeed.length(), '0');
	}
	return groundSpeed;
}

string AT3Tags::GetFormattedSpeedAssigned(CFlightPlan& FlightPlan)
{
	string speedAssigned;

	if (FlightPlan.GetControllerAssignedData().GetAssignedSpeed() == 999) {
		speedAssigned = "SHS ";
		return speedAssigned;
	} else if (FlightPlan.GetControllerAssignedData().GetAssignedMach() != 0) {
		speedAssigned = to_string(FlightPlan.GetControllerAssignedData().GetAssignedMach());
		if (speedAssigned.length() <= 2) {
			speedAssigned.insert(0, 2 - speedAssigned.length(), '0');
			speedAssigned.insert(0, "M");
		}
	} else if (FlightPlan.GetControllerAssignedData().GetAssignedMach() == 0 && FlightPlan.GetControllerAssignedData().GetAssignedSpeed() > 0) {
		speedAssigned = to_string((FlightPlan.GetControllerAssignedData().GetAssignedSpeed() + 5) / 10);
		if (speedAssigned.length() <= 2) {
			speedAssigned.insert(0, 2 - speedAssigned.length(), '0');
			speedAssigned.insert(0, "S");
		}
	} else if (FlightPlan.GetControllerAssignedData().GetAssignedSpeed() == 0) {
		speedAssigned = "    ";
	} else {
		speedAssigned = "999 ";
	}

	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(7); //find TopSky "+" or "-" speed conditions

	if (flightStrip.find("/s+/") != string::npos) {
		speedAssigned.append("+");
	} else if (flightStrip.find("/s-/") != string::npos) {
		speedAssigned.append("-");
	} else {
		speedAssigned.append(" ");
	}

	return speedAssigned;
}

void AT3Tags::GetRouteCode(CFlightPlan& FlightPlan) {
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();

	size_t startContainer = spadCurrent.find("/R/");
	size_t endContainer = spadCurrent.find("/R//");

	if (startContainer != string::npos && endContainer != string::npos) {
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(3, "");
		string rteCode = spadCurrent.substr(startContainer, endContainer + 4);
		spadCurrent.erase(startContainer, (endContainer + 4) - startContainer);
		FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(3, rteCode.c_str());
	}
}

string AT3Tags::GetRouteCodeLine4(CFlightPlan& FlightPlan)
{
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(3);
	string runway = FlightPlan.GetFlightPlanData().GetArrivalRwy();
	string lineStr;
 
	size_t startContainer = flightStrip.find("/R/");
	size_t endContainer = flightStrip.find("/R//");

	try {
		if (startContainer != string::npos && endContainer != string::npos && flightStrip.find("_") != string::npos) {
			if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) { //matches dest with json in case of dest changes after rte assignment
				lineStr = flightStrip.substr(startContainer + 3, endContainer - 3); //always get route from spad/flight strip in case of version mismatch
				lineStr = lineStr.substr(0, lineStr.find("_"));
			}
		}
		else if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end() && strlen(FlightPlan.GetFlightPlanData().GetArrivalRwy()) != 0) {

			// resolve flightplan route and remove stepclimb info to avoid the interference
			vector<string> route = split(FlightPlan.GetFlightPlanData().GetRoute(), ' ');
			string fpRoute = "";
			for (std::size_t i = 0; i < route.size(); i++) {
				if (i != 0 && i != route.size()) {
					route[i] = route[i].substr(0, route[i].find_first_of('/'));
				}
				boost::to_upper(route[i]);
				fpRoute = fpRoute + route[i] + " ";
			}

			for (auto& rte : rteJson[FlightPlan.GetFlightPlanData().GetDestination()][runway.substr(0, 2)]["routes"].items()) { //matches exact route only for auto assigning route code
				if (fpRoute.find(rte.value()["route"]) != string::npos) {
					lineStr = rte.key();
				}
			}
		}
	}
	catch (...) {
		lineStr = "   ";
	}

	if (lineStr.length() < 3) {
		lineStr.insert(lineStr.length(), 3 - lineStr.length(), ' ');

	}

	return lineStr;
}

void AT3Tags::GetAssignedAPP(CFlightPlan& FlightPlan) {
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();

	size_t startContainer = spadCurrent.find("/A/");
	size_t endContainer = spadCurrent.find("/A//");

	if (startContainer != string::npos && endContainer != string::npos) {
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, "");
		string app = spadCurrent.substr(startContainer, endContainer + 4);
		spadCurrent.erase(startContainer, (endContainer + 4) - startContainer);
		FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, app.c_str());
	}
}

string AT3Tags::GetAPPDEPLine4(CFlightPlan& FlightPlan)
{
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(2);
	string lineStr;

	size_t startContainer = flightStrip.find("/A/");
	size_t endContainer = flightStrip.find("/A//");

	if (startContainer != string::npos && endContainer != string::npos && flightStrip.find("_") != string::npos) {
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) { //matches dest with json in case of dest changes after app assignment
			lineStr = flightStrip.substr(startContainer + 3, endContainer - 1);
			lineStr = lineStr.substr(0, lineStr.find("_"));  //always get route from spad/flight strip in case of version mismatch
		}
	}
	else if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end() && strlen(FlightPlan.GetFlightPlanData().GetArrivalRwy()) != 0) {
		vector<string> appsVec = GetAvailableApps(FlightPlan.GetFlightPlanData().GetDestination(), FlightPlan.GetFlightPlanData().GetArrivalRwy());
		if (appsVec.size() > 0) {
			string app = appsVec[0]; //selects default app if no assignment, which is [0]
			if (app.find("_") != string::npos) {
				lineStr = app.substr(0, app.find("_"));
				string spadItem = "/A/" + app + "/A//";
				FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, spadItem.c_str());
			}
		}
	}

	if (lineStr.length() < 1) {
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetOrigin()) != arptSet.end() && strlen(FlightPlan.GetFlightPlanData().GetSidName()) != 0) {
			lineStr = FlightPlan.GetFlightPlanData().GetSidName();
		}
		else {
			lineStr = "APP RTE >";
		}
	}

	return lineStr;
}

string AT3Tags::GetAMCLine4(CFlightPlan& FlightPlan)
{
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(2);
	string lineStr;

	size_t startContainer = flightStrip.find("/A/");
	size_t endContainer = flightStrip.find("/A//");

	if (startContainer != string::npos && endContainer != string::npos && flightStrip.find("_") != string::npos) {
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) { //matches dest with json in case of dest changes after app assignment
			lineStr = flightStrip.substr(startContainer + 3, endContainer - 1);
			lineStr = lineStr.substr(0, lineStr.find("_"));
		}
	}
	else if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end() && strlen(FlightPlan.GetFlightPlanData().GetArrivalRwy()) != 0) {
		vector<string> appsVec = GetAvailableApps(FlightPlan.GetFlightPlanData().GetDestination(), FlightPlan.GetFlightPlanData().GetArrivalRwy());
		if (appsVec.size() > 0) {
			string app = appsVec[0]; //selects default app if no assignment, which is [0]
			if (app.find("_") != string::npos) {
				lineStr = app.substr(0, app.find("_"));
				string spadItem = "/A/" + app + "/A//";
				FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, spadItem.c_str());
			}
		}
	}

	if (lineStr.length() < 1) {
		lineStr = "APP RTE >";
	}

	return lineStr;
}

string AT3Tags::GetFormattedETA(CFlightPlan& FlightPlan, int minutes)
{
	try {
		string runway = FlightPlan.GetFlightPlanData().GetArrivalRwy();

		string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(3);

		size_t startContainer = flightStrip.find("/R/");
		size_t endContainer = flightStrip.find("/R//");
		string rteCode = flightStrip.substr(startContainer + 3, endContainer - 3);
		rteCode = rteCode.substr(0, rteCode.find("_"));

		int timeToGate = -1;

		string gate = rteJson[FlightPlan.GetFlightPlanData().GetDestination()][runway.substr(0, 2)]["routes"][rteCode]["gate"];
		
		if (gate.length() > 0) { 
			string prefix = gate.substr(0, 1);

			if (gate != "R") { //directly skip to runway calculation
				for (int i = FlightPlan.GetExtractedRoute().GetPointsNumber() - 1; i >= 0; i--) { //iterate from end to start for STAR waypoints
					if (FlightPlan.GetExtractedRoute().GetPointName(i - 1) == gate) {
						timeToGate = FlightPlan.GetExtractedRoute().GetPointDistanceInMinutes(i - 1);
						break;
					}
				}
			}

			if (timeToGate < 0) {
				timeToGate = FlightPlan.GetExtractedRoute().GetPointDistanceInMinutes(FlightPlan.GetExtractedRoute().GetPointsNumber() - 1); //time to dest
				prefix = "R";
			}
			timeToGate = minutes + timeToGate;

			if (timeToGate > 59) {
				timeToGate = timeToGate - 60;
			}

			string timeStr = to_string(timeToGate);

			if (timeStr.length() < 2) {
				timeStr.insert(0, 2 - timeStr.length(), '0');
			}

			timeStr.insert(0, prefix);

			return timeStr;
		}
		else { //gates may be left empty, e.g. VMMC PRC STARs
			return "   ";
		}
	}
	catch (...) {
		return "   ";
	}
}

string AT3Tags::GetAMANDelay(CFlightPlan& FlightPlan) 
{
	int delay = trunc(GetCurrentDelay(FlightPlan.GetCallsign()));
	if (delay == 0) {
		return "  ";
	} else if (delay > 0) {
		return "+" + to_string(delay);
	}
	else if (delay < 0) {
		return to_string(delay);
	}
}

string AT3Tags::GetCallsign(CFlightPlan& FlightPlan)
{
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(5); //find TopSky "/cpdlc/" annotations

	if (flightStrip.find("/cpdlc/") != string::npos || flightStrip.find("/cpdlc-/") != string::npos) {
		string callsignStr = FlightPlan.GetCallsign();
		return "[" + callsignStr + "]";
	} else {
		return FlightPlan.GetCallsign();
	}
}

string AT3Tags::GetADSBCallsign(CRadarTarget& RadarTarget)
{
	return RadarTarget.GetCallsign();
}

string AT3Tags::GetATYPWTC(CFlightPlan& FlightPlan)
{
	string ATYPWTC = "";
	ATYPWTC += FlightPlan.GetFlightPlanData().GetAircraftFPType();
	if (ATYPWTC.length() < 4) {
		ATYPWTC.insert(ATYPWTC.length(), 4 - ATYPWTC.length(), ' ');
	}
	ATYPWTC += FlightPlan.GetFlightPlanData().GetAircraftWtc();
	return ATYPWTC;
}

string AT3Tags::GetVSIndicator(CRadarTarget& RadarTarget)
{
	string vsIndicator;
	if (RadarTarget.GetVerticalSpeed() > 200) {
		vsIndicator = "^";
	} else if (RadarTarget.GetVerticalSpeed() < -200) {
		vsIndicator = "|";
	} else {
		vsIndicator = " ";
	}
	return vsIndicator;
}

string AT3Tags::GetFormattedArrivalRwy(CFlightPlan& FlightPlan)
{
	if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) {
		string runway = FlightPlan.GetFlightPlanData().GetArrivalRwy();
		if (runway.length() < 3) {
			runway.insert(runway.length(), 3 - runway.length(), ' ');
		}
		return runway;
	} else {
		return "   ";
	}
}

string AT3Tags::GetALRT(CFlightPlan& FlightPlan)
{
	string CurrentCallsign = FlightPlan.GetCallsign();

	// ID warning
	if (FlightPlan.GetCorrelatedRadarTarget().IsValid()) {
		if (!isCorrelateCorrect(FlightPlan, CurrentCallsign)) {
			return "ID";
		}
	}

	// CC warning
	if (FlightPlan.GetCorrelatedRadarTarget().IsValid()) {
		string receivedSquawk = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetSquawk();
		string assignedSquawk = FlightPlan.GetControllerAssignedData().GetSquawk();

		if (assignedSquawk != "" && receivedSquawk != assignedSquawk) {
			return "CC";
		}
	}

	// HOW warning
	if (FlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED) {
		if (callsignToHandoffTimer.find(CurrentCallsign) == callsignToHandoffTimer.end()) {
			callsignToHandoffTimer[CurrentCallsign] = 0;
		}
		else if (callsignToHandoffTimer[CurrentCallsign] >= HOW_WARNING_TIME) {
			return "HOW";
		}
	}
	else {
		callsignToHandoffTimer.erase(CurrentCallsign);
	}

	// CJS warning
	string controller = FlightPlan.GetCoordinatedNextController();
	int controllerSuffix;

	controllerSuffix = (controller.size() > 0) ? ControllerSelect(controller.c_str()).GetFacility() : 0;

	if (FlightPlan.GetSectorExitMinutes() == -1 && FlightPlan.GetTrackingControllerIsMe() && controllerSuffix > 0 && controllerSuffix != 4) { // not unicom or tower
		return "CJS";
	}

	// SCA warning

	std::string CurrentPrefix, CurrentNum;
	SplitCallsign(CurrentCallsign, CurrentPrefix, CurrentNum);

	if (FlightPlan.GetTrackingControllerIsMe() == true) {
		for (EuroScopePlugIn::CRadarTarget otherPlane = RadarTargetSelectFirst(); otherPlane.IsValid(); otherPlane = RadarTargetSelectNext(otherPlane)) {
			if (!otherPlane.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) continue;
			if (otherPlane.GetCallsign() == CurrentCallsign) continue;

			string otherCallsign = otherPlane.GetCallsign();

			std::string Prefix2, Num2;
			SplitCallsign(otherCallsign, Prefix2, Num2);

			if (isSimilarCallsign(CurrentPrefix, CurrentNum, Prefix2, Num2)){
				return "SCA";
			}
		}
	}
	
	return "";
}

string AT3Tags::GetWTG(CFlightPlan& FlightPlan)
{
	if (string(FlightPlan.GetFlightPlanData().GetOrigin()) == "VHHH" || string(FlightPlan.GetFlightPlanData().GetOrigin()) == "VMMC" || string(FlightPlan.GetFlightPlanData().GetOrigin()) == "VHHX") {
		return "";
	}

	unordered_map<string, string> alias;
	alias["A"] = "J";
	alias["B"] = "H";
	alias["C"] = "U";
	alias["D"] = "M";
	alias["E"] = "S";
	alias["F"] = "S";
	alias["G"] = "L";

	string icao = FlightPlan.GetFlightPlanData().GetAircraftFPType();
	auto mapIt = wtgMap.find(icao);
	if (mapIt == wtgMap.end())
		return "";

	auto aliasIt = alias.find(mapIt->second);
	if (aliasIt == alias.end())
		return "";

	return aliasIt->second;
}