#pragma once
#include "EuroScopePlugIn.h"
#include <sstream>
#include <iostream>
#include <string>
#include "Constant.hpp"
#include <fstream>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using namespace std;
using namespace EuroScopePlugIn;
using json = nlohmann::json;

class CVFPCPlugin :
	public EuroScopePlugIn::CPlugIn
{
public:
	CVFPCPlugin();
	virtual ~CVFPCPlugin();

	void sendMessage(string type, string message);

	void sendMessage(string message);

	bool IsStringInList(const string& target, const vector<string>& list);

	bool IsDestinationMatch(const string& destination, const json& rule);

	bool IsAirwayMatch(const string& route, const json& rule);

	string CheckAltitude(int rfl, const json& rules);

	string ValidateFlightPlan(CFlightPlan& flightPlan, const json& sidData);

protected:

};

