#include "pch.h"
#include <fstream>
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <sstream>

#include "DribbleCoach.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/GameEvent/TutorialWrapper.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"
#include "bakkesmod/wrappers/GameWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod/wrappers/ControllerWrapper.h" 
#include "bakkesmod/wrappers/ReplayServerWrapper.h"

BAKKESMOD_PLUGIN(DribbleCoach, "AI Dribble Coach", plugin_version, PLUGINTYPE_THREADED)

bool isDribblen = false;
std::string query = "";
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
const std::string le_prompt = "checking out this replay of a car and ball practicing dribbling, until it touches ground. Give a brief one sentence recommendation to the player saying what was done well and/or what could improve.  Make your response no longer than nine words.";
const std::string data_row_header = "time, car_x, car_y, car_z, ball_x, ball_y, ball_z";
const std::string replay_prepare = "replay_prepare";
const std::string replay_prompt = "replay_prompt";
std::string replay_assistant_id = "";
std::string yonder_ai_text = "";

void DribbleCoach::onLoad()
{
    _globalCvarManager = cvarManager;
    cvarManager->registerCvar("anthropic_api_key", "YOUR_SECRET_KEY", "API key for the Anthropic AI service", true);
    cvarManager->registerNotifier(replay_prepare, std::bind(&DribbleCoach::OnCommand, this, std::placeholders::_1), "Starts/stops replay analysis", PERMISSION_REPLAY);
    cvarManager->registerNotifier(replay_prompt, std::bind(&DribbleCoach::OnCommand, this, std::placeholders::_1), "Queries AI on current replay", PERMISSION_REPLAY);

    gameWrapper->HookEvent("Function TAGame.Ball_TA.OnRigidBodyCollision", std::bind(&DribbleCoach::OnDroppedBall, this, std::placeholders::_1));
    resetKey = this->gameWrapper->GetFNameIndexByString("XboxTypeS_DPad_Up");
    gameWrapper->RegisterDrawable(std::bind(&DribbleCoach::Render, this, std::placeholders::_1));
    LOG("Loaded.");
}

void DribbleCoach::onReplayAssistant()
{
    ReplayServerWrapper gew = gameWrapper->GetGameEventAsReplay();
    LOG("Preparing replay: " + gew.GetReplay().GetId().ToString());
    server_thread = std::thread(std::bind(&DribbleCoach::prepareReplay, this, gew.GetReplay().GetId().ToString()));
    server_thread.detach();
}

void DribbleCoach::onQueryAssistant(std::vector<std::string> params)
{
    ReplayServerWrapper gew = gameWrapper->GetGameEventAsReplay();
    LOG("Prompting replay: " + gew.GetReplay().GetId().ToString());
    server_thread = std::thread(std::bind(&DribbleCoach::queryReplayAssistant, this, gew.GetReplay().GetId().ToString(), params));
    server_thread.detach();
}

void DribbleCoach::OnCommand(std::vector<std::string> params)
{
    std::string command = params.at(0);
        
    if (command.compare(replay_prepare) == 0) {
        this->onReplayAssistant();
    } else if (command.compare(replay_prompt) == 0) {
        params.erase(params.begin());
        this->onQueryAssistant(params);
    }
}

void DribbleCoach::Render(CanvasWrapper canvas)
{
    if (!gameWrapper->IsInFreeplay())
        return;

    ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();

    if (tutorial.GetGameBalls().Count() == 0)
        return;

    BallWrapper ball = tutorial.GetGameBalls().Get(0);
    CarWrapper car = tutorial.GetGameCar();
   
    if (ball.IsNull() || car.IsNull())
        return;

    // Detect if dribble reset
    if (gameWrapper->IsKeyPressed(resetKey)) {

        Vector playerVelocity = car.GetVelocity();
        Vector addToBall = Vector(playerVelocity.X, playerVelocity.Y, 170);

        addToBall.X = max(min(20.0f, addToBall.X), -20.0f);
        addToBall.Y = max(min(30.0f, addToBall.Y), -30.0f);

        ball.SetLocation(car.GetLocation() + addToBall);
        ball.SetVelocity(playerVelocity);
        isDribblen = true;
        yonder_ai_text = "";

        playbackData.push_back(le_prompt);
        playbackData.push_back(data_row_header);

        this->OnRecordTick();
        //gameWrapper->UnregisterDrawables();
        return;
    }

    if (yonder_ai_text.length() == 0)
        return;

    canvas.SetColor(255, 255, 255, 255);
    canvas.DrawString(yonder_ai_text, 3, 3);

}

