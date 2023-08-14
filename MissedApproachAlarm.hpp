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

class MissedApproachAlarmLogic :
	public EuroScopePlugIn::CPlugIn
{
public:
	MissedApproachAlarmLogic();

	vector<string>  getMissedApproaches(const char * dest);

	void ackMissedApproach(void);

};

class MissedApproachAlarm :
    public EuroScopePlugIn::CRadarScreen
{
protected:

	RECT m_Area = { 190, 523, 590, 863 };
	RECT b_Area = { 9, 7, 30, 30 };
	POINT m_Offset = { 857, 310 };
private:
	int ackButtonState = 0;

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

	void flashButton(HDC hDC, CRect button);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
};