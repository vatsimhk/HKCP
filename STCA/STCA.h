#pragma once
#include <EuroScopePlugIn.h>
#include <string>
#include <vector>
#include <algorithm>
#define M_PI 3.14159265358979323846

/*
STCA Algorithm Provided by Upper Area Control PlugIn 
https://github.com/pierr3/UACPlugin

Licensed under GPL v3.0
*/

using namespace std;
using namespace EuroScopePlugIn;


class CSTCA
{
public:
	CSTCA();
	virtual ~CSTCA();

	vector<string> Alerts;
	vector<string> mediumAlerts;

	int high_level_sep = 5;
	int low_level_sep = 3;
	int disable_level = 1000;
	int level_reduced_sep = 11000;
	const static int time_to_extrapolate = 120;
	const static int medium_time_to_extrapolate = 300;
	int altitude_sep = 950;

	void OnTimer(CPlugIn * pl);
	bool IsSTCA(string cs);

	CPosition Extrapolate(CPosition init, double angle, double meters);

	// Radians

	inline static double DegToRad(double x)
	{
		return x / 180.0 * M_PI;
	};

	inline static double RadToDeg(double x)
	{
		return x / M_PI * 180.0;
	};
};

