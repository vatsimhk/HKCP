#pragma once
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "Atis/AtisDisplay.hpp"
#include "MissedApproach/MissedApproachAlarm.hpp"
#include "AT3/AT3RadarTargetDisplay.hpp"
#include "Constant.hpp"
#include <sstream>
#include <vector>
#include <thread>
#include <mmsystem.h>
#include <string>
#include <iostream>
#include <ctime>


using namespace std;
using namespace EuroScopePlugIn;

class AtisDisplay;
class MissedApproachAlarm;

class HKCPDisplay :
	public EuroScopePlugIn::CRadarScreen
{
private:
	AtisDisplay* AtisDisp;
	MissedApproachAlarm* MissAlarm;
	AT3RadarTargetDisplay* RadarTargets;
public:

	HKCPDisplay();
	virtual ~HKCPDisplay();

	//---OnAsrContentLoaded--------------------------------------------

	virtual void OnAsrContentLoaded(bool Loaded);

	//---OnAsrContentToBeSaved------------------------------------------

	virtual void OnAsrContentToBeSaved();

	//---OnRefresh------------------------------------------------------

	virtual void OnRefresh(HDC hDC, int Phase);

	//---OnClickScreenObject-----------------------------------------

	virtual void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);


	//---OnMoveScreenObject---------------------------------------------

	virtual void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);

	//---OnButtonDownScreenObject---------------------------------------------

	virtual void OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	//---OnButtonDownScreenObject---------------------------------------------

	virtual void OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	virtual void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	virtual void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);

	virtual void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);

	virtual bool OnCompileCommand(const char* sCommandLine);

	virtual void OnAsrContentToBeClosed(void);
};
