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

class HKCPDisplay;

class AtisDisplay :
    public EuroScopePlugIn::CRadarScreen
{
protected:
    static RECT a_Area;
    static RECT b_Area;
    static POINT a_Offset;
    static bool visible;
    static bool highlightSync;

public:
    static vector<string> atisLetters;

    AtisDisplay();

    virtual ~AtisDisplay();


    //---OnAsrContentLoaded--------------------------------------------

    virtual void OnAsrContentLoaded(bool Loaded);

    //---OnAsrContentToBeSaved------------------------------------------

    virtual void OnAsrContentToBeSaved();

    void OnRefresh(HDC hDC, int Phase, HKCPDisplay* Display);

    virtual void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);

    virtual void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

    virtual bool OnCompileCommand(const char* sCommandLine);

    virtual void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);

    inline virtual void OnAsrContentToBeClosed(void)
    {
        delete this;
    };

    string IncrementLetter(string letter, int button);

};