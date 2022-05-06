#pragma once
#include "stdafx.h"

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

const COLORREF TAG_GREEN = RGB(0, 190, 0);
const COLORREF TAG_GREY = RGB(128, 128, 128);
const COLORREF TAG_RED = RGB(240, 40, 40);
const COLORREF TAG_YELLOW = RGB(220, 220, 0);
const COLORREF TAG_CYAN = RGB(114, 216, 250);
const COLORREF TAG_BLUE = RGB(0, 90, 255);

inline static bool startsWith(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre), lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
};