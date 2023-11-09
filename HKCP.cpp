// rStatus.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "HKCP.hpp"
#include "HKCPDisplay.hpp"
#include "VFPC/analyzeFP.hpp"
#include "MissedApproach/MissedApproachAlarm.hpp"
#include "MissedApproach/MissedApproachPlugin.hpp"
#include "Atis/AtisPlugin.hpp"
#include "Atis/AtisDisplay.hpp"

HKCPPlugin* gpMyPlugin = NULL;
CVFPCPlugin* VFPC = NULL;
AtisPlugin* Atis = NULL;
MissedApproachPlugin* Mapp = NULL;

void    __declspec (dllexport)    EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	// create the instance
	*ppPlugInInstance = gpMyPlugin = new HKCPPlugin();
}

HKCPPlugin::HKCPPlugin() : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT) {
	VFPC = new CVFPCPlugin();
	Atis = new AtisPlugin();
	Mapp = new MissedApproachPlugin();
}

HKCPPlugin::~HKCPPlugin() {
	delete VFPC;
	delete Atis;
	delete Mapp;
}

CRadarScreen* HKCPPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	return new HKCPDisplay();
}

void HKCPPlugin::OnFunctionCall(int FunctionId, const char* ItemString, POINT Pt, RECT Area) {
	VFPC->OnFunctionCall(FunctionId, ItemString, Pt, Area);
}

void HKCPPlugin::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize) {
	VFPC->OnGetTagItem(FlightPlan, RadarTarget, ItemCode, TagData, sItemString, pColorCode, pRGB, pFontSize);
}

void HKCPPlugin::OnTimer(int Count) {
	VFPC->OnTimer(Count);
	Atis->OnTimer(Count);
}

void HKCPPlugin::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	VFPC->OnFlightPlanDisconnect(FlightPlan);
}

bool HKCPPlugin::OnCompileCommand(const char* sCommandLine) {
	return VFPC->OnCompileCommand(sCommandLine);
}

void HKCPPlugin::OnCompilePrivateChat(const char* sSenderCallsign, const char* sReceiverCallsign, const char* sChatMessage) {
	Atis->OnCompilePrivateChat(sSenderCallsign, sReceiverCallsign, sChatMessage);
}

//---EuroScopePlugInExit-----------------------------------------------

void    __declspec (dllexport)    EuroScopePlugInExit(void)
{
	// delete the instance
	delete gpMyPlugin;
}
