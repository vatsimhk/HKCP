#include "stdafx.h"
#include "MissedApproachPlugin.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include <time.h>

using namespace EuroScopePlugIn;

vector<string> MissedApproachPlugin::activeArrRunways = {};

//PLUGIN Helper Functions

MissedApproachPlugin::MissedApproachPlugin() : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT) {
	activeArrRunways = getArrivalRunways();
}

vector<string> MissedApproachPlugin::getArrivalRunways() {
	CSectorElement runway;
	vector<string> activeRunways;
	for (runway = SectorFileElementSelectFirst(SECTOR_ELEMENT_RUNWAY); runway.IsValid(); runway = SectorFileElementSelectNext(runway, SECTOR_ELEMENT_RUNWAY)) {
		if (runway.IsElementActive(false, 0)) {
			activeRunways.push_back(runway.GetRunwayName(0));
		}
		if (runway.IsElementActive(false, 1)) {
			activeRunways.push_back(runway.GetRunwayName(1));
		}
	}
	return activeRunways;
}

void MissedApproachPlugin::initMissedApproach(const char* callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	if (!fpl.IsValid()) return;

	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();
	controllerData.SetScratchPadString("MISAP_");
}

void MissedApproachPlugin::ackMissedApproach(const char* callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CController myself = ControllerMyself();
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();

	string ackMessage = "MISAP_ACK_";
	ackMessage.append(myself.GetPositionId());
	if (strcmp(controllerData.GetScratchPadString(), "MISAP_") == 0) {
		controllerData.SetScratchPadString(ackMessage.c_str());
	}
	//couldn't find it, handle error
}

void MissedApproachPlugin::resetMissedApproach(const char* callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();
	if (strstr(controllerData.GetScratchPadString(), "MISAP_") != NULL) {
		controllerData.SetScratchPadString("");
	}
	//couldn't find it, handle error
}

void MissedApproachPlugin::OnAirportRunwayActivityChanged() {
	activeArrRunways = getArrivalRunways();
}

int MissedApproachPlugin::getPositionType() {
	CController myself = ControllerMyself();
	return myself.GetFacility();
}

vector<string> MissedApproachPlugin::getASELAircraftData() {
	vector<string> acftData = {};
	CFlightPlan fpl = FlightPlanSelectASEL();
	activeArrRunways = getArrivalRunways();
	CRadarTargetPositionData rdr = fpl.GetCorrelatedRadarTarget().GetPosition();
	if (!fpl.IsValid() || rdr.GetPressureAltitude() < 50 || fpl.GetDistanceToDestination() > 20.0) {
		//return empty if FPL is invalid, on the ground, or too far from destination
		return acftData;
	}
	else {
		CFlightPlanData data = fpl.GetFlightPlanData();
		acftData.push_back(fpl.GetCallsign());
		acftData.push_back(data.GetDestination());
		acftData.push_back(data.GetArrivalRwy());
	}

	return acftData;
}

bool MissedApproachPlugin::matchArrivalAirport(const char* arrivalArpt) {
	const char* myself = ControllerMyself().GetCallsign();
	if (strstr(myself, arrivalArpt) == NULL) {
		return false;
	}
	return true;
}

const char* MissedApproachPlugin::checkForAck(const char* callsign) {
	CFlightPlanControllerAssignedData controllerData = FlightPlanSelect(callsign).GetControllerAssignedData();
	const char* ptr = strstr(controllerData.GetScratchPadString(), "MISAP_ACK_");
	if (ptr != NULL) {
		ptr = ptr + strlen("MISAP_ACK_");
		return (ptr != NULL && strlen(ptr) == 3) ? ptr : "???";
	}
	return NULL;
}
