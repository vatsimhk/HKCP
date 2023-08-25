#pragma once
#include "EuroScopePlugIn.h"
#include "Constant.hpp"
#include "HKCPDisplay.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <map>


using namespace std;
using namespace EuroScopePlugIn;

class AtisDisplay :
    public EuroScopePlugIn::CRadarScreen
{
protected:
    static RECT a_Area;
    static RECT b_Area;
    static POINT a_Offset;

public:
    AtisDisplay();

    virtual ~AtisDisplay();


    //---OnAsrContentLoaded--------------------------------------------

    virtual void OnAsrContentLoaded(bool Loaded);

    //---OnAsrContentToBeSaved------------------------------------------

    virtual void OnAsrContentToBeSaved();

    void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

    virtual void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);

    virtual bool OnCompileCommand(const char* sCommandLine);

    inline virtual void OnAsrContentToBeClosed(void)
    {
        delete this;
    };
};