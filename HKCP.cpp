// rStatus.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "analyzeFP.hpp"
#include "MissedApproachAlarm.hpp"

CVFPCPlugin* gpMyPlugin = NULL;
vector<MissedApproachAlarm*> ScreensOpened;

void    __declspec (dllexport)    EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	// create the instance
	*ppPlugInInstance = gpMyPlugin = new CVFPCPlugin();
}

CRadarScreen* CVFPCPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	MissedApproachAlarm* rd = new MissedApproachAlarm();
	ScreensOpened.push_back(rd);
	return rd;
}

//---EuroScopePlugInExit-----------------------------------------------

void    __declspec (dllexport)    EuroScopePlugInExit(void)
{
	// delete the instance
	delete gpMyPlugin;
}