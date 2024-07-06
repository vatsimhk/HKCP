#include "stdafx.h"
#include "HKCPDisplay.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include "AT3RadarTargetDisplay.hpp"
#include <gdiplus.h>

using namespace Gdiplus;
using namespace EuroScopePlugIn;

AT3RadarTargetDisplay::AT3RadarTargetDisplay(int _CJSLabelSize, int _CJSLabelOffset, double _PlaneIconScale) :
	CJSLabelSize(_CJSLabelSize), CJSLabelOffset(_CJSLabelOffset), PlaneIconScale(_PlaneIconScale)
{

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

	vector<CFlightPlanPositionPredictions> fppp_vec;
	vector<string> callsigns_vec;

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

		// Add callsigns and position predictions to vector
		callsigns_vec.push_back(acft.GetCallsign());
		CFlightPlanPositionPredictions fppp = fp.GetPositionPredictions();
		fppp_vec.push_back(fppp);

		// Setup container
		GraphicsContainer gContainer = g.BeginContainer();

		// Set brush color based on state
		SolidBrush aircraftBrush(DEFAULT_UNCONCERNED);
		dc.SetTextColor(DEFAULT_UNCONCERNED.ToCOLORREF());
		if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED) {
			aircraftBrush.SetColor(DEFAULT_ASSUMED);
			dc.SetTextColor(DEFAULT_ASSUMED.ToCOLORREF());
		}
		else if (fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED) {
			aircraftBrush.SetColor(DEFAULT_ASSUMED);
			dc.SetTextColor(DEFAULT_REDUNDANT.ToCOLORREF());
		}
		else if (fp.GetState() == FLIGHT_PLAN_STATE_REDUNDANT || fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED) {
			aircraftBrush.SetColor(DEFAULT_REDUNDANT);
			dc.SetTextColor(DEFAULT_REDUNDANT.ToCOLORREF());
		}

		// Override aircraft color conditions
		if (fp.GetSectorEntryMinutes() <= 1 && fp.GetSectorEntryMinutes() >= 0 && strlen(fp.GetTrackingControllerId()) == 0) {
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
		g.ResetTransform();


		Pen CAPen(STCA_RED, 1.5f);
		Point chevronLeft[3] = {
			Point(-10, 10),
			Point(-16, 1),
			Point(-10, -8)
		};
		Point chevronRight[3] = {
			Point(10, 10),
			Point(16, 1),
			Point(10, -8)
		};
		// Loop through all other aircraft to detect conflicts
		for (size_t i = 0; i < fppp_vec.size() - 1; i++) {
			// Skip if on ground
			if (fppp.GetAltitude(0) < 50) {
				break;
			}
			if (fppp_vec[i].GetAltitude(0) < 50) {
				continue;
			}

			// STCA: look up to two minutes in the future
			for (int j = 0; j <= 2; j++) {
				int altDifference = abs(fppp.GetAltitude(j) - fppp_vec[i].GetAltitude(j));
				double latSep = fppp.GetPosition(j).DistanceTo(fppp_vec[i].GetPosition(j));

				if (fppp.GetAltitude(j) < 24500 && latSep < 3 && altDifference < 1000 || 
					fppp.GetAltitude(j) > 24500 && latSep < 5 && altDifference < 1000) {
					string message = "conflict alert between " + callsigns_vec.back() + callsigns_vec[i];
					GetPlugIn()->DisplayUserMessage("STCA", "STCA", message.c_str(), true, true, false, false, false);

					g.TranslateTransform(acftLocation.x, acftLocation.y, MatrixOrderAppend);
					g.DrawLines(&CAPen, chevronLeft, 3);
					g.DrawLines(&CAPen, chevronRight, 3);
					g.ResetTransform();

					POINT otherAcftLocation = Display->ConvertCoordFromPositionToPixel(fppp_vec[i].GetPosition(0));
					g.TranslateTransform(otherAcftLocation.x, otherAcftLocation.y, MatrixOrderAppend);
					g.DrawLines(&CAPen, chevronLeft, 3);
					g.DrawLines(&CAPen, chevronRight, 3);
					g.ResetTransform();
				}
			}

		}

		DeleteObject(&chevronLeft);
		DeleteObject(&chevronRight);

		// Cleanup
		g.EndContainer(gContainer);
		DeleteObject(&aircraftIcon);

		if (fp.GetState() == FLIGHT_PLAN_STATE_ASSUMED) {
			acft = GetPlugIn()->RadarTargetSelectNext(acft);
			continue;
		}

		// Draw CJS
		dc.SelectObject(EuroScopeFont);
		dc.SetTextAlign(TA_CENTER);
		if (fp.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED) {
			dc.TextOutA(acftLocation.x, acftLocation.y - CJSLabelOffset, fp.GetHandoffTargetControllerId());
		}
		else {
			dc.TextOutA(acftLocation.x, acftLocation.y - CJSLabelOffset, fp.GetTrackingControllerId());
		}

		// Increment to next aircraft
		acft = GetPlugIn()->RadarTargetSelectNext(acft);
	}

	// Restore context
	dc.RestoreDC(sDC);

	//De-allocate graphics objects
	dc.Detach();
	g.ReleaseHDC(hDC);
	dc.DeleteDC();
}

void AT3RadarTargetDisplay::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
}
