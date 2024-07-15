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
#include "AT3/AT3Tags.hpp"

HKCPPlugin* gpMyPlugin = NULL;
CVFPCPlugin* VFPC = NULL;
AtisPlugin* Atis = NULL;
MissedApproachPlugin* Mapp = NULL;
AT3Tags* tags = NULL;

void    __declspec (dllexport)    EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	// create the instance
	*ppPlugInInstance = gpMyPlugin = new HKCPPlugin();
}

HKCPPlugin::HKCPPlugin() : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT) {
	colorAssumed = GetTopSkyColorSettings("Color_Assumed", RGB(241, 246, 255));
	colorNotAssumed = GetTopSkyColorSettings("Color_Unconcerned", RGB(117, 132, 142));
	colorRedundant = GetTopSkyColorSettings("Color_Redundant", RGB(229, 214, 130));
	
	VFPC = new CVFPCPlugin();
	Atis = new AtisPlugin();
	Mapp = new MissedApproachPlugin();
	tags = new AT3Tags(colorAssumed, colorNotAssumed, colorRedundant);
}

HKCPPlugin::~HKCPPlugin() {
	delete VFPC;
	delete Atis;
	delete Mapp;
	delete tags;
}

COLORREF HKCPPlugin::GetTopSkyColorSettings(string settingName, COLORREF defaultColor)
{
	char DllPathFile[_MAX_PATH];

	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	string path = DllPathFile;
	path.resize(path.size() - strlen("HKCP/HKCP.dll"));
	string settingsPath = path + "TopSky/TopSkySettings.txt";

	ifstream TopSkySettingsFile(settingsPath);
	if (!TopSkySettingsFile) {
		DisplayUserMessage("HKCP", "HKCP", "Unable to find TopSkySettings.txt for colors", true, true, false, false, false);
		return defaultColor;
	}

	string line;
	while (getline(TopSkySettingsFile, line)) {
		if (line.find(settingName) != string::npos) {
			break;
		}
	}

	line.erase(0, settingName.length() + 1);
	int r, g, b;
	istringstream(line.substr(0, 3)) >> r;
	istringstream(line.substr(4, 7)) >> g;
	istringstream(line.substr(8, 11)) >> b;

	return RGB(r, g, b);
}

CRadarScreen* HKCPPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	const char* buffer;
	int CJSLabelSize = 12, CJSLabelOffset = 25;
	bool CJSLabelShowWhenTracked = 0;
	double PlaneIconScale = 1.0;

	buffer = GetDataFromSettings("CJSLabelSize");
	if (buffer != NULL) {
		CJSLabelSize = atoi(buffer);
	}
	else {
		SaveDataToSettings("CJSLabelSize", "CJSLabelSize", to_string(CJSLabelSize).c_str());
	}

	buffer = GetDataFromSettings("CJSLabelOffset");
	if (buffer != NULL) {
		CJSLabelOffset = atoi(buffer);
	}
	else {
		SaveDataToSettings("CJSLabelOffset", "CJSLabelOffset", to_string(CJSLabelOffset).c_str());
	}

	buffer = GetDataFromSettings("CJSLabelShowWhenTracked");
	if (buffer != NULL) {
		CJSLabelShowWhenTracked = atoi(buffer);
	}
	else {
		SaveDataToSettings("CJSLabelShowWhenTracked", "CJSLabelShowWhenTracked", to_string(CJSLabelShowWhenTracked).c_str());
	}

	buffer = GetDataFromSettings("PlaneIconScale");
	if (buffer != NULL) {
		PlaneIconScale = atof(buffer);
	}
	else {
		SaveDataToSettings("PlaneIconScale", "PlaneIconScale", to_string(PlaneIconScale).c_str());
	}

	return new HKCPDisplay(CJSLabelSize, 
						   CJSLabelOffset, 
						   CJSLabelShowWhenTracked, 
						   PlaneIconScale, 
						   sDisplayName, 
						   colorAssumed, 
						   colorNotAssumed, 
						   colorRedundant);
}

void HKCPPlugin::OnFunctionCall(int FunctionId, const char* ItemString, POINT Pt, RECT Area) {
	VFPC->OnFunctionCall(FunctionId, ItemString, Pt, Area);
	tags->OnFunctionCall(FunctionId, ItemString, Pt, Area);
}

void HKCPPlugin::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize) {
	tags->OnGetTagItem(FlightPlan, RadarTarget, ItemCode, TagData, sItemString, pColorCode, pRGB, pFontSize);
	VFPC->OnGetTagItem(FlightPlan, RadarTarget, ItemCode, TagData, sItemString, pColorCode, pRGB, pFontSize);
}

void HKCPPlugin::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType) {
	tags->OnFlightPlanControllerAssignedDataUpdate(FlightPlan, DataType);
}

void HKCPPlugin::OnTimer(int Count) {
	VFPC->OnTimer(Count);
	Atis->OnTimer(Count);
	tags->OnTimer(Count);
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
