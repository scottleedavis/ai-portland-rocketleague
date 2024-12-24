#include "pch.h"
#include <fstream>
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <sstream>

#include "AICoachBakkesPlugin.h"
#include "bakkesmod/wrappers/GameEvent/TutorialWrapper.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/GameWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod/wrappers/ControllerWrapper.h" 

BAKKESMOD_PLUGIN(AICoachBakkesPlugin, "AI Dribble Coach", plugin_version, PLUGINTYPE_THREADED)

bool isDribble = false;
std::string query = "";
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
const std::string le_prompt = "checking out this replay of a car and ball practicing dribbling, until it touches ground. Give a brief one sentence recommendation to the player saying what was done well and/or what could improve.  Make your response no longer than nine words.";
const std::string data_row_header = "time, car_x, car_y, car_z, ball_x, ball_y, ball_z";
std::string yonder_ai_text = "";

void AICoachBakkesPlugin::onLoad()
{
    _globalCvarManager = cvarManager;
    cvarManager->registerCvar("anthropic_api_key", "YOUR_SECRET_KEY", "API key for the Anthropic AI service", true);

    cvarManager->registerNotifier("ai_dribble", std::bind(&AICoachBakkesPlugin::OnCommand, this, std::placeholders::_1), "Starts/stops dribble analysis", PERMISSION_FREEPLAY);
    gameWrapper->HookEvent("Function TAGame.Ball_TA.OnRigidBodyCollision", std::bind(&AICoachBakkesPlugin::OnDroppedBall, this, std::placeholders::_1));
    resetKey = this->gameWrapper->GetFNameIndexByString("XboxTypeS_DPad_Up");
    gameWrapper->RegisterDrawable(std::bind(&AICoachBakkesPlugin::Render, this, std::placeholders::_1));

    LOG("Loaded.");
}

void AICoachBakkesPlugin::OnDroppedBall(std::string eventName)
{
    if (isDribble) {
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
            
            isDribble = false;
            LOG("Querying AI...");

            std::string output = "";
            for (unsigned int i = 0; i < playbackData.size(); i++) output += playbackData.at(i) + " ";

            server_thread = std::thread(std::bind(&AICoachBakkesPlugin::AskAnthropic, this, output));
            server_thread.detach();

            playbackData.clear();
            playbackData.push_back(le_prompt);
            playbackData.push_back(data_row_header);

        }      
    }
    this->OnRecordTick();
}

void AICoachBakkesPlugin::OnRecordTick()
{
    if (!isDribble)
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

void AICoachBakkesPlugin::OnCommand(std::vector<std::string> params)
{
    std::string command = params.at(0);
    if (command.compare("ai_dribble") == 0) {
        if (!gameWrapper->IsInFreeplay())
            return;
        ServerWrapper tutorial = gameWrapper->GetGameEventAsServer();

        if (tutorial.GetGameBalls().Count() == 0)
            return;

        BallWrapper ball = tutorial.GetGameBalls().Get(0);
        CarWrapper car = tutorial.GetGameCar();
        if (ball.IsNull() || car.IsNull())
            return;
        Vector playerVelocity = car.GetVelocity();
        Vector addToBall = Vector(playerVelocity.X, playerVelocity.Y, 170);

        addToBall.X = max(min(20.0f, addToBall.X), -20.0f);
        addToBall.Y = max(min(30.0f, addToBall.Y), -30.0f);

        ball.SetLocation(car.GetLocation() + addToBall);
        ball.SetVelocity(playerVelocity);
        isDribble = true;
        yonder_ai_text = "";

        playbackData.push_back(le_prompt);
        playbackData.push_back(data_row_header);
        
        this->OnRecordTick();

    }
}

void AICoachBakkesPlugin::Render(CanvasWrapper canvas)
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
        isDribble = true;
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

void AICoachBakkesPlugin::AskAnthropic(std::string prompt) {
    const std::string url = "https://api.anthropic.com/v1/messages";
    const std::string data = "{\"model\": \"claude-3-5-sonnet-20241022\", \"max_tokens\": 8192,\"messages\": [{\"role\": \"user\", \"content\": \"" + prompt + "\"}]}";
    std::string secret = cvarManager->getCvar("anthropic_api_key").getStringValue();

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"AICoachBakkesPlugin/1.0",                 // User agent
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

std::string AICoachBakkesPlugin::TrimString(const std::string& input) {
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

