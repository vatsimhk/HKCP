#include "stdafx.h"
#include "HKCPDisplay.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include "AT3RadarTargetDisplay.hpp"

using namespace Gdiplus;
using namespace EuroScopePlugIn;

AT3RadarTargetDisplay::AT3RadarTargetDisplay(int _CJSLabelSize, int _CJSLabelOffset, bool _CJSLabelShowWhenTracked, double _PlaneIconScale, COLORREF colorA, COLORREF colorNA, COLORREF colorR) :
	CJSLabelSize(_CJSLabelSize), CJSLabelOffset(_CJSLabelOffset), CJSLabelShowWhenTracked(_CJSLabelShowWhenTracked), PlaneIconScale(_PlaneIconScale)
{
	colorAssumed.SetFromCOLORREF(colorA);
	colorNotAssumed.SetFromCOLORREF(colorNA);
	colorRedundant.SetFromCOLORREF(colorR);
}

void AT3RadarTargetDisplay::OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display)
{
	if (Phase != REFRESH_PHASE_BEFORE_TAGS) {
		return;
	}

	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Save context for later
	int sDC = dc.SaveDC();

	// Create graphics object
	Graphics g(hDC);

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
		if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED) {
			aircraftBrush.SetColor(colorAssumed);
			dc.SetTextColor(colorAssumed.ToCOLORREF());
		}
		else if (fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED) {
			aircraftBrush.SetColor(colorAssumed);
			dc.SetTextColor(colorRedundant.ToCOLORREF());
		}
		else if (fp.GetState() == FLIGHT_PLAN_STATE_REDUNDANT || fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED) {
			aircraftBrush.SetColor(colorRedundant);
			dc.SetTextColor(colorRedundant.ToCOLORREF());
		}

		// Override aircraft color conditions
		if (pd.GetPressureAltitude() > 100 && strlen(fp.GetTrackingControllerId()) == 0 &&
			fp.GetSectorEntryMinutes() <= 1 && fp.GetSectorEntryMinutes() >= 0 ) {
			aircraftBrush.SetColor(OVERRIDE_AIW);
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
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		// Define aircraft icon
		Point aircraftIcon[19] = {
			Point(0,-7),
			Point(-1,-6),
			Point(-1,-2),
			Point(-7,3),
			Point(-7,4),
			Point(-1,2),
			Point(-1,6),
			Point(-4,8),
			Point(-4,9),
			Point(0,8),
			Point(4,9),
			Point(4,8),
			Point(1,6),
			Point(1,2),
			Point(7,4),
			Point(7,3),
			Point(1,-2),
			Point(1,-6),
			Point(0,-7)
		};

		// Draw the aircraft icon
		g.FillPolygon(&aircraftBrush, aircraftIcon, 19);

		// Cleanup
		g.EndContainer(gContainer);
		DeleteObject(&aircraftIcon);

		if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED && !CJSLabelShowWhenTracked) {
			acft = GetPlugIn()->RadarTargetSelectNext(acft);
			continue;
		}

		// Draw CJS
		dc.SelectObject(EuroScopeFont);
		dc.SetTextAlign(TA_CENTER);
		CSize CJSLabelSize;

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
		} else if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED) {
			if (CJSLabelShowFreq[fp.GetCallsign()]) {
				CJSLabelText = GetControllerFreqFromId(GetControllerIdFromCallsign(fp.GetCoordinatedNextController()));
				dc.SetTextColor(colorAssumed.ToCOLORREF());
			}
			else {
				CJSLabelText = GetControllerIdFromCallsign(fp.GetCoordinatedNextController());
			}
		} else {
			if (CJSLabelShowFreq[fp.GetCallsign()]) {
				CJSLabelText = GetControllerFreqFromId(fp.GetTrackingControllerId());
				dc.SetTextColor(colorAssumed.ToCOLORREF());
			} else {
				CJSLabelText = fp.GetTrackingControllerId();
			}
		}

		// Remove trailing up to two trailing zeroes
		for (int i = 0; i < 2; i++) {
			if (CJSLabelText.back() == '0') {
				CJSLabelText.pop_back();
			}
		}
		dc.TextOutA(acftLocation.x, acftLocation.y - CJSLabelOffset, CJSLabelText.c_str());

		// Create rectangle around CJS label for click spot
		CJSLabelSize = dc.GetTextExtent(CJSLabelText.c_str());
		POINT CJSLabelPoint = { acftLocation.x - CJSLabelSize.cx / 2, acftLocation.y - CJSLabelOffset};
		CRect CJSLabelRect(CJSLabelPoint, CJSLabelSize);
		Display->AddScreenObject(CJS_INDICATOR, fp.GetCallsign(), CJSLabelRect, true, "");

		// Increment to next aircraft
		acft = GetPlugIn()->RadarTargetSelectNext(acft);
	}

	drawConflictChevrons(Display->STCA->Alerts, STCA_RED, g, Display);
	drawConflictChevrons(Display->STCA->mediumAlerts, MTCA_YELLOW, g, Display);

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

void AT3RadarTargetDisplay::drawConflictChevrons(vector<string> acftCallsigns, Color alertColor, Graphics& g, HKCPDisplay* Display)
{
	for (auto& callsign : acftCallsigns) {
		// Setup container
		GraphicsContainer gContainer = g.BeginContainer();

		// Define STCA Chevron Graphics
		Pen CAPen(alertColor, 1.5f);
		Point chevronLeft[3] = {
			Point(-10, 9),
			Point(-15, 1),
			Point(-10, -7)
		};
		Point chevronRight[3] = {
			Point(10, 9),
			Point(15, 1),
			Point(10, -7)
		};

		// Select radar target
		auto acft = GetPlugIn()->RadarTargetSelect(callsign.c_str());
		POINT acftLocation = Display->ConvertCoordFromPositionToPixel(acft.GetPosition().GetPosition());

		// Draw STCA chevrons
		g.TranslateTransform(acftLocation.x, acftLocation.y, MatrixOrderAppend);
		g.DrawLines(&CAPen, chevronLeft, 3);
		g.DrawLines(&CAPen, chevronRight, 3);
		g.ResetTransform();

		g.EndContainer(gContainer);
	}
}
