#pragma once
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "HKCPDisplay.hpp"
#include <sstream>
#include <vector>
#include <thread>
#include <mmsystem.h>
#include <string>
#include <iostream>
#include <ctime>


using namespace std;
using namespace EuroScopePlugIn;

class HKCPDisplay;

class MissedApproachAlarm :
    public EuroScopePlugIn::CRadarScreen
{
protected:

	static RECT m_Area;
	static RECT c_Area;
	static RECT c_Area_Min;
	static RECT i_Area;
	static RECT i_Area_Min;
	static POINT m_Offset;
	static POINT c_Offset;
	static POINT i_Offset;
private:
	static int ackButtonState;
	static int actButtonState;
	static int actButtonHold;
	static clock_t now;	
	static int resetButtonState;
	static int windowVisibility; // 0 = hidden, 1 = minimised, 2 = full
	static vector<string> missedAcftData;
	static vector<string> activeMAPPRunways;
	static vector<string> selectedAcftData;
	static string ackStation;

public:

    MissedApproachAlarm();
    virtual ~MissedApproachAlarm();

	//---OnAsrContentLoaded--------------------------------------------

	virtual void OnAsrContentLoaded(bool Loaded);

	//---OnAsrContentToBeSaved------------------------------------------

	virtual void OnAsrContentToBeSaved();

	//---OnRefresh------------------------------------------------------

	void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

	//---OnClickScreenObject-----------------------------------------

	virtual void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);


	//---OnMoveScreenObject---------------------------------------------

	virtual void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);

	//---OnButtonDownScreenObject---------------------------------------------

	virtual void OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	//---OnButtonDownScreenObject---------------------------------------------

	virtual void OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	virtual void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	virtual void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);

	virtual bool OnCompileCommand(const char* sCommandLine);

	void flashButton(HDC hDC, CRect button);

	void drawAlarmWindow(HDC hDC, HKCPDisplay* Display);

	void drawConfigWindow(HDC hDC, HKCPDisplay* Display);

	void drawIndicatorUnit(HDC hDC, HKCPDisplay* Display);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
};