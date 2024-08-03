#include "stdafx.h"
#include "HKCPDisplay.hpp"


HKCPDisplay::HKCPDisplay(int CJSLabelSize, 
						 int CJSLabelOffset, 
						 bool CJSLabelShowWhenTracked, 
						 double PlaneIconScale, 
						 string RadarDisplayType, 
						 COLORREF colorA, 
						 COLORREF colorNA, 
						 COLORREF colorR,
						 CSTCA* _STCA)
{
	MissAlarm = new MissedApproachAlarm();
	RadarTargets = new AT3RadarTargetDisplay(CJSLabelSize, CJSLabelOffset, CJSLabelShowWhenTracked, PlaneIconScale, colorA, colorNA, colorR);

	if (RadarDisplayType == "Standard ES radar screen") {
		isESRadarDisplay = true;
	}
	else {
		isESRadarDisplay = false;
	}

	STCA = _STCA;
}

HKCPDisplay::~HKCPDisplay()
{
}

void HKCPDisplay::OnAsrContentLoaded(bool Loaded)
{
	MissAlarm->OnAsrContentLoaded(Loaded);
}

void HKCPDisplay::OnAsrContentToBeSaved()
{
	MissAlarm->OnAsrContentToBeSaved();
}

void HKCPDisplay::OnRefresh(HDC hDC, int Phase)
{
	MissAlarm->OnRefresh(hDC, Phase, this);

	if (isESRadarDisplay) {
		RadarTargets->OnRefresh(hDC, Phase, this);
	}
}

void HKCPDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	MissAlarm->OnClickScreenObject(ObjectType, sObjectId, Pt, Area, Button);

	if (isESRadarDisplay) {
		RadarTargets->OnClickScreenObject(ObjectType, sObjectId, Pt, Area, Button, this);
	}
}

void HKCPDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	MissAlarm->OnMoveScreenObject(ObjectType, sObjectId, Pt, Area, Released);
}

void HKCPDisplay::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	MissAlarm->OnButtonDownScreenObject(ObjectType, sObjectId, Pt, Area, Button);
	RequestRefresh();
}

void HKCPDisplay::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	MissAlarm->OnButtonUpScreenObject(ObjectType, sObjectId, Pt, Area, Button);
}

void HKCPDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	MissAlarm->OnDoubleClickScreenObject(ObjectType, sObjectId, Pt, Area, Button);
}

void HKCPDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) {

}

void HKCPDisplay::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
	MissAlarm->OnFlightPlanControllerAssignedDataUpdate(FlightPlan, DataType);
}

bool HKCPDisplay::OnCompileCommand(const char* sCommandLine)
{
	bool a = MissAlarm->OnCompileCommand(sCommandLine);
	return a;
}

void HKCPDisplay::OnAsrContentToBeClosed(void)
{
	MissAlarm->OnAsrContentToBeClosed();
	RadarTargets->OnAsrContentToBeClosed();
	delete this;
};
