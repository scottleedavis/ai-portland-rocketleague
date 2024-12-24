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

BAKKESMOD_PLUGIN(AICoachBakkesPlugin, "AI Dribble Coach", plugin_version, PLUGINTYPE_THREADED)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
bool isDribble = false;
std::string query = "";
const std::string le_prompt = "here is a replay of a car and a ball practicing ground dribbling before dropping in freeplay. Give a brief one sentence recommendation to the player.";
const std::string data_row_header = "time, car_x, car_y, car_z, ball_x, ball_y, ball_z";

void AICoachBakkesPlugin::onLoad()
{
    _globalCvarManager = cvarManager;

    cvarManager->registerCvar("anthropic_api_key", "", "API key for the Anthropic AI service", true);

    // You can also print the current value of the CVar
    std::string apiKey = cvarManager->getCvar("anthropic_api_key").getStringValue();
    LOG("Loading. ");

    cvarManager->registerNotifier("ai_dribble", std::bind(&AICoachBakkesPlugin::OnCommand, this, std::placeholders::_1), "Starts/stops dribble analysis", PERMISSION_FREEPLAY);
    gameWrapper->HookEvent("Function TAGame.Ball_TA.OnRigidBodyCollision", std::bind(&AICoachBakkesPlugin::OnDroppedBall, this, std::placeholders::_1));
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

        if (ballLocation.Z <= 94) {
            // Ball is near or on the ground
            isDribble = false;
            LOG("Querying AI...");

            std::string output = "";
            for (unsigned int i = 0; i < playbackData.size(); i++)
            {
                output += playbackData.at(i) + " ";
            }
            server_thread = std::thread(std::bind(&AICoachBakkesPlugin::AskAnthropic, this, output));
            server_thread.detach();


            playbackData.clear();
            playbackData.push_back(le_prompt);
            playbackData.push_back(data_row_header);

        }      
    }
    this->OnRecordTick("do");
}

void AICoachBakkesPlugin::OnRecordTick(std::string eventName)
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

        playbackData.push_back(le_prompt);
        playbackData.push_back(data_row_header);
        
        this->OnRecordTick("do");

    }
}

std::string AICoachBakkesPlugin::AskAnthropic(std::string prompt) {
    const std::string url = "https://api.anthropic.com/v1/messages";
    const std::string data = "{\"model\": \"claude-3-5-sonnet-20241022\", \"max_tokens\": 8192,\"messages\": [{\"role\": \"user\", \"content\": \"" + prompt + "\"}]}";
    const std::string apiKey = cvarManager->getCvar("anthropic_api_key").getStringValue();
    LOG("Waiting....");

    // Initialize WinHTTP session handle
    HINTERNET hSession = WinHttpOpen(
        L"A WinHTTP Example Program/1.0",          // User agent
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,        // Access type
        WINHTTP_NO_PROXY_NAME,                    // Proxy name
        WINHTTP_NO_PROXY_BYPASS,                  // Proxy bypass
        0);

    if (hSession == NULL) {
        LOG("WinHttpOpen failed: " + std::to_string(GetLastError()));
        return "error";
    }

    // Open a secure connection to the server on port 443
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"api.anthropic.com",                     // Server name
        INTERNET_DEFAULT_HTTPS_PORT,              // HTTPS port (443)
        0);

    if (hConnect == NULL) {
        WinHttpCloseHandle(hSession);
        LOG("WinHttpConnect failed: " + std::to_string(GetLastError()));
        return "error";
    }

    // Create the HTTP request for POST method
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,                // Connection handle
        L"POST",                 // HTTP method
        L"/v1/messages",         // Path (API endpoint)
        NULL,                    // HTTP version (use default)
        WINHTTP_NO_REFERER,      // Referrer (not needed)
        WINHTTP_NO_ADDITIONAL_HEADERS, // Additional headers (not needed)
        WINHTTP_FLAG_SECURE);    // Enable secure connection (HTTPS)

    if (hRequest == NULL) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpOpenRequest failed: " + std::to_string(GetLastError()));
        return "error";
    }

    // Set the headers (Content-Type and Authorization with API Key)
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"anthropic-version: 2023-06-01\r\n";
    headers += L"x-api-key: " + std::wstring(apiKey.begin(), apiKey.end()) + L"\r\n";

    // Send the HTTP POST request
    BOOL result = WinHttpSendRequest(
        hRequest,
        headers.c_str(),                          // Headers
        -1,                                       // Length of the header
        (LPVOID)data.c_str(),                     // Request body
        data.size(),                              // Body length
        data.size(),                              // Total length
        0);                                       // Context (optional)

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpSendRequest failed: " + std::to_string(GetLastError()));
        return "error";
    }

    // Receive the response from the server
    result = WinHttpReceiveResponse(hRequest, NULL);

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG("WinHttpReceiveResponse failed: " + std::to_string(GetLastError()));
        return "error";
    }

    // Read the response data
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    char buffer[4096];  // Fixed size buffer
    std::stringstream responseStream;

    // Read the response data in chunks
    while (WinHttpReadData(hRequest, (LPVOID)buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        // Debug: Log the size of the data read
       //std::cout << "Bytes read: " << bytesRead << std::endl;

        // Write the buffer content into the response stream
        responseStream.write(buffer, bytesRead);
        totalBytesRead += bytesRead;

        // Optional: Check if buffer has been filled to prevent overflows
        if (totalBytesRead >= 100000) {
            LOG("Warning: Response data exceeds 100KB");
            return "error";
        }
    }

    // Convert response stream to string
    std::string response = responseStream.str();

    // Clean up
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Return the response
    LOG(this->TrimString(response));
    return response;
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

    return input.substr(startPos, endPos - startPos); 
}

