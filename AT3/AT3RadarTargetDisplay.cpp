#include "stdafx.h"
#include "HKCPDisplay.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include "AT3RadarTargetDisplay.hpp"
#include <time.h>

using namespace Gdiplus;
using namespace EuroScopePlugIn;
using namespace DrawRouteUI;

AT3RadarTargetDisplay::AT3RadarTargetDisplay(int _CJSLabelSize, int _CJSLabelOffset, bool _CJSLabelShowWhenTracked, double _PlaneIconScale, COLORREF colorA, COLORREF colorNA, COLORREF colorR) :
	CJSLabelSize(_CJSLabelSize), CJSLabelOffset(_CJSLabelOffset), CJSLabelShowWhenTracked(_CJSLabelShowWhenTracked), PlaneIconScale(_PlaneIconScale)
{
	colorAssumed.SetFromCOLORREF(colorA);
	colorNotAssumed.SetFromCOLORREF(colorNA);
	colorRedundant.SetFromCOLORREF(colorR);
	colorRouteDraw.SetFromCOLORREF(ROUTE_DRAW);
	colorRouteDrawDCT.SetFromCOLORREF(ROUTE_DRAW_DCT);
}

void AT3RadarTargetDisplay::OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display)
{
	if (Phase != REFRESH_PHASE_AFTER_TAGS) {
		return;
	}

	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Save context for later
	int sDC = dc.SaveDC();
	
	// Setup exclusion zone for CDC
	dc.ExcludeClipRect(0, 0, 9999, Display->GetToolbarArea().bottom + TopSky_ToolbarHeight);

	// Create graphics object
	Graphics g(hDC);

	GraphicsState gState = g.Save();

	// Create font
	CFont EuroScopeFont;
	EuroScopeFont.CreateFont(
		CJSLabelSize,              // nHeight
		0,                        // nWidth
		0,                        // nEscapement
		0,                        // nOrientation
		FW_NORMAL,                // nWeight
		FALSE,                    // bItalic
		FALSE,                    // bUnderline
		0,                        // cStrikeOut
		ANSI_CHARSET,             // nCharSet
		OUT_DEFAULT_PRECIS,       // nOutPrecision
		CLIP_DEFAULT_PRECIS,      // nClipPrecision
		DEFAULT_QUALITY,          // nQuality
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
		_T("EuroScope")  // lpszFacename
	);      
	

	// Select first aircraft
	CRadarTarget acft;
	acft = GetPlugIn()->RadarTargetSelectFirst();

	// Loop through all aircrafts
	while (acft.IsValid()) {
		// Get Flight plan and position data
		CFlightPlan fp = Display->GetPlugIn()->FlightPlanSelect(acft.GetCallsign());
		CRadarTargetPositionData pd = acft.GetPosition();

		string callsign = fp.GetCallsign();

		if (!fp.IsValid() || !pd.IsValid()) {
			acft = GetPlugIn()->RadarTargetSelectNext(acft);
			continue;
		}

		// Skip drawing if not mode C
		if (!pd.GetTransponderC()) {
			acft = GetPlugIn()->RadarTargetSelectNext(acft);
			continue;
		}

		// Setup container
		GraphicsContainer gContainer = g.BeginContainer();

		// Set brush color based on state
		SolidBrush aircraftBrush(colorNotAssumed);
		dc.SetTextColor(colorNotAssumed.ToCOLORREF());
		if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED || fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED) {
			aircraftBrush.SetColor(colorAssumed);
			dc.SetTextColor(colorAssumed.ToCOLORREF());
		}
		else if (fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED) {
			aircraftBrush.SetColor(colorAssumed);
			dc.SetTextColor(colorRedundant.ToCOLORREF());
		}
		else if (fp.GetState() == FLIGHT_PLAN_STATE_REDUNDANT || fp.GetState() == FLIGHT_PLAN_STATE_COORDINATED) {
			aircraftBrush.SetColor(colorRedundant);
			dc.SetTextColor(colorRedundant.ToCOLORREF());
		}

		// Override aircraft color conditions
		if (pd.GetPressureAltitude() > 100 && strlen(fp.GetTrackingControllerId()) == 0 &&
			fp.GetSectorEntryMinutes() <= 1 && fp.GetSectorEntryMinutes() >= 0) {
			if ((fp.GetDistanceFromOrigin() > 8 && fp.GetDistanceToDestination() > 8) || pd.GetPressureAltitude() > 3000) { //not approaching/departing
				aircraftBrush.SetColor(OVERRIDE_AIW);
			}
		}
		if (strcmp(pd.GetSquawk(), "7700") == 0) {
			aircraftBrush.SetColor(OVERRIDE_EMER);
		}

		// Get and set location
		POINT acftLocation = Display->ConvertCoordFromPositionToPixel(acft.GetPosition().GetPosition());
		g.ScaleTransform(PlaneIconScale, PlaneIconScale, MatrixOrderAppend);
		g.TranslateTransform(acftLocation.x, acftLocation.y, MatrixOrderAppend);
		g.RotateTransform(acft.GetPosition().GetReportedHeadingTrueNorth());

		// Set Anti-aliasing
		//g.SetSmoothingMode(SmoothingModeAntiAlias);

		// Define aircraft icon
		Point aircraftIcon[19] = {
			Point(0,-7),
			Point(-1,-6),
			Point(-1,-2),
			Point(-7,1),
			Point(-7,3),
			Point(-1,1),
			Point(-1,4),
			Point(-4,5),
			Point(-4,7),
			Point(0,6),
			Point(4,7),
			Point(4,5),
			Point(1,4),
			Point(1,1),
			Point(7,3),
			Point(7,1),
			Point(1,-2),
			Point(1,-6),
			Point(0,-7)
		};

		// Draw the aircraft icon
		g.FillPolygon(&aircraftBrush, aircraftIcon, 19);

		// Cleanup
		g.EndContainer(gContainer);
		DeleteObject(&aircraftIcon);

		// Draw CJS
		dc.SelectObject(EuroScopeFont);
		dc.SetTextAlign(TA_CENTER);
		CSize CJSLabelSize;

		if (fp.GetState() != FLIGHT_PLAN_STATE_ASSUMED || CJSLabelShowWhenTracked) {
			// Set CJS label text to CJS or frequency based on saved state
			string CJSLabelText;
			CJSLabelShowFreq.emplace(fp.GetCallsign(), false);
			if (fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED) {
				if (CJSLabelShowFreq[fp.GetCallsign()]) {
					CJSLabelText = GetControllerFreqFromId(fp.GetHandoffTargetControllerId());
					dc.SetTextColor(colorAssumed.ToCOLORREF());
				}
				else {
					CJSLabelText = fp.GetHandoffTargetControllerId();
				}
			}
			else if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED) {
				if (CJSLabelShowFreq[fp.GetCallsign()]) {
					CJSLabelText = GetControllerFreqFromId(GetControllerIdFromCallsign(fp.GetCoordinatedNextController()));
					dc.SetTextColor(colorAssumed.ToCOLORREF());
				}
				else {
					CJSLabelText = GetControllerIdFromCallsign(fp.GetCoordinatedNextController());
				}
			}
			else {
				if (CJSLabelShowFreq[fp.GetCallsign()]) {
					CJSLabelText = GetControllerFreqFromId(fp.GetTrackingControllerId());
					dc.SetTextColor(colorAssumed.ToCOLORREF());
				}
				else {
					CJSLabelText = fp.GetTrackingControllerId();
				}
			}

			// Remove trailing up to two trailing zeroes
			for (int i = 0; i < 2; i++) {
				if (CJSLabelText.back() == '0') {
					CJSLabelText.pop_back();
				}
			}
			dc.ExtTextOutA(acftLocation.x, acftLocation.y - CJSLabelOffset, ETO_CLIPPED, CRect(0, Display->GetToolbarArea().bottom + TopSky_ToolbarHeight, 9999, 9999), CJSLabelText.c_str(), CJSLabelText.length(), NULL);

			// Create rectangle around CJS label for click spot
			CJSLabelSize = dc.GetTextExtent(CJSLabelText.c_str());
			POINT CJSLabelPoint = { acftLocation.x - CJSLabelSize.cx / 2, acftLocation.y - CJSLabelOffset };
			CRect CJSLabelRect(CJSLabelPoint, CJSLabelSize);
			Display->AddScreenObject(CJS_INDICATOR, fp.GetCallsign(), CJSLabelRect, true, "");
		}

		// Create route draw
		if (AT3Tags::showRouteDraw[fp.GetCallsign()]) {
			CFlightPlanExtractedRoute extractedRoute = fp.GetExtractedRoute();
			int pointCount = extractedRoute.GetPointsNumber();
			bool isRAM;
			int nextPointID;
			int probeNextID = -1;

			for (int i = 0; i <= pointCount; i++) {
				if (extractedRoute.GetPointDistanceInMinutes(i) != -1) {
					probeNextID = i;
					break;
				}
			}

			if (extractedRoute.GetPointsAssignedIndex() != -1) {
				nextPointID = extractedRoute.GetPointsAssignedIndex();
			}
			else {
				nextPointID = extractedRoute.GetPointsCalculatedIndex();
			}
			CPosition acftCoor = acft.GetPosition().GetPosition();
			CPosition prevPointCoor = extractedRoute.GetPointPosition(nextPointID - 1);
			CPosition nextPointCoor = extractedRoute.GetPointPosition(nextPointID);
			
			if (extractedRoute.GetPointsAssignedIndex() == -1 && (acftCoor.DistanceTo(prevPointCoor) + acftCoor.DistanceTo(nextPointCoor) - 2) > nextPointCoor.DistanceTo(prevPointCoor)) {
				isRAM = true;
			}
			else {
				isRAM = false;
			}

			// If probing DCT, draw type 3
			if (nextPointID != probeNextID) {
				nextPointID = extractedRoute.GetPointsCalculatedIndex();
				createRouteDraw(&fp, acftLocation, DRAW_ROUTE_TYPE_PROBE_DCT, nextPointID, probeNextID, &g, &dc, Display);
			}// If off-route, draw type 2
			else if (isRAM) {
				createRouteDraw(&fp, acftLocation, DRAW_ROUTE_TYPE_OFF_ROUTE, nextPointID, probeNextID, &g, &dc, Display);
			}// Else, draw type 1
			else {
				createRouteDraw(&fp, acftLocation, DRAW_ROUTE_TYPE_REGULAR, nextPointID, probeNextID, &g, &dc, Display);
			}
		}
		// Increment to next aircraft
		acft = GetPlugIn()->RadarTargetSelectNext(acft);
	}
  
	g.Restore(gState);
  
	// Draw ASEL highlight box
	CRadarTarget aselRt = GetPlugIn()->RadarTargetSelectASEL();
	if (aselRt.IsValid()) {
		CPosition aselPos = aselRt.GetPosition().GetPosition();

		GraphicsContainer gContainer = g.BeginContainer();
		Pen aselPen(colorNotAssumed);

		CFlightPlan aselFp = aselRt.GetCorrelatedFlightPlan();
		if (aselFp.IsValid()) {
			if (aselFp.GetState() == FLIGHT_PLAN_STATE_ASSUMED || aselFp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED) {
				aselPen.SetColor(colorAssumed);
			}
		}

		POINT aselLocation = Display->ConvertCoordFromPositionToPixel(aselPos);
		g.ScaleTransform(PlaneIconScale, PlaneIconScale, MatrixOrderAppend);
		g.TranslateTransform(aselLocation.x, aselLocation.y, MatrixOrderAppend);

		Point aselIcon[4] = {
			Point(14, 14),
			Point(14, -14),
			Point(-14, -14),
			Point(-14, 14),
		};

		// Draw the aircraft icon
		g.DrawPolygon(&aselPen, aselIcon, 4);

		// Cleanup
		g.EndContainer(gContainer);
		DeleteObject(&aselIcon);
	};
  
	// Restore context
	dc.RestoreDC(sDC);

	//De-allocate graphics objects
	dc.Detach();
	g.ReleaseHDC(hDC);
	dc.DeleteDC();
}

