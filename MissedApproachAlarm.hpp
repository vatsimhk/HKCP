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

	void ackMissedApproach(const char * callsign);

	void initMissedApproach(const char * callsign);

	void resetMissedApproach(const char* callsign);

	vector<string> getArrivalRunways();

	virtual void OnAirportRunwayActivityChanged(void);

	int getPositionType();

	vector<string> getASELAircraftData(void);

	bool matchArrivalAirport(const char* arrivalArpt);

	const char * checkForAck(const char* callsign);

};

class MissedApproachAlarm :
    public EuroScopePlugIn::CRadarScreen
{
protected:

	static RECT m_Area;
	static RECT b_Area;
	static RECT c_Area;
	static RECT c_Area_Min;
	static RECT i_Area;
	static POINT m_Offset;
private:
	static int ackButtonState;
	static int actButtonState;
	static int resetButtonState;
	static int configWindowState; // 0 = hidden, 1 = minimised, 2 = full
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

	virtual void OnRefresh(HDC hDC, int Phase);

	//---OnClickScreenObject-----------------------------------------

	virtual void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);


	//---OnMoveScreenObject---------------------------------------------

	virtual void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);

	//---OnButtonDownScreenObject---------------------------------------------

	virtual void OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	//---OnButtonDownScreenObject---------------------------------------------

	virtual void OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

	virtual void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);

	virtual bool OnCompileCommand(const char* sCommandLine);

	void flashButton(HDC hDC, CRect button);

	void drawAlarmWindow(HDC hDC);

	void drawConfigWindow(HDC hDC);

	void drawIndicatorUnit(HDC hDC);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
};