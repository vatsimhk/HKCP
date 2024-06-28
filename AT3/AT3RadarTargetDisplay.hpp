#pragma once
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "HKCPDisplay.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>


using namespace std;
using namespace EuroScopePlugIn;

class HKCPDisplay;

class AT3RadarTargetDisplay :
    public EuroScopePlugIn::CRadarScreen
{
public:
    AT3RadarTargetDisplay();
    
    void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
};

