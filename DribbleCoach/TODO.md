```
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <curl/curl.h>  // For HTTP requests

class ReplayNotifierPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
    virtual void onLoad();
    virtual void onUnload();

private:
    void onReplayStarted();
    void sendReplayInfo(const std::string& replayFilePath);
};

void ReplayNotifierPlugin::onLoad()
{
    gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GameEvent_Replay_TA.Init",
        [this](ServerWrapper caller, void* params, std::string eventName) {
            onReplayStarted();
        });
}

void ReplayNotifierPlugin::onUnload()
{
    // Clean up if necessary
}

void ReplayNotifierPlugin::onReplayStarted()
{
    std::string replayFilePath = gameWrapper->GetCurrentReplayName();
    std::cout << "Replay started: " << replayFilePath << std::endl;

    // Send the replay info to the ReplayAssistant
    sendReplayInfo(replayFilePath);
}

void ReplayNotifierPlugin::sendReplayInfo(const std::string& replayFilePath)
{
    // Example using cURL to send a POST request to a local server
    CURL* curl = curl_easy_init();
    if (curl)
    {
        const std::string url = "http://localhost:5000/replay";
        std::string postData = "replayFilePath=" + replayFilePath;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}
```