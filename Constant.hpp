#pragma once
#include "stdafx.h"

#define MY_PLUGIN_NAME      "HKCP"
#define MY_PLUGIN_VERSION   "2.0"
#define MY_PLUGIN_DEVELOPER "HKvACC, Jan Fries, Hendrik Peter, Sven Czarnian"
#define MY_PLUGIN_COPYRIGHT "GPL v3"
#define MY_PLUGIN_VIEW_AVISO  "Hong Kong Controller Plugin"

#define PLUGIN_WELCOME_MESSAGE	"Welcome to the HKvACC Controlelr Plugin"

const int TAG_ITEM_FPCHECK = 1;
const int TAG_ITEM_FPCHECK_IF_FAILED = 2;
const int TAG_ITEM_FPCHECK_IF_FAILED_STATIC = 3;

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

inline static bool startsWith(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre), lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
};