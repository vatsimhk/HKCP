#pragma once
#include <curl/curl.h>
#include "EuroScopePlugIn.h"
#include "Constant.hpp"
#include "AtisDisplay.hpp"
#include <vector>
#include <string>	


using namespace std;
using namespace EuroScopePlugIn;

class AtisPlugin :
	public EuroScopePlugIn::CPlugIn

{
public:
	static vector<string> atisLettersDatafeed;

	AtisPlugin();

	~AtisPlugin();

	virtual void OnTimer(int Count);

	virtual CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);
	
	virtual void OnCompilePrivateChat(const char* sSenderCallsign, const char* sReceiverCallsign, const char* sChatMessage);

	void GetDataFeedATIS();
};
