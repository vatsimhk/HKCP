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
    AT3RadarTargetDisplay(int _CJSLabelSize, int _CJSLabelOffset, double _PlaneIconScale);
    
    void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

	void OnRadarTargetPositionUpdate(CRadarTarget RadarTarget);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
private:
	int CJSLabelSize;
	int CJSLabelOffset;
	double PlaneIconScale;
};

