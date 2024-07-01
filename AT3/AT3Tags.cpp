#include "stdafx.h"
#include "AT3Tags.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include <string>

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
	RegisterTagItemType("AT3 Slot", TAG_ITEM_AT3_SLOT);
	RegisterTagItemType("AT3 Callsign", TAG_ITEM_AT3_CALLSIGN);
	RegisterTagItemType("AT3 ATYP + WTC", TAG_ITEM_AT3_ATYPWTC);
	RegisterTagItemType("AT3 VS Indicator", TAG_ITEM_AT3_VS_INDICATOR);
	RegisterTagItemType("AT3 Arrival Runway", TAG_ITEM_AT3_ARRIVAL_RWY);
}


void AT3Tags::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
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
			tagOutput = GetRouteCode(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_APPDEP_LINE4:
			tagOutput = GetAPPDEPLine4(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_AMC_LINE4:
			tagOutput = GetAMCLine4(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_SLOT:
			tagOutput = GetFormattedSlot(FlightPlan, RadarTarget);
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

void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	CFlightPlan FlightPlan = AT3Tags().FlightPlanSelectASEL();
	if (!FlightPlan.IsValid()) {
		return;
	}

	string origin = FlightPlan.GetFlightPlanData().GetOrigin();
	string SID = FlightPlan.GetFlightPlanData().GetSidName();
	string dest = FlightPlan.GetFlightPlanData().GetDestination();
	string destRunway = FlightPlan.GetFlightPlanData().GetArrivalRwy();
	string STAR = FlightPlan.GetFlightPlanData().GetStarName();

	string APPMenuName = dest + destRunway + "APPs";

	switch (FunctionId) {
		case TAG_FUNC_APP_SEL_MENU:
			AT3Tags().OpenPopupList(Area, APPMenuName.c_str(), 1);
		}
}

string AT3Tags::GetFormattedAltitude(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	int altitude = RadarTarget.GetPosition().GetPressureAltitude();
	int transAlt = AT3Tags().GetTransitionAltitude();
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
	int transAlt = AT3Tags().GetTransitionAltitude();
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
			case 0:
				if (finalAltAssigned != 0) {
					formattedAltAssigned = to_string((finalAltAssigned + 50) / 100);
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
				formattedAltAssigned = "A";
				break;
			case 2:
				formattedAltAssigned = "V";
				break;

		}
	}
	else {
		formattedAltAssigned = "999";
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

	if (FlightPlan.GetControllerAssignedData().GetAssignedMach() != 0) {
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
	}
	else if (FlightPlan.GetControllerAssignedData().GetAssignedSpeed() == 0) {
		speedAssigned = "   ";
	}
	else {
		speedAssigned = "999";
	}
	return speedAssigned;
}

string AT3Tags::GetRouteCode(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return "ABC";
}

string AT3Tags::GetAPPDEPLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return "APP RTE >";
}

string AT3Tags::GetAMCLine4(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return "APP RTE >";
}

string AT3Tags::GetFormattedSlot(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return "D00";
}

string AT3Tags::GetCallsign(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return FlightPlan.GetCallsign();
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
	} else if (RadarTarget.GetVerticalSpeed() > -200) {
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