#include "stdafx.h"
#include "AtisDisplay.hpp"
#include "AtisPlugin.hpp"
#include <sstream>
#include <vector>
#include <thread>
#include <mmsystem.h>
#include <string>
#include <iostream>

using namespace EuroScopePlugIn;

RECT AtisDisplay::a_Area = { 50, 100, 175, 161 };
RECT AtisDisplay::b_Area = { 9, 7, 30, 30 };
POINT AtisDisplay::a_Offset = { 125, 61 };

AtisDisplay::AtisDisplay() {

}

AtisDisplay::~AtisDisplay() {
	try {
		this->OnAsrContentToBeSaved();
		//this->EuroScopePlugInExitCustom();
	}
	catch (exception& e) {
		stringstream s;
		s << e.what() << endl;
		AfxMessageBox(string("Error occurred " + s.str()).c_str());
	}
}

void AtisDisplay::OnAsrContentLoaded(bool Loaded) {

}

void AtisDisplay::OnAsrContentToBeSaved() {

}

void AtisDisplay::OnRefresh(HDC hDC, int Phase) {
	if (Phase != REFRESH_PHASE_AFTER_LISTS)
		return;

	// Basics
	CDC dc;
	dc.Attach(hDC);

	dc.SetTextColor(qTextColor);
	CBrush textBorderBrush(qTextColor);
	CBrush borderBrush(qBorderColor);
	CBrush backgroundBrush(qBackgroundColor);

	// Fonts
	CFont fontButton, fontTimer;
	fontButton.CreateFont(15, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));
	fontTimer.CreateFont(18, 0, 0, 0, 0, false, false,
		0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, _T("Arial"));

	CFont* oldFont = dc.SelectObject(&fontTimer);

	CRect windowAreaCRect(a_Area);
	windowAreaCRect.NormalizeRect();

	dc.FillSolidRect(windowAreaCRect, qBackgroundColor);
	dc.FrameRect(windowAreaCRect, &borderBrush);
	AddScreenObject(DRAWING_APPWINDOW, "window", windowAreaCRect, true, "");


	// Rects
	CRect tempAreaCRect(windowAreaCRect.left + 6, windowAreaCRect.top + 6, windowAreaCRect.left + 57, windowAreaCRect.bottom - 6);
	CRect buttonAreaCRect(b_Area);
	buttonAreaCRect.NormalizeRect();
	CRgn buttonAreaCRgn;
	buttonAreaCRgn.CreateRectRgn(buttonAreaCRect.left, buttonAreaCRect.top, buttonAreaCRect.right, buttonAreaCRect.bottom);

	string hhDepLetter = AtisPlugin::atisLetters[0];
	string hhArrLetter = AtisPlugin::atisLetters[1];
	string mcLetter = AtisPlugin::atisLetters[2];
	string hxLetter = AtisPlugin::atisLetters[3];

	dc.SelectObject(&fontButton);

	buttonAreaCRect.OffsetRect(windowAreaCRect.TopLeft());
	buttonAreaCRgn.OffsetRgn(windowAreaCRect.TopLeft());
	dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	dc.DrawText(hhDepLetter.c_str(), hhDepLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	buttonAreaCRect.OffsetRect(12, 24);
	buttonAreaCRgn.OffsetRgn(12, 24);
	dc.DrawText("HH", 2, buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);


	buttonAreaCRect.OffsetRect(12, -24);
	buttonAreaCRgn.OffsetRgn(12, -24);
	dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	dc.DrawText(hhArrLetter.c_str(), hhArrLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	buttonAreaCRect.OffsetRect(31, 0);
	buttonAreaCRgn.OffsetRgn(31, 0);
	dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	dc.DrawText(mcLetter.c_str(), hxLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	buttonAreaCRect.OffsetRect(0, 24);
	buttonAreaCRgn.OffsetRgn(0, 24);
	dc.DrawText("MC", 2, buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	buttonAreaCRect.OffsetRect(31, -24);
	buttonAreaCRgn.OffsetRgn(31, -24);
	dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	dc.DrawText(hxLetter.c_str(), mcLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	buttonAreaCRect.OffsetRect(0, 24);
	buttonAreaCRgn.OffsetRgn(0, 24);
	dc.DrawText("HX", 2, buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	dc.Detach();
}

void AtisDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	if (strcmp(sObjectId, "window") == 0) {

		CRect appWindowRect(a_Area);
		appWindowRect.NormalizeRect();

		POINT TopLeft = { Area.left, Area.top };
		POINT BottomRight = { TopLeft.x + appWindowRect.Width(), TopLeft.y + appWindowRect.Height() };
		CRect newPos(TopLeft, BottomRight);
		newPos.NormalizeRect();

		a_Area = newPos;
	}
}