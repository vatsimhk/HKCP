#pragma once
#include <curl/curl.h>
#include "EuroScopePlugIn.h"
#include "Constant.hpp"
#include "AtisDisplay.hpp"
#include <vector>
#include <string>	
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>


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

	static AtisPlugin& GetInstance() {
		static AtisPlugin instance;
		return instance;
	}

private:
	// threading by task
	std::thread sync_thread;
	std::thread update_check_thread;
	std::atomic<bool> stop_requested;

	void GetDataFeedATISAsync();
};