void AT3RadarTargetDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button, HKCPDisplay* Display)
{
	if (ObjectType != CJS_INDICATOR) {
		return;
	}

	if (Button == BUTTON_LEFT) {
		// Toggle between freq and CJS label
		string callsign = sObjectId;
		CJSLabelShowFreq[callsign] = !CJSLabelShowFreq[callsign];
	} else if (Button == BUTTON_RIGHT) {
		// Open next controller menu
		Display->StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_SECTOR_INDICATOR, "", NULL, TAG_ITEM_FUNCTION_ASSIGNED_NEXT_CONTROLLER, Pt, Area);
	}
}

string AT3RadarTargetDisplay::GetControllerFreqFromId(string ID)
{
	double freq = GetPlugIn()->ControllerSelectByPositionId(ID.c_str()).GetPrimaryFrequency();
	if (freq < 100.0) {
		return "";
	}

	string freqString = to_string(freq);
	freqString.resize(7);
	return freqString;
}

string AT3RadarTargetDisplay::GetControllerIdFromCallsign(string callsign)
{
	return GetPlugIn()->ControllerSelect(callsign.c_str()).GetPositionId();
}

string AT3RadarTargetDisplay::formatRouteTag (CFlightPlanExtractedRoute& extractedRoute, int nextPointID, tm* tm_gmt) const {

	const char* nextPointName;
	int totalMinutes;

	if (extractedRoute.GetPointDistanceInMinutes(nextPointID) != -1) {
		totalMinutes = tm_gmt->tm_min + extractedRoute.GetPointDistanceInMinutes(nextPointID);
	}
	else {
		totalMinutes = tm_gmt->tm_min;
	}

	// 2. Calculate correct hour and minute with wrapping
	// Add the extra hours to current hour, then wrap at 24
	int finalHour = (tm_gmt->tm_hour + (totalMinutes / 60)) % 24;
	// Wrap minutes at 60
	int finalMin = totalMinutes % 60;

	// 3. Format with leading zeros ("%02d" means pad with zero to 2 digits)
	char buf[5];
	snprintf(buf, sizeof(buf), "%02d%02d", finalHour, finalMin);
	string nextPointETA = buf;

	nextPointName = extractedRoute.GetPointName(nextPointID);
	return string(nextPointName) + " " + nextPointETA;
}

