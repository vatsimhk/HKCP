#include "HKCPDisplay.hpp"


HKCPDisplay::HKCPDisplay()
{
	AtisDisp = new AtisDisplay();
	MissAlarm = new MissedApproachAlarm();
}

HKCPDisplay::~HKCPDisplay()
{
}

void HKCPDisplay::OnAsrContentLoaded(bool Loaded)
{
	MissAlarm->OnAsrContentLoaded(Loaded);
	AtisDisp->OnAsrContentLoaded(Loaded);
}

void HKCPDisplay::OnAsrContentToBeSaved()
{
	MissAlarm->OnAsrContentToBeSaved();
	AtisDisp->OnAsrContentToBeSaved();
}

void HKCPDisplay::OnRefresh(HDC hDC, int Phase)
{
	MissAlarm->OnRefresh(hDC, Phase, this);
	AtisDisp->OnRefresh(hDC, Phase, this);
}

void HKCPDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	MissAlarm->OnClickScreenObject(ObjectType, sObjectId, Pt, Area, Button);
	AtisDisp->OnClickScreenObject(ObjectType, sObjectId, Pt, Area, Button);
}

void HKCPDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	MissAlarm->OnMoveScreenObject(ObjectType, sObjectId, Pt, Area, Released);
	AtisDisp->OnMoveScreenObject(ObjectType, sObjectId, Pt, Area, Released);
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
	AtisDisp->OnOverScreenObject(ObjectType, sObjectId, Pt, Area);
	RequestRefresh();
}

void HKCPDisplay::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
	MissAlarm->OnFlightPlanControllerAssignedDataUpdate(FlightPlan, DataType);
}

bool HKCPDisplay::OnCompileCommand(const char* sCommandLine)
{
	bool a = MissAlarm->OnCompileCommand(sCommandLine);
	bool b = AtisDisp->OnCompileCommand(sCommandLine);
	return a || b;
}

void HKCPDisplay::OnAsrContentToBeClosed(void)
{
	MissAlarm->OnAsrContentToBeClosed();
	AtisDisp->OnAsrContentToBeClosed();
	delete this;
};
