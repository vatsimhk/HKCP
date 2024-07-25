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

RECT AtisDisplay::a_Area = { 50, 100, 175, 196 };
RECT AtisDisplay::b_Area = { 9, 7, 30, 30 };
POINT AtisDisplay::a_Offset = { 125, 96 };
bool AtisDisplay::visible = false;
bool AtisDisplay::highlightSync = false;
vector<string> AtisDisplay::atisLetters = { "-", "-", "-", "-" };

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
	const char* p_value;
	if ((p_value = GetDataFromAsr(string("AtisTopLeftX").c_str())) != NULL) {
		a_Area.left = atoi(p_value);
		a_Area.right = a_Area.left + a_Offset.x;
	}
	if ((p_value = GetDataFromAsr(string("AtisTopLeftY").c_str())) != NULL) {
		a_Area.top = atoi(p_value);
		a_Area.bottom = a_Area.top + a_Offset.y;
	}
}

void AtisDisplay::OnAsrContentToBeSaved() {
	SaveDataToAsr(string("AtisTopLeftX").c_str(), "Atis position", to_string(a_Area.left).c_str());
	SaveDataToAsr(string("AtisTopLeftY").c_str(), "Atis position", to_string(a_Area.top).c_str());
}

void AtisDisplay::OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display) {
	if (Phase != REFRESH_PHASE_AFTER_LISTS)
		return;

	if (!visible)
		return;

	// Basics
	CDC dc;
	dc.Attach(hDC);

	dc.SetTextColor(qTextColor);
	CBrush textBorderBrush(qTextColor);
	CBrush orangeBorderBrush(BUTTON_ORANGE_ON);
	CBrush greyBorderBrush(BUTTON_GREY);
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
	Display->AddScreenObject(DRAWING_APPWINDOW, "atis_window", windowAreaCRect, true, "");


	// Rects
	CRect tempAreaCRect(windowAreaCRect.left + 6, windowAreaCRect.top + 6, windowAreaCRect.left + 57, windowAreaCRect.bottom - 6);
	CRect buttonAreaCRect(b_Area);
	buttonAreaCRect.NormalizeRect();
	CRgn buttonAreaCRgn;
	buttonAreaCRgn.CreateRectRgn(buttonAreaCRect.left, buttonAreaCRect.top, buttonAreaCRect.right, buttonAreaCRect.bottom);

	string hhDepLetter = atisLetters[0];
	string hhArrLetter = atisLetters[1];
	string mcLetter = atisLetters[2];
	string hxLetter = atisLetters[3];

	dc.SelectObject(&fontButton);

	buttonAreaCRect.OffsetRect(windowAreaCRect.TopLeft());
	buttonAreaCRgn.OffsetRgn(windowAreaCRect.TopLeft());
	if (hhDepLetter != AtisPlugin::atisLettersDatafeed[0]) {
		dc.SetTextColor(BUTTON_ORANGE_ON);
		dc.FrameRgn(&buttonAreaCRgn, &orangeBorderBrush, 1, 1);
	}
	else {
		dc.SetTextColor(qTextColor);
		dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	}
	dc.DrawText(hhDepLetter.c_str(), hhDepLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	Display->AddScreenObject(ATIS_LETTER, "vhhh_d", buttonAreaCRect, true, "");

	dc.SetTextColor(qTextColor);
	buttonAreaCRect.OffsetRect(12, 24);
	buttonAreaCRgn.OffsetRgn(12, 24);
	dc.DrawText("HH", 2, buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	buttonAreaCRect.OffsetRect(12, -24);
	buttonAreaCRgn.OffsetRgn(12, -24);
	if (hhArrLetter != AtisPlugin::atisLettersDatafeed[1]) {
		dc.SetTextColor(BUTTON_ORANGE_ON);
		dc.FrameRgn(&buttonAreaCRgn, &orangeBorderBrush, 1, 1);
	}
	else {
		dc.SetTextColor(qTextColor);
		dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	}
	dc.DrawText(hhArrLetter.c_str(), hhArrLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	Display->AddScreenObject(ATIS_LETTER, "vhhh_a", buttonAreaCRect, true, "");

	buttonAreaCRect.OffsetRect(31, 0);
	buttonAreaCRgn.OffsetRgn(31, 0);
	if (mcLetter != AtisPlugin::atisLettersDatafeed[2]) {
		dc.SetTextColor(BUTTON_ORANGE_ON);
		dc.FrameRgn(&buttonAreaCRgn, &orangeBorderBrush, 1, 1);
	}
	else {
		dc.SetTextColor(qTextColor);
		dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	}
	dc.DrawText(mcLetter.c_str(), hxLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	Display->AddScreenObject(ATIS_LETTER, "vmmc", buttonAreaCRect, true, "");

	buttonAreaCRect.OffsetRect(0, 24);
	buttonAreaCRgn.OffsetRgn(0, 24);
	dc.SetTextColor(qTextColor);
	dc.DrawText("MC", 2, buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	buttonAreaCRect.OffsetRect(31, -24);
	buttonAreaCRgn.OffsetRgn(31, -24);
	if (hxLetter != AtisPlugin::atisLettersDatafeed[3]) {
		dc.SetTextColor(BUTTON_ORANGE_ON);
		dc.FrameRgn(&buttonAreaCRgn, &orangeBorderBrush, 1, 1);
	}
	else {
		dc.SetTextColor(qTextColor);
		dc.FrameRgn(&buttonAreaCRgn, &textBorderBrush, 1, 1);
	}
	dc.DrawText(hxLetter.c_str(), mcLetter.length(), buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	Display->AddScreenObject(ATIS_LETTER, "vhhx", buttonAreaCRect, true, "");

	buttonAreaCRect.OffsetRect(0, 24);
	buttonAreaCRgn.OffsetRgn(0, 24);
	dc.SetTextColor(qTextColor);
	dc.DrawText("HX", 2, buttonAreaCRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	CRect syncButton(windowAreaCRect.left + 20, windowAreaCRect.bottom - 25, windowAreaCRect.right - 20, windowAreaCRect.bottom - 5);
	CRgn syncButtonRgn;
	syncButtonRgn.CreateRectRgn(syncButton.left, syncButton.top, syncButton.right, syncButton.bottom);
	if (highlightSync) {
		dc.FrameRgn(&syncButtonRgn, &textBorderBrush, 1, 1);
		dc.SetTextColor(qTextColor);
	}
	else {
		dc.FrameRgn(&syncButtonRgn, &greyBorderBrush, 1, 1);
		dc.SetTextColor(BUTTON_GREY);
	}
	dc.DrawText("SYNC", strlen("SYNC"), syncButton, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	Display->AddScreenObject(SYNC_BUTTON, "", syncButton, true, "");
	
	dc.Detach();
}

void AtisDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	if (strcmp(sObjectId, "atis_window") == 0 || ObjectType == ATIS_LETTER || ObjectType == SYNC_BUTTON) {

		CRect appWindowRect(a_Area);
		appWindowRect.NormalizeRect();

		POINT TopLeft = { Area.left, Area.top };
		POINT BottomRight = { TopLeft.x + appWindowRect.Width(), TopLeft.y + appWindowRect.Height() };
		CRect newPos(TopLeft, BottomRight);
		newPos.NormalizeRect();

		a_Area = newPos;
	}
}

void AtisDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
	if (ObjectType == SYNC_BUTTON && Button == BUTTON_LEFT) {
		AtisPlugin::GetInstance().GetDataFeedATIS();
		atisLetters = AtisPlugin::atisLettersDatafeed;
		return;
	}
	
	if (ObjectType != ATIS_LETTER)
		return;

	if (strcmp(sObjectId, "vhhh_d") == 0) {
		atisLetters[0] = IncrementLetter(atisLetters[0], Button);
	}
	if (strcmp(sObjectId, "vhhh_a") == 0) {
		atisLetters[1] = IncrementLetter(atisLetters[1], Button);
	}
	if (strcmp(sObjectId, "vmmc") == 0) {
		atisLetters[2] = IncrementLetter(atisLetters[2], Button);
	}
	if (strcmp(sObjectId, "vhhx") == 0) {
		atisLetters[3] = IncrementLetter(atisLetters[3], Button);
	}
}

void AtisDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) {
	if (ObjectType == SYNC_BUTTON) {
		highlightSync = true;
	}
	else {
		highlightSync = false;
	}
}

string AtisDisplay::IncrementLetter(string letter, int button) {
	if (letter == "-") {
		return "A";
	}
	if (letter == "Z" && button == BUTTON_LEFT) {
		return "A";
	}
	if (letter == "A" && button == BUTTON_RIGHT) {
		return "Z";
	}

	char newLetter[2];
	strcpy_s(newLetter, letter.c_str());
	if (button == BUTTON_LEFT) {
		newLetter[0]++;
	}
	else if (button == BUTTON_RIGHT) {
		newLetter[0]--;
	}
	return newLetter;
}

bool AtisDisplay::OnCompileCommand(const char* sCommandLine)
{
	if (strcmp(sCommandLine, ".atishide") == 0) {
		visible = false;
		return true;
	}
	if (strcmp(sCommandLine, ".atisshow") == 0) {
		visible = true;
		return true;
	}

	return false;
}
