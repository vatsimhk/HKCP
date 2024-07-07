#pragma once
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "HKCPDisplay.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>


using namespace std;
using namespace EuroScopePlugIn;

class HKCPDisplay;

class AT3RadarTargetDisplay :
    public EuroScopePlugIn::CRadarScreen
{
public:
    AT3RadarTargetDisplay(int _CJSLabelSize, int _CJSLabelOffset, bool _CJSLabelShowWhenTracked, double _PlaneIconScale);
    
    void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

	void OnClickScreenObject(int ObjectType,
		const char* sObjectId,
		POINT Pt,
		RECT Area,
		int Button,
		HKCPDisplay* Display);

	string GetControllerFreqFromId(string ID);

	string GetControllerIdFromCallsign(string callsign);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
private:
	int CJSLabelSize;
	int CJSLabelOffset;
	bool CJSLabelShowWhenTracked;
	double PlaneIconScale;
	unordered_map<string, bool> CJSLabelShowFreq;
};

