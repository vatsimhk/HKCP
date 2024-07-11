#include "stdafx.h"
#include "AT3Tags.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include "MAESTROapi.h"
#include <string>
#include <array>
#include <chrono>
#include <ctime>

using namespace EuroScopePlugIn;

AT3Tags::AT3Tags() : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT)
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
	RegisterTagItemType("AT3 ATYP + WTC", TAG_ITEM_AT3_ATYPWTC);
	RegisterTagItemType("AT3 VS Indicator", TAG_ITEM_AT3_VS_INDICATOR);
	RegisterTagItemType("AT3 Arrival Runway", TAG_ITEM_AT3_ARRIVAL_RWY);
	RegisterTagItemType("AT3 AMAN Delay", TAG_ITEM_AT3_DELAY);

	RegisterTagItemFunction("AT3 Approach Selection Menu", TAG_FUNC_APP_SEL_MENU);
	RegisterTagItemFunction("AT3 Route Selection Menu", TAG_FUNC_RTE_SEL_MENU);

	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	path = DllPathFile;
	path.resize(path.size() - strlen("HKCP.dll"));
	string appPath = path + "HKCPApproaches.json";
	string rtePath = path + "HKCPRoutes.json";

	fstream appsFile(appPath);
	appsJson = json::parse(appsFile);
	appsFile.close();
	appsFile.clear();

	fstream rteFile(rtePath);
	rteJson = json::parse(rteFile);
	rteFile.close();
	rteFile.clear();

	for (auto& arpt : appsJson.items()) {
		arptSet.insert(arpt.key());
	}
}

vector<string> AT3Tags::GetAvailableApps(string airport, string runway) {
	vector<string> appsVec;

	try {
		for (auto& app : appsJson[airport][runway]) {
			appsVec.push_back(app[0]);
		}
	}
	catch (...) {
		appsVec.push_back("BAD DATA     ");
	}

	if (appsVec.size() < 1) {
		appsVec.push_back("BAD DATA     ");
	}

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
	catch (...) {
		rteVec.push_back("BAD DATA     ");
	}

	if (rteVec.size() < 1) {
		rteVec.push_back("BAD DATA     ");
	}

	return rteVec;
}

void AT3Tags::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType) {
	if (!FlightPlan.IsValid()) {
		return;
	}

	switch (DataType) {
		case 5:
			GetAssignedAPP(FlightPlan);
			GetRouteCode(FlightPlan);
	}
}