void AT3RadarTargetDisplay::drawRouteLine(Graphics* g, CDC* dc, HKCPDisplay* Display, CRect allowedArea, Pen& pen, POINT point1, POINT point2, const char* airway) {
	g->DrawLine(&pen, (INT)point1.x, (INT)point1.y, (INT)point2.x, (INT)point2.y);
	dc->ExtTextOutA(((INT)point1.x + (INT)point2.x) / 2, ((INT)point1.y + (INT)point2.y) / 2, ETO_CLIPPED, &allowedArea, airway, strlen(airway), NULL);
}

void AT3RadarTargetDisplay::drawRouteTag(Graphics* g, CDC* dc, HKCPDisplay* Display, CRect allowedArea, Pen& pen, POINT point, const char* routeTag) {
	CSize routeTagSize = dc->GetTextExtent(routeTag);

	g->DrawLine(&pen, (INT)point.x + LeaderStartX, (INT)point.y + LeaderStartY, (INT)point.x + LeaderElbowX, (INT)point.y + LeaderEndY);
	g->DrawLine(&pen, (INT)point.x + LeaderElbowX, (INT)point.y + LeaderElbowY, (INT)point.x + LeaderEndX, (INT)point.y + LeaderEndY);
	dc->ExtTextOutA(point.x + TextOffsetX + (routeTagSize.cx / 2), point.y + LeaderEndY - (routeTagSize.cy / 2), ETO_CLIPPED, &allowedArea, routeTag, strlen(routeTag), NULL);
}