void DribbleCoach::OnDroppedBall(std::string eventName)
{
    if (isDribblen) {
        if (!gameWrapper->IsInFreeplay())
            return;
        ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();
        if (tutorial.GetGameBalls().Count() == 0)
            return;

        BallWrapper ball = tutorial.GetGameBalls().Get(0);
        CarWrapper car = tutorial.GetGameCar();
        if (ball.IsNull() || car.IsNull())
            return;

        Vector ballLocation = ball.GetLocation();
        Vector ballVelocity = ball.GetVelocity();

        // Ball is near or on the ground
        if (ballLocation.Z <= 94) {

            isDribblen = false;
            LOG("Querying AI...");

            std::string output = "";
            for (unsigned int i = 0; i < playbackData.size(); i++) output += playbackData.at(i) + " ";

            server_thread = std::thread(std::bind(&DribbleCoach::AskAnthropic, this, output));
            server_thread.detach();

            playbackData.clear();
            playbackData.push_back(le_prompt);
            playbackData.push_back(data_row_header);

        }
    }
    this->OnRecordTick();
}

void DribbleCoach::OnRecordTick()
{
    if (!isDribblen)
        return;

    ServerWrapper server = gameWrapper->GetGameEventAsServer();
    auto players = gameWrapper->GetGameEventAsServer().GetCars();

    if (!gameWrapper->IsInFreeplay())
        return;
    ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();
    if (tutorial.GetGameBalls().Count() == 0)
        return;

    BallWrapper ball = tutorial.GetGameBalls().Get(0);
    CarWrapper car = tutorial.GetGameCar();
    if (ball.IsNull() || car.IsNull())
        return;

    std::string data_string = std::to_string(server.GetSecondsElapsed()) + "," + std::to_string(car.GetLocation().X) + "," + std::to_string(car.GetLocation().Y) + "," + std::to_string(car.GetLocation().Z) + "," + std::to_string(ball.GetLocation().X) + "," + std::to_string(ball.GetLocation().Y) + "," + std::to_string(ball.GetLocation().Z);
    playbackData.push_back(data_string);

}