void AT3Tags::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	time_t nowt = time(0);
	struct tm* tmp = gmtime(&nowt);

	int minu = tmp->tm_min;

	if (!FlightPlan.IsValid() || !RadarTarget.IsValid()) {
		return;
	}

	*pColorCode = TAG_COLOR_RGB_DEFINED;
	switch (FlightPlan.GetState()) {
		case FLIGHT_PLAN_STATE_NON_CONCERNED:
			*pRGB = RGB(117, 132, 142);
			break;
		case FLIGHT_PLAN_STATE_NOTIFIED:
			*pRGB = RGB(117, 132, 142);
			break;
		case FLIGHT_PLAN_STATE_COORDINATED:
			*pRGB = RGB(117, 132, 142);
			break;
		case FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED:
			*pRGB = RGB(229, 214, 130);
			break;
		case FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED:
			*pRGB = RGB(241, 246, 255);
			break;
		case FLIGHT_PLAN_STATE_ASSUMED:
			*pRGB = RGB(241, 246, 255);
			break;
		case FLIGHT_PLAN_STATE_REDUNDANT:
			*pRGB = RGB(229, 214, 130);
			break;
	}
	
	string tagOutput;

	switch (ItemCode) {
		case TAG_ITEM_AT3_ALTITUDE:
			tagOutput = GetFormattedAltitude(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_ALTITUDE_ASSIGNED:
			tagOutput = GetFormattedAltitudedAssigned(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_TRACK:
			tagOutput = GetFormattedTrack(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_HEADING_ASSIGNED:
			tagOutput = GetFormattedHeadingAssigned(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_SPEED:
			tagOutput = GetFormattedGroundspeed(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_SPEED_ASSIGNED:
			tagOutput = GetFormattedSpeedAssigned(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_ROUTE_CODE:
			tagOutput = GetRouteCodeLine4(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_APPDEP_LINE4:
			tagOutput = GetAPPDEPLine4(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_AMC_LINE4:
			tagOutput = GetAMCLine4(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_ETA:
			tagOutput = GetFormattedETA(FlightPlan, RadarTarget, minu);
			break;
		case TAG_ITEM_AT3_DELAY:
			tagOutput = GetAMANDelay(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_CALLSIGN:
			tagOutput = GetCallsign(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_ATYPWTC:
			tagOutput = GetATYPWTC(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_VS_INDICATOR:
			tagOutput = GetVSIndicator(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_ARRIVAL_RWY:
			tagOutput = GetArrivalRwy(FlightPlan, RadarTarget);
			break;
		default:
			tagOutput = "";
	}

	// Convert string output to character array
	strcpy_s(sItemString, 16, tagOutput.substr(0, 15).c_str());
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
	}

	switch (FunctionId) {
		case TAG_FUNC_APP_SEL_MENU:
			OpenPopupList(Area, APPMenuName.c_str(), 1);
			if (appsVec.size() > 0) {
				AddPopupListElement(appsVec[0].c_str(), "", TAG_FUNC_APP_SEL_ITEM_1, false, 2, false);
			} else {
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

		case TAG_FUNC_APP_SEL_ITEM_1: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[0] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str()); 
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_2: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[1] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_3: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[2] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_4: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[3] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_5: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[4] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_6: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[5] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_7: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[6] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_APP_SEL_ITEM_8: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/A/" + appsVec[7] + "/A//";
			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
			break;
		}
		case TAG_FUNC_RTE_SEL_MENU:
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
		case TAG_FUNC_RTE_SEL_ITEM_1: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[0] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[0].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_2: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[1] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[1].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_3: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[2] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[2].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_4: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[3] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[3].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_5: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[4] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[4].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_6: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[5] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[5].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_7: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[6] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[6].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
		case TAG_FUNC_RTE_SEL_ITEM_8: {
			string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
			string spadItem = "/R/" + rteVec[7] + "/R//";

			spadCurrent.append(spadItem);
			FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());

			string starStr = rteJson[dest][destRunway.substr(0, 2)]["routes"][rteVec[7].substr(0, 3)]["star"];
			starStr = " " + starStr + "/" + destRunway;

			string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();

			if (fpRoute.find(starStr) == string::npos) {
				fpRoute = fpRoute + starStr;
				FlightPlan.GetFlightPlanData().SetRoute(fpRoute.c_str()); //assign STAR if not already assigned
			}

			break;
		}
	}
}

string AT3Tags::GetFormattedAltitude(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
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

string AT3Tags::GetFormattedTrack(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string track = to_string(static_cast<int>(trunc(RadarTarget.GetTrackHeading())));
	if (track.length() <= 3) {
		track.insert(0, 3 - track.length(), '0');
	}
	return track;
}

string AT3Tags::GetFormattedHeadingAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
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

string AT3Tags::GetFormattedGroundspeed(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string groundSpeed = to_string((RadarTarget.GetGS() + 5) / 10);
	if (groundSpeed.length() <= 2) {
		groundSpeed.insert(0, 2 - groundSpeed.length(), '0');
	}
	return groundSpeed;
}

string AT3Tags::GetFormattedSpeedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
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

	if (spadCurrent.find("/R/") != string::npos && spadCurrent.find("/R//") != string::npos) {
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(3, "");
		size_t startContainer = spadCurrent.find("/R/");
		size_t endContainer = spadCurrent.find("/R//");
		string rteCode = spadCurrent.substr(startContainer, endContainer + 4);
		spadCurrent.erase(startContainer, (endContainer + 4) - startContainer);
		FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(3, rteCode.c_str());
	}
}

string AT3Tags::GetRouteCodeLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(3);
	string runway = FlightPlan.GetFlightPlanData().GetArrivalRwy();
	string fpRoute = FlightPlan.GetFlightPlanData().GetRoute();
	string lineStr;
 

	if (flightStrip.find("/R/") != string::npos && flightStrip.find("/R//") != string::npos) {
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) { //matches dest with json in case of dest changes after rte assignment
			size_t startContainer = flightStrip.find("/R/");
			size_t endContainer = flightStrip.find("/R//");
			lineStr = flightStrip.substr(startContainer + 3, endContainer - 3).substr(0, 3);
		}
	}
	else if (strlen(FlightPlan.GetFlightPlanData().GetArrivalRwy()) != 0) {
		for (auto& rte : rteJson[FlightPlan.GetFlightPlanData().GetDestination()][runway.substr(0, 2)]["routes"].items()) { //matches exact route only for auto assigning route code
			if (fpRoute.find(rte.value()["route"]) != string::npos) {
				lineStr = rte.key();
			}
		}
	}

	if (lineStr.length() < 1) {
		lineStr = "   ";
	}

	return lineStr;
}

void AT3Tags::GetAssignedAPP(CFlightPlan& FlightPlan) {
	string spadCurrent = FlightPlan.GetControllerAssignedData().GetScratchPadString();

	if (spadCurrent.find("/A/") != string::npos && spadCurrent.find("/A//") != string::npos) {
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, "");
		size_t startContainer = spadCurrent.find("/A/");
		size_t endContainer = spadCurrent.find("/A//");
		string app = spadCurrent.substr(startContainer, endContainer + 4);
		spadCurrent.erase(startContainer, (endContainer + 4) - startContainer);
		FlightPlan.GetControllerAssignedData().SetScratchPadString(spadCurrent.c_str());
		FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, app.c_str());
	}
}

string AT3Tags::GetAPPDEPLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(2);
	string lineStr;

	if (flightStrip.find("/A/") != string::npos && flightStrip.find("/A//") != string::npos && flightStrip.find("_") != string::npos) {
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) { //matches dest with json in case of dest changes after app assignment
			size_t startContainer = flightStrip.find("/A/");
			size_t endContainer = flightStrip.find("/A//");
			lineStr = flightStrip.substr(startContainer + 3, endContainer - 1);
			lineStr = lineStr.substr(0, lineStr.find("_"));
		}
	}
	else if (strlen(FlightPlan.GetFlightPlanData().GetArrivalRwy()) != 0) {
		string app = GetAvailableApps(FlightPlan.GetFlightPlanData().GetDestination(), FlightPlan.GetFlightPlanData().GetArrivalRwy())[0]; //selects default app if no assignment, which is [0]
		if (app.find("_") != string::npos) {
			lineStr = app.substr(0, app.find("_"));
			string spadItem = "/A/" + app + "/A//";
			FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, spadItem.c_str());
		}
	}

	if (lineStr.length() < 1) {
		if (strlen(FlightPlan.GetFlightPlanData().GetSidName()) != 0) {
			lineStr = FlightPlan.GetFlightPlanData().GetSidName();
		}
		else {
			lineStr = "APP RTE >";
		}
	}

	return lineStr;
}

string AT3Tags::GetAMCLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(2);
	string lineStr;

	if (flightStrip.find("/A/") != string::npos && flightStrip.find("/A//") != string::npos && flightStrip.find("_") != string::npos) {
		if (arptSet.find(FlightPlan.GetFlightPlanData().GetDestination()) != arptSet.end()) { //matches dest with json in case of dest changes after app assignment
			size_t startContainer = flightStrip.find("/A/");
			size_t endContainer = flightStrip.find("/A//");
			lineStr = flightStrip.substr(startContainer + 3, endContainer - 1);
			lineStr = lineStr.substr(0, lineStr.find("_"));
		}
	}
	else if (strlen(FlightPlan.GetFlightPlanData().GetArrivalRwy()) != 0) {
		string app = GetAvailableApps(FlightPlan.GetFlightPlanData().GetDestination(), FlightPlan.GetFlightPlanData().GetArrivalRwy())[0]; //selects default app if no assignment, which is [0]
		if (app.find("_") != string::npos) {
			lineStr = app.substr(0, app.find("_"));
			string spadItem = "/A/" + app + "/A//";
			FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(2, spadItem.c_str());
		}
	}

	if (lineStr.length() < 1) {
		lineStr = "APP RTE >";
	}

	return lineStr;
}

string AT3Tags::GetFormattedETA(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget, int minutes)
{
	try {
		string runway = FlightPlan.GetFlightPlanData().GetArrivalRwy();

		string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(3);
		size_t startContainer = flightStrip.find("/R/");
		size_t endContainer = flightStrip.find("/R//");
		string rteCode = flightStrip.substr(startContainer + 3, endContainer - 3).substr(0, 3);

		int timeToGate = 0;
		string gate = rteJson[FlightPlan.GetFlightPlanData().GetDestination()][runway.substr(0, 2)]["routes"][rteCode]["gate"];
		string prefix = gate.substr(0, 1);

		for (int i = FlightPlan.GetExtractedRoute().GetPointsNumber() - 1; i >= 0; i--) {
			if (FlightPlan.GetExtractedRoute().GetPointName(i -  1) == gate) {
				timeToGate = FlightPlan.GetExtractedRoute().GetPointDistanceInMinutes(i -  1); 
				break;
			}
		}

		if (timeToGate <= 0) {
			timeToGate = FlightPlan.GetExtractedRoute().GetPointDistanceInMinutes(FlightPlan.GetExtractedRoute().GetPointsNumber() - 1); // gate is now destination airport
			prefix = "R";
		}
		timeToGate = minutes + timeToGate;

		if (timeToGate > 59) {
			timeToGate = timeToGate - 60;
		}

		string timeStr = to_string(timeToGate);

		if (timeStr.length() < 2) {
			timeStr.insert(0, 2 - timeStr.length(), '0');
			timeStr.insert(0, "S");
		}

		timeStr.insert(0, prefix);

		return timeStr;
	}
	catch (...) {
		return "   ";
	}
}

string AT3Tags::GetAMANDelay(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget) 
{
	int delay = trunc(GetCurrentDelay(FlightPlan.GetCallsign()));
	if (delay != 0) {
		if (delay > 0) {
			return "+" + to_string(delay);
		} else {
			return "-" + to_string(delay);
		}
	} else {
		return "";
	}
}

string AT3Tags::GetCallsign(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string flightStrip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(5); //find TopSky "/cpdlc/" annotations

	if (flightStrip.find("/cpdlc/") != string::npos || flightStrip.find("/cpdlc-/") != string::npos) {
		string callsignStr = FlightPlan.GetCallsign();
		return "[" + callsignStr + "]";
	} else {
		return FlightPlan.GetCallsign();
	}
}

string AT3Tags::GetATYPWTC(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	string ATYPWTC = "";
	ATYPWTC += FlightPlan.GetFlightPlanData().GetAircraftFPType();
	ATYPWTC += FlightPlan.GetFlightPlanData().GetAircraftWtc();
	return ATYPWTC;
}

string AT3Tags::GetVSIndicator(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
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

string AT3Tags::GetArrivalRwy(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return FlightPlan.GetFlightPlanData().GetArrivalRwy();
}
