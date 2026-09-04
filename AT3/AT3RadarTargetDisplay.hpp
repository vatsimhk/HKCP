#pragma once
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "HKCPDisplay.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <gdiplus.h>
#include "AT3Tags.hpp"

namespace DrawRouteUI {
	enum DrawType {
		DRAW_ROUTE_TYPE_REGULAR,
		DRAW_ROUTE_TYPE_OFF_ROUTE,
		DRAW_ROUTE_TYPE_PROBE_DCT,
		DRAW_ROUTE_TYPE_PROBE_DCT_NORMAL
	};

	// Leader line offsets
	const int LeaderStartX = 3;
	const int LeaderStartY = -9;
	const int LeaderElbowX = 22;
	const int LeaderElbowY = -66;
	const int LeaderEndX = 32;
	const int LeaderEndY = -66;
	const int TextOffsetX = 42;

	// Symbol layout (BMW Logo)
	const int SymbolSize = 11;
	const int SymbolRadius = SymbolSize / 2; // 5

	const int TopSky_ToolbarHeight = 21;
}

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;
using namespace DrawRouteUI;

class HKCPDisplay;
class HKCPPlugin;

class AT3RadarTargetDisplay :
    public EuroScopePlugIn::CRadarScreen
{
public:
    AT3RadarTargetDisplay(int _CJSLabelSize, int _CJSLabelOffset, bool _CJSLabelShowWhenTracked, double _PlaneIconScale, COLORREF colorA, COLORREF colorNA, COLORREF colorR);
    
    void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

	void OnClickScreenObject(int ObjectType,
		const char* sObjectId,
		POINT Pt,
		RECT Area,
		int Button,
		HKCPDisplay* Display);

	string GetControllerFreqFromId(string ID);

	string GetControllerIdFromCallsign(string callsign);

	//  This gets called before OnAsrContentToBeSaved()
	inline virtual void OnAsrContentToBeClosed(void)
	{
		delete this;
	};
private:
	int CJSLabelSize;
	int CJSLabelOffset;
	bool CJSLabelShowWhenTracked;
	double PlaneIconScale;
	unordered_map<string, bool> CJSLabelShowFreq;

	Color colorAssumed;
	Color colorNotAssumed;
	Color colorRedundant;
	Color colorRouteDraw;
	Color colorRouteDrawDCT;

	string formatRouteTag(CFlightPlanExtractedRoute& extractedRoute, int nextPointID, tm* tm_gmt) const;

	void drawRouteLine(Graphics* g, CDC* dc, HKCPDisplay* Display, CRect allowedArea, Pen& pen, POINT point1, POINT point2, const char* airway);

	void drawRouteTag(Graphics* g, CDC* dc, HKCPDisplay* Display, CRect allowedArea, Pen& pen, POINT point, const char* routeTag);

	void drawWPTSelectSymbol(Graphics* g, CDC* dc, Pen& pen, SolidBrush& Brush, POINT point);

	void createRouteDraw(CFlightPlan* fp, POINT acftLocation, enum DrawType DrawType, int nextPointID, int probeNextID, Graphics* g, CDC* dc, HKCPDisplay* Display);
};