void DribbleCoach::AskAnthropic(std::string prompt) {
    const std::string url = "https://api.anthropic.com/v1/messages";
    const std::string data = "{\"model\": \"claude-3-5-sonnet-20241022\", \"max_tokens\": 8192,\"messages\": [{\"role\": \"user\", \"content\": \"" + prompt + "\"}]}";
    std::string secret = cvarManager->getCvar("anthropic_api_key").getStringValue();

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"DribbleCoach/1.0",                 // User agent
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,          // Access type
        WINHTTP_NO_PROXY_NAME,                      // Proxy name
        WINHTTP_NO_PROXY_BYPASS,                    // Proxy bypass
        0);

    if (hSession == NULL) {
        LOG("WinHttpOpen failed: " + std::to_string(GetLastError()));
        return;
    }

    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"api.anthropic.com",                       // Server name
        INTERNET_DEFAULT_HTTPS_PORT,                // HTTPS port (443)
        0);

    if (hConnect == NULL) {
        WinHttpCloseHandle(hSession);
        LOG("WinHttpConnect failed: " + std::to_string(GetLastError()));
        return;
    }

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,                                   // Connection handle
        L"POST",                                    // HTTP method
        L"/v1/messages",                            // Path (API endpoint)
        NULL,                                       // HTTP version (use default)
        WINHTTP_NO_REFERER,                         // Referrer (not needed)
        WINHTTP_NO_ADDITIONAL_HEADERS,              // Additional headers (not needed)
        WINHTTP_FLAG_SECURE);                       // Enable secure connection (HTTPS)

    if (hRequest == NULL) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpOpenRequest failed: " + std::to_string(GetLastError()));
        return;
    }

    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"anthropic-version: 2023-06-01\r\n";
    headers += L"x-api-key: " + std::wstring(secret.begin(), secret.end()) + L"\r\n";

    BOOL result = WinHttpSendRequest(
        hRequest,
        headers.c_str(),                            // Headers
        -1,                                         // Length of the header
        (LPVOID)data.c_str(),                       // Request body
        data.size(),                                // Body length
        data.size(),                                // Total length
        0);                                         // Context (optional)

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpSendRequest failed: " + std::to_string(GetLastError()));
        return;
    }

    result = WinHttpReceiveResponse(hRequest, NULL);

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpReceiveResponse failed: " + std::to_string(GetLastError()));
        return;
    }

    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    char buffer[4096];  
    std::stringstream responseStream;

    while (WinHttpReadData(hRequest, (LPVOID)buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {

        responseStream.write(buffer, bytesRead);
        totalBytesRead += bytesRead;

        if (totalBytesRead >= 100000) {
            LOG("Warning: Response data exceeds 100KB");
            return;
        }
    }
    const std::string why_dribble = responseStream.str();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);


    LOG(this->TrimString(why_dribble));

}

