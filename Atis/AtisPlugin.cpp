#include "stdafx.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include "AtisPlugin.hpp"
#include "AtisDisplay.hpp"
#include <iostream>
#include <fstream>


using namespace rapidjson;
using namespace EuroScopePlugIn;

vector<string> AtisPlugin::atisLettersDatafeed = { "-", "-", "-", "-" };

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response)
{
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

AtisPlugin::AtisPlugin() : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT) {

}

AtisPlugin::~AtisPlugin() {

}

void AtisPlugin::OnTimer(int Count) {
    if (Count % 15 != 0)
        return;

    GetDataFeedATIS();

    for (int i = 0; i <= 3; i++) {
        if (atisLettersDatafeed[i] == "Z" && AtisDisplay::atisLetters[i] == "A") {
            continue;
        }
        if (atisLettersDatafeed[i].compare(AtisDisplay::atisLetters[i]) > 0 || //Datafeed is ahead of displayed letter
            AtisDisplay::atisLetters[i] == "-" || //No displayed letter yet
            (atisLettersDatafeed[i] == "A" && AtisDisplay::atisLetters[i] == "Z")) //Handle special case of looping back to A
        {
            AtisDisplay::atisLetters[i] = atisLettersDatafeed[i]; //Update display with datafeed letter
        }
    }
}

void AtisPlugin::GetDataFeedATIS() {
    string url = "https://data.vatsim.net/v3/vatsim-data.json";

    string response;
    CURL* curl = curl_easy_init();
    string message;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            message.append("API request failed: ");
            message.append(curl_easy_strerror(res));
            DisplayUserMessage("Updater", "", message.c_str(), true, true, false, false, false);
            return;
        }

        curl_easy_cleanup(curl);
    }


    // Parse the response to extract the latest tag name
    // Assuming the response is in JSON format, you can use a JSON library to parse it
    Document document;
    document.Parse<0>(response.c_str());

    Value& atisList = document["atis"];
    assert(atisList.IsArray());

    //reset ATIS letters
    atisLettersDatafeed = { "-", "-", "-", "-" };
    for (SizeType i = 0; i < atisList.Size(); i++) {
        string callsign = atisList[i]["callsign"].GetString();
        string letter = atisList[i]["atis_code"].GetString();
        if (letter.length() > 1) {
            return;
        }
        if (strcmp(callsign.c_str(), "VHHH_D_ATIS") == 0) {
            atisLettersDatafeed[0] = letter;
        }
        if (strcmp(callsign.c_str(), "VHHH_A_ATIS") == 0) {
            atisLettersDatafeed[1] = letter;
        }
        if (strcmp(callsign.c_str(), "VMMC_ATIS") == 0) {
            atisLettersDatafeed[2] = letter;
        }
        if (strcmp(callsign.c_str(), "VHHX_ATIS") == 0) {
            atisLettersDatafeed[3] = letter;
        }
    }
}

void AtisPlugin::OnCompilePrivateChat(const char* sSenderCallsign, const char* sReceiverCallsign, const char* sChatMessage) {
    /*
    Unable to detect EuroScope generated messages, maybe will fix one day
    
    DisplayUserMessage("Debug", "Debug", "Detected a private message", true, true, false, false, false);
    if (strstr(sChatMessage, "New ATIS for ") == NULL) {
        return;
    }
    const char* letter;
    letter = strstr(sChatMessage, "VHHH");
    if (letter != NULL) {
        letter += 8;
        if (strcmp(sSenderCallsign, "VHHH_D_ATIS") == 0) {
            atisLetters[0] = *letter;
            return;
        }
        if (strcmp(sSenderCallsign, "VHHH_A_ATIS") == 0) {
            atisLetters[1] = *letter;
            return;
        }
    }
    letter = strstr(sChatMessage, "VMMC");
    if (letter != NULL && strcmp(sSenderCallsign, "VMMC_ATIS") == 0) {
        letter += 8;
        atisLetters[2] = *letter;
        return;
    }
    letter = strstr(sChatMessage, "VHHX");
    if (letter != NULL && strcmp(sSenderCallsign, "VHHX_ATIS") == 0) {
        letter += 8;
        atisLetters[3] = *letter;
        return;
    }*/
}

CRadarScreen* AtisPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated) {
    return new AtisDisplay();
}