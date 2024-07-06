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
		if (strcmp(runway.GetRunwayName(0), "NAP") == 0) {
			continue;
		}
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
	string buf = controllerData.GetScratchPadString();
	buf.append("\\MISS");
	controllerData.SetScratchPadString(buf.c_str());
}

void MissedApproachPlugin::ackMissedApproach(const char* callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CController myself = ControllerMyself();
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();

	string buf = controllerData.GetScratchPadString();
	if (buf.find("\\MISS") != string::npos) {
		buf.append("\\ACK\\");
		buf.append(myself.GetPositionId());
		controllerData.SetScratchPadString(buf.c_str());
	}
	//couldn't find it, handle error
}

void MissedApproachPlugin::resetMissedApproach(const char* callsign) {
	CFlightPlan fpl = FlightPlanSelect(callsign);
	CFlightPlanData data;
	CFlightPlanControllerAssignedData controllerData;
	data = fpl.GetFlightPlanData();
	controllerData = fpl.GetControllerAssignedData();
	string buf = controllerData.GetScratchPadString();
	size_t index;
	string miss = "\\MISS";
	string ack = "\\ACK";
	index = buf.find(miss);
	if (index != string::npos) {
		buf.erase(index, miss.length());
	}
	index = buf.find(ack);
	if (index != string::npos) {
		buf.erase(index, ack.length()+4);
	}
	controllerData.SetScratchPadString(buf.c_str());
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
	if (!fpl.IsValid() || rdr.GetPressureAltitude() < 50 || fpl.GetDistanceToDestination() > 20.0 || *fpl.GetTrackingControllerId() != '\0') {
		//return empty if FPL is invalid, on the ground, too far from destination, or tracked
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
	const char* ptr = strstr(controllerData.GetScratchPadString(), "\\MISS\\ACK\\");
	if (ptr != NULL) {
		ptr = ptr + strlen("\\MISS\\ACK\\");
		return (ptr != NULL && strlen(ptr) == 3) ? ptr : "???";
	}
	return NULL;
}
