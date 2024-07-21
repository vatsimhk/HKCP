#pragma once
#include "EuroScopePlugIn.h"
#include <sstream>
#include <vector>
#include <thread>
#include <mmsystem.h>
#include <string>
#include <iostream>


using namespace std;
using namespace EuroScopePlugIn;

class MissedApproachPlugin :
	public EuroScopePlugIn::CPlugIn

{
public:
	static vector<string> activeArrRunways;

	MissedApproachPlugin();

	void ackMissedApproach(const char* callsign);

	void initMissedApproach(const char* callsign);

	void resetMissedApproach(const char* callsign);

	vector<string> getArrivalRunways();

	virtual void OnAirportRunwayActivityChanged(void);

	int getPositionType();

	vector<string> getASELAircraftData(void);

	bool matchArrivalAirport(const char* arrivalArpt);

	string checkForAck(string scratchPadString);

};
