#include "stdafx.h"
#include "HKCPDisplay.hpp"
#include "Constant.hpp"
#include "EuroScopePlugIn.h"
#include "AT3RadarTargetDisplay.hpp"
#include <gdiplus.h>

using namespace Gdiplus;
using namespace EuroScopePlugIn;

AT3RadarTargetDisplay::AT3RadarTargetDisplay() {

}

void AT3RadarTargetDisplay::OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display)
{
	if (Phase != REFRESH_PHASE_BEFORE_TAGS) {
		return;
	}

	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Create graphics object
	Graphics g(hDC);

	// Select first aircraft
	CRadarTarget acft;
	acft = GetPlugIn()->RadarTargetSelectFirst();

	// Loop through all aircrafts
	while (acft.IsValid()) {
		// Setup containers and brushess
		GraphicsContainer gContainer = g.BeginContainer();
		SolidBrush assumedBrush(Color(241, 246, 255));
		SolidBrush notAssumedBrush(Color(117, 132, 142));

		// Get Flight plan
		CFlightPlan fp = Display->GetPlugIn()->FlightPlanSelect(acft.GetCallsign());

		// Get and set location
		POINT acftLocation = Display->ConvertCoordFromPositionToPixel(acft.GetPosition().GetPosition());
		g.ScaleTransform(1.2, 1.2, MatrixOrderAppend);
		g.TranslateTransform(acftLocation.x, acftLocation.y, MatrixOrderAppend);
		g.RotateTransform(acft.GetPosition().GetReportedHeadingTrueNorth());

		// Set Anti-aliasing
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		// Define aircraft icon
		Point aircraftIcon[19] = {
			Point(0,-8),
			Point(-1,-7),
			Point(-1,-2),
			Point(-8,3),
			Point(-8,4),
			Point(-1,2),
			Point(-1,7),
			Point(-4,9),
			Point(-4,10),
			Point(0,9),
			Point(4,10),
			Point(4,9),
			Point(1,7),
			Point(1,2),
			Point(8,4),
			Point(8,3),
			Point(1,-2),
			Point(1,-7),
			Point(0,-8)
		};

		if (fp.GetTrackingControllerIsMe()) {
			g.FillPolygon(&assumedBrush, aircraftIcon, 19);
		}
		else {
			g.FillPolygon(&notAssumedBrush, aircraftIcon, 19);
		}

		g.EndContainer(gContainer);
		DeleteObject(&aircraftIcon);

		// Increment to next aircraft
		acft = GetPlugIn()->RadarTargetSelectNext(acft);
	}

	//De-allocate graphics objects
	dc.Detach();
	g.ReleaseHDC(hDC);
	dc.DeleteDC();
}
