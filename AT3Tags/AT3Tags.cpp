#include "stdafx.h"
#include "AT3Tags.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"

using namespace EuroScopePlugIn;

AT3Tags::AT3Tags() : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT)
{
	RegisterTagItemType("AT3 Altitude", TAG_ITEM_AT3_ALTITUDE);
	RegisterTagItemType("AT3 Assigned Altitude", TAG_ITEM_AT3_ALTITUDE_ASSIGNED);
	RegisterTagItemType("AT3 Track", TAG_ITEM_AT3_TRACK);
	RegisterTagItemType("AT3 Assigned Heading", TAG_ITEM_AT3_HEADING_ASSIGNED);
	RegisterTagItemType("AT3 Groundspeed", TAG_ITEM_AT3_SPEED);
	RegisterTagItemType("AT3 Assigned Speed (IAS)", TAG_ITEM_AT3_SPEED_ASSIGNED);
	RegisterTagItemType("AT3 TMA Entry Point", TAG_ITEM_AT3_TMA_ENTRY);
	RegisterTagItemType("AT3 Approach Type", TAG_ITEM_AT3_APP_TYPE);
	RegisterTagItemType("AT3 Slot", TAG_ITEM_AT3_SLOT);
}

void AT3Tags::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid() || !RadarTarget.IsValid()) {
		return;
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
		case TAG_ITEM_AT3_TMA_ENTRY:
			tagOutput = GetFormattedEntryPoint(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_APP_TYPE:
			tagOutput = GetFormattedApproachType(FlightPlan, RadarTarget);
			break;
		case TAG_ITEM_AT3_SLOT:
			tagOutput = GetFormattedSlot(FlightPlan, RadarTarget);
			break;
		default:
			tagOutput = "";
	}

	// Convert string output to character array
	strcpy_s(sItemString, 16, tagOutput.substr(0, 15).c_str());
}

string AT3Tags::GetFormattedAltitude(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedAltitudedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedTrack(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedHeadingAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedGroundspeed(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedSpeedAssigned(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedEntryPoint(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedApproachType(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}

string AT3Tags::GetFormattedSlot(CFlightPlan& FlightPlan, CRadarTarget& RadarTarget)
{
	return string();
}