void DribbleCoach::prepareReplay(const std::string& replayFileName)
{

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"DribbleCoach/1.0",                 // User agent
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,          // Access type
        WINHTTP_NO_PROXY_NAME,                      // Proxy name
        WINHTTP_NO_PROXY_BYPASS,                    // Proxy bypass
        0);

    if (hSession == NULL) {
        LOG("WinHttpOpen failed: " + std::to_string(GetLastError()));
        return;
    }

    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"localhost",                       // Server name
        5000,                               // HTTP port 
        0);

    if (hConnect == NULL) {
        WinHttpCloseHandle(hSession);
        LOG("WinHttpConnect failed: " + std::to_string(GetLastError()));
        return;
    }

    std::wstring wReplayFileName(replayFileName.begin(), replayFileName.end());  // Convert std::string to std::wstring
    std::wstring fullPath = L"/replays/" + wReplayFileName;                      // Concatenate the path

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,                               // Connection handle
        L"GET",                                 // HTTP method
        fullPath.c_str(),                               // Path (API endpoint)
        NULL,                                   // HTTP version (use default)
        WINHTTP_NO_REFERER,                     // Referrer (not needed)
        WINHTTP_NO_ADDITIONAL_HEADERS,          // Additional headers (not needed)
        0);                                     // No flags for secure connection (i.e., HTTP, not HTTPS)

    if (hRequest == NULL) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpOpenRequest failed: " + std::to_string(GetLastError()));
        return;
    }


    std::wstring headers = L"";

    BOOL result = WinHttpSendRequest(
        hRequest,
        headers.c_str(),                            // Headers
        -1,                                         // Length of the header
        NULL,                       // Request body
        0,                                // Body length
        0,                                // Total length
        0);                                         // Context (optional)

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpSendRequest failed: " + std::to_string(GetLastError()));
        return;
    }



    result = WinHttpReceiveResponse(hRequest, NULL);

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpReceiveResponse failed: " + std::to_string(GetLastError()));
        return;
    }

    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    char buffer[4096];
    std::stringstream responseStream;

    while (WinHttpReadData(hRequest, (LPVOID)buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {

        responseStream.write(buffer, bytesRead);
        totalBytesRead += bytesRead;

        if (totalBytesRead >= 100000) {
            LOG("Warning: Response data exceeds 100KB");
            return;
        }
    }
    const std::string assistant_id = responseStream.str();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    replay_assistant_id = assistant_id;
    LOG("Assistant created: "+assistant_id);

}

void DribbleCoach::queryReplayAssistant(const std::string& replayFileName,std::vector<std::string> params) {
    // if params size is zero, send a get request to localhost:3000/query/<guid>
    // if params size is > zero, join them all with a single space urlecoded (i think %20), and send to localhost:3000/query/<guid>/<the_joined_params>
    std::string query_text = joinWithUrlEncodedSpace(params);

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"DribbleCoach/1.0",                 // User agent
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,          // Access type
        WINHTTP_NO_PROXY_NAME,                      // Proxy name
        WINHTTP_NO_PROXY_BYPASS,                    // Proxy bypass
        0);

    if (hSession == NULL) {
        LOG("WinHttpOpen failed: " + std::to_string(GetLastError()));
        return;
    }

    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"localhost",                       // Server name
        5000,                               // HTTP port 
        0);

    if (hConnect == NULL) {
        WinHttpCloseHandle(hSession);
        LOG("WinHttpConnect failed: " + std::to_string(GetLastError()));
        return;
    }

    std::wstring wReplayFileName(replayFileName.begin(), replayFileName.end());  // Convert std::string to std::wstring
    std::wstring wReplayQuery(query_text.begin(), query_text.end());  // Convert std::string to std::wstring
    std::wstring fullPath = L"/query/" + wReplayFileName +L"/" + wReplayQuery;                      // Concatenate the path

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,                               // Connection handle
        L"GET",                                 // HTTP method
        fullPath.c_str(),                               // Path (API endpoint)
        NULL,                                   // HTTP version (use default)
        WINHTTP_NO_REFERER,                     // Referrer (not needed)
        WINHTTP_NO_ADDITIONAL_HEADERS,          // Additional headers (not needed)
        0);                                     // No flags for secure connection (i.e., HTTP, not HTTPS)

    if (hRequest == NULL) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpOpenRequest failed: " + std::to_string(GetLastError()));
        return;
    }


    std::wstring headers = L"";

    BOOL result = WinHttpSendRequest(
        hRequest,
        headers.c_str(),                            // Headers
        -1,                                         // Length of the header
        NULL,                       // Request body
        0,                                // Body length
        0,                                // Total length
        0);                                         // Context (optional)

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpSendRequest failed: " + std::to_string(GetLastError()));
        return;
    }



    result = WinHttpReceiveResponse(hRequest, NULL);

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpReceiveResponse failed: " + std::to_string(GetLastError()));
        return;
    }

    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    char buffer[4096];
    std::stringstream responseStream;

    while (WinHttpReadData(hRequest, (LPVOID)buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {

        responseStream.write(buffer, bytesRead);
        totalBytesRead += bytesRead;

        if (totalBytesRead >= 100000) {
            LOG("Warning: Response data exceeds 100KB");
            return;
        }
    }
    const std::string query_replay = responseStream.str();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    LOG(query_replay);
}


std::string DribbleCoach::TrimString(const std::string& input) {
    size_t startPos = input.find("\"text\":"); 
    if (startPos == std::string::npos) {
        return ""; 
    }

    startPos += 8; 

    size_t endPos = input.find("\"", startPos); 
    if (endPos == std::string::npos) {
        return "";
    }
    yonder_ai_text = input.substr(startPos, endPos - startPos);
    return yonder_ai_text;
}

std::string DribbleCoach::joinWithUrlEncodedSpace(const std::vector<std::string>& params) {
    if (params.empty()) {
        return "";  // Return empty string if the vector is empty
    }

    std::ostringstream oss;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i != 0) {
            oss << "%20";  // Add URL-encoded space between elements
        }
        oss << params[i];
    }

    return oss.str();
}