void AT3RadarTargetDisplay::drawWPTSelectSymbol(Graphics* g, CDC* dc, Pen& pen, SolidBrush& Brush, POINT point) {
	g->DrawEllipse(&pen, point.x - SymbolRadius, point.y - SymbolRadius, SymbolSize, SymbolSize);
	g->FillPie(&Brush, point.x - SymbolRadius, point.y - SymbolRadius, SymbolSize, SymbolSize, 0, 90);
	g->FillPie(&Brush, point.x - SymbolRadius, point.y - SymbolRadius, SymbolSize, SymbolSize, 180, 90);
}

void AT3RadarTargetDisplay::createRouteDraw(CFlightPlan* fp, POINT acftLocation, enum DrawType DrawType, int nextPointID, int probeNextID, Graphics* g, CDC* dc, HKCPDisplay* Display) {
	SolidBrush wptBrush(colorRouteDraw);
	Pen routePen(colorRouteDraw, 1);
	dc->SetTextColor(colorRouteDraw.ToCOLORREF());
	CFlightPlanExtractedRoute extractedRoute = fp->GetExtractedRoute();
	int pointCount = extractedRoute.GetPointsNumber();
	POINT prevPoint;
	POINT nextPoint;
	POINT probeNext;
	bool isFirstPoint = true;
	const char* airway;
	string routeTag;
	CRect allowedArea(0, Display->GetToolbarArea().bottom + TopSky_ToolbarHeight, 99999, 99999);

	time_t curr_time;
	curr_time = time(NULL);
	tm* tm_gmt = gmtime(&curr_time);

	prevPoint = Display->ConvertCoordFromPositionToPixel(extractedRoute.GetPointPosition(nextPointID - 1));
	nextPoint = Display->ConvertCoordFromPositionToPixel(extractedRoute.GetPointPosition(nextPointID));
	probeNext = Display->ConvertCoordFromPositionToPixel(extractedRoute.GetPointPosition(probeNextID));

	for (nextPointID; nextPointID < pointCount; nextPointID++) {
		nextPoint = Display->ConvertCoordFromPositionToPixel(extractedRoute.GetPointPosition(nextPointID));

		if (DrawType == DRAW_ROUTE_TYPE_REGULAR) {
			if (isFirstPoint && extractedRoute.GetPointsAssignedIndex() != -1) {
				airway = "";
			}
			else {
				airway = extractedRoute.GetPointAirwayName(nextPointID);
			}
		}
		else {
			airway = extractedRoute.GetPointAirwayName(nextPointID);
		}

		routeTag = formatRouteTag(extractedRoute, nextPointID, tm_gmt);

		// Type 1: Normal route draw (start from aircraft to next point)
		if (DrawType == DRAW_ROUTE_TYPE_REGULAR) {
			if (extractedRoute.GetPointDistanceInMinutes(nextPointID) == -1 || (std::string(extractedRoute.GetPointName(nextPointID)).length() == 4 && nextPointID == pointCount - 1)) {
				prevPoint = nextPoint;
				continue;
			}
			else if (isFirstPoint) {
				drawRouteLine(g, dc, Display, allowedArea, routePen, acftLocation, nextPoint, airway);
				drawRouteTag(g, dc, Display, allowedArea, routePen, nextPoint, routeTag.c_str());
				isFirstPoint = false;
			}//Else draw point to point
			else {
				drawRouteLine(g, dc, Display, allowedArea, routePen, prevPoint, nextPoint, airway);
				drawRouteTag(g, dc, Display, allowedArea, routePen, nextPoint, routeTag.c_str());
			}
		}
		// Type 2: Off route - Draw from previous point to end
		if (DrawType == DRAW_ROUTE_TYPE_OFF_ROUTE) {
			if (std::string(extractedRoute.GetPointName(nextPointID)).length() == 4 && nextPointID == pointCount - 1) {
				prevPoint = nextPoint;
				continue;
			}
			else if (isFirstPoint) {
				routeTag = formatRouteTag(extractedRoute, nextPointID - 1, tm_gmt);

				drawRouteTag(g, dc, Display, allowedArea, routePen, prevPoint, routeTag.c_str());
				isFirstPoint = false;
			}
			routeTag = formatRouteTag(extractedRoute, nextPointID, tm_gmt);

			drawRouteLine(g, dc, Display, allowedArea, routePen, prevPoint, nextPoint, airway);
			drawRouteTag(g, dc, Display, allowedArea, routePen, nextPoint, routeTag.c_str());
		}
		// Type 3: Probing DCT - Draw from previous point to end + Draw from aircraft to prev and next point
		if (DrawType == DRAW_ROUTE_TYPE_PROBE_DCT) {
			wptBrush.SetColor(colorRouteDrawDCT);
			routePen.SetColor(colorRouteDrawDCT);
			dc->SetTextColor(colorRouteDrawDCT.ToCOLORREF());
			drawRouteLine(g, dc, Display, allowedArea, routePen, acftLocation, prevPoint, "DCT");
			drawRouteLine(g, dc, Display, allowedArea, routePen, acftLocation, probeNext, "DCT");

			routeTag = formatRouteTag(extractedRoute, probeNextID, tm_gmt);

			drawRouteTag(g, dc, Display, allowedArea, routePen, probeNext, routeTag.c_str());

			routeTag = formatRouteTag(extractedRoute, nextPointID - 1, tm_gmt);

			drawRouteTag(g, dc, Display, allowedArea, routePen, prevPoint, routeTag.c_str());

			// Draw BMW logo
			routePen.SetWidth(2);
			drawWPTSelectSymbol(g, dc, routePen, wptBrush, prevPoint);
			drawWPTSelectSymbol(g, dc, routePen, wptBrush, probeNext);
				
			wptBrush.SetColor(colorRouteDraw);
			routePen.SetColor(colorRouteDraw);
			routePen.SetWidth(1);
			dc->SetTextColor(colorRouteDraw.ToCOLORREF());
			DrawType = DRAW_ROUTE_TYPE_PROBE_DCT_NORMAL;
		}
		//Type 4: continue of type 3;
		if (DrawType == DRAW_ROUTE_TYPE_PROBE_DCT_NORMAL) {
			routeTag = formatRouteTag(extractedRoute, nextPointID, tm_gmt);

			if (std::string(extractedRoute.GetPointName(nextPointID)).length() == 4 && nextPointID == pointCount - 1) {
				prevPoint = nextPoint;
				continue;
			}
			airway = extractedRoute.GetPointAirwayName(nextPointID);
			drawRouteLine(g, dc, Display, allowedArea, routePen, prevPoint, nextPoint, airway);
			if (nextPointID == probeNextID) {
				prevPoint = nextPoint;
				continue;
			}

			drawRouteTag(g, dc, Display, allowedArea, routePen, nextPoint, routeTag.c_str());
		}
		prevPoint = nextPoint;
	}
}