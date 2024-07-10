#pragma once
#include "stdafx.h"
#include <gdiplus.h>

#define MY_PLUGIN_NAME      "HKCP"
#define MY_PLUGIN_VERSION   "2.0"
#define MY_PLUGIN_DEVELOPER "HKvACC, Jan Fries, Hendrik Peter, Sven Czarnian"
#define MY_PLUGIN_COPYRIGHT "GPL v3"
#define MY_PLUGIN_VIEW_AVISO  "Hong Kong Controller Plugin"

#define PLUGIN_WELCOME_MESSAGE	"Welcome to the HKvACC Controller Plugin"

const int TAG_ITEM_FPCHECK = 1;
const int TAG_ITEM_FPCHECK_IF_FAILED = 2;
const int TAG_ITEM_FPCHECK_IF_FAILED_STATIC = 3;

const int TAG_ITEM_AT3_ALTITUDE = 10;
const int TAG_ITEM_AT3_ALTITUDE_ASSIGNED = 11;
const int TAG_ITEM_AT3_TRACK = 12;
const int TAG_ITEM_AT3_HEADING_ASSIGNED = 13;
const int TAG_ITEM_AT3_SPEED = 14;
const int TAG_ITEM_AT3_SPEED_ASSIGNED = 15;
const int TAG_ITEM_AT3_ROUTE_CODE = 16;
const int TAG_ITEM_AT3_APPDEP_LINE4 = 17;
const int TAG_ITEM_AT3_AMC_LINE4 = 18;
const int TAG_ITEM_AT3_SLOT = 19;
const int TAG_ITEM_AT3_CALLSIGN = 20;
const int TAG_ITEM_AT3_ATYPWTC = 21;
const int TAG_ITEM_AT3_VS_INDICATOR = 22;
const int TAG_ITEM_AT3_ARRIVAL_RWY = 23;

const int TAG_FUNC_APP_SEL_MENU = 200;
const int TAG_FUNC_APP_SEL_ITEM_1 = 201;
const int TAG_FUNC_APP_SEL_ITEM_2 = 202;
const int TAG_FUNC_APP_SEL_ITEM_3 = 203;
const int TAG_FUNC_APP_SEL_ITEM_4 = 204;
const int TAG_FUNC_APP_SEL_ITEM_5 = 205;
const int TAG_FUNC_APP_SEL_ITEM_6 = 206;
const int TAG_FUNC_APP_SEL_ITEM_7 = 207;
const int TAG_FUNC_APP_SEL_ITEM_8 = 208;
const int TAG_FUNC_APP_SEL_DUMMY = 209;

const int TAG_FUNC_RTE_SEL_MENU = 300;
const int TAG_FUNC_RTE_SEL_ITEM_1 = 301;
const int TAG_FUNC_RTE_SEL_ITEM_2 = 302;
const int TAG_FUNC_RTE_SEL_ITEM_3 = 303;
const int TAG_FUNC_RTE_SEL_ITEM_4 = 304;
const int TAG_FUNC_RTE_SEL_ITEM_5 = 305;
const int TAG_FUNC_RTE_SEL_ITEM_6 = 306;
const int TAG_FUNC_RTE_SEL_ITEM_7 = 307;
const int TAG_FUNC_RTE_SEL_ITEM_8 = 308;
const int TAG_FUNC_RTE_SEL_DUMMY = 309;

const int TAG_FUNC_CHECKFP_MENU = 100;
const int TAG_FUNC_CHECKFP_CHECK = 101;
const int TAG_FUNC_ON_OFF = 102;
const int TAG_FUNC_CHECKFP_FLAS = 103;
const int TAG_FUNC_FL_UP = 104;
const int TAG_FUNC_FL_DOWN = 105;
const int TAG_FUNC_MODRFL_MENU = 106;

const int DRAWING_APPWINDOW = 1211;
const int ACK_BUTTON = 1212;
const int RWY_ENABLE_BUTTON = 1213;
const int WINDOW_TITLE_BAR = 1214;
const int ACT_BUTTON = 1215;
const int RESET_BUTTON = 1216;
const int ATIS_LETTER = 1217;
const int SYNC_BUTTON = 1218;
const int CJS_INDICATOR = 1219;

const COLORREF TAG_GREEN = RGB(0, 190, 0);
const COLORREF TAG_GREY = RGB(128, 128, 128);
const COLORREF TAG_RED = RGB(240, 40, 40);
const COLORREF TAG_YELLOW = RGB(220, 220, 0);
const COLORREF TAG_CYAN = RGB(114, 216, 250);
const COLORREF TAG_BLUE = RGB(0, 90, 255);
const COLORREF BLACK = RGB(0, 0, 0);

const COLORREF qBackgroundColor(RGB(26, 26, 26));
const COLORREF qHighlightColor(RGB(125, 125, 125));
const COLORREF qBorderColor(RGB(45, 45, 48));
const COLORREF qTextColor(RGB(255, 255, 255));
const COLORREF BUTTON_ORANGE_OFF = RGB(159, 80, 0);
const COLORREF BUTTON_ORANGE_ON = (RGB(255, 128, 0));
const COLORREF BUTTON_GREEN = RGB(91, 194, 54);
const COLORREF BUTTON_RED_OFF = RGB(120, 10, 10);
const COLORREF BUTTON_RED_ON = RGB(240, 50, 50);
const COLORREF BUTTON_GREY = RGB(140, 140, 140);

const Gdiplus::Color DEFAULT_ASSUMED = Gdiplus::Color(241, 246, 255);
const Gdiplus::Color DEFAULT_UNCONCERNED = Gdiplus::Color(117, 132, 142);
const Gdiplus::Color DEFAULT_REDUNDANT = Gdiplus::Color(229, 214, 130);
const Gdiplus::Color OVERRIDE_AIW = Gdiplus::Color(255, 158, 112);
const Gdiplus::Color OVERRIDE_EMER = Gdiplus::Color(255, 0, 0);

inline static bool startsWith(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre), lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
};