#include <curl/curl.h>
#include <rapidjson/document.h>
#include "AtisPlugin.hpp"
#include <iostream>
#include <fstream>


using namespace rapidjson;
using namespace EuroScopePlugIn;

vector<string> AtisPlugin::atisLetters = { "-", "-", "-", "-" };

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
        else {
            // DisplayUserMessage("Updater", "", "Got Response!", true, true, false, false, false);
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
    atisLetters = { "-", "-", "-", "-" };
    for (SizeType i = 0; i < atisList.Size(); i++) {
        string callsign = atisList[i]["callsign"].GetString();
        string letter = atisList[i]["atis_code"].GetString();
        if (letter.length() > 1) {
            return;
        }
        if (strcmp(callsign.c_str(), "VHHH_D_ATIS") == 0) {
            atisLetters[0] = letter;
        }
        if (strcmp(callsign.c_str(), "VHHH_A_ATIS") == 0) {
            atisLetters[1] = letter;
        }
        if (strcmp(callsign.c_str(), "VMMC_ATIS") == 0) {
            atisLetters[2] = letter;
        }
        if (strcmp(callsign.c_str(), "VHHX_ATIS") == 0) {
            atisLetters[3] = letter;
        }
    }

}

CRadarScreen* AtisPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated) {
    return new AtisDisplay();
}