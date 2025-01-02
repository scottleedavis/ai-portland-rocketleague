#include "pch.h"
#include <fstream>
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <sstream>

#include "ReplayAssistant.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/GameEvent/TutorialWrapper.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"
#include "bakkesmod/wrappers/GameWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod/wrappers/ControllerWrapper.h" 
#include "bakkesmod/wrappers/ReplayServerWrapper.h"

BAKKESMOD_PLUGIN(ReplayAssistant, "ReplayAssistant", plugin_version, PLUGINTYPE_THREADED)


std::string prompt = "";
std::string thread_id = "";
std::string assistant_id = "";
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

const std::string replay_prepare = "replay_prepare";
const std::string replay_prompt = "replay_prompt";
const std::string replay_messages = "replay_messages";


void ReplayAssistant::LogWindow() {
    const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollRegion##", ImVec2(0, -footerHeightToReserve), false, 0))
    {
        ImGui::PushTextWrapPos();

        for (const std::string& item : m_ConsoleSystem)
            ImGui::TextUnformatted(item.c_str());

        ImGui::PopTextWrapPos();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }
}

void ReplayAssistant::InputBar() {

    ImGuiInputTextFlags inputTextFlags =
        ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackCompletion |
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways;

    bool reclaimFocus = false;

    ImGui::PushItemWidth(-ImGui::GetStyle().ItemSpacing.x * 7);
    if (ImGui::InputText("Send", &m_Buffer[0], m_Buffer.capacity(), inputTextFlags, ReplayAssistant::InputCallback, this))
    {
        if (!m_Buffer.empty())
        {
            std::string message = "user: " + m_Buffer;
            LOG(message);
            m_ConsoleSystem.push_back(message);

            this->OnPromptAssistant(this->split_string_on_spaces(m_Buffer));
            

        }

        reclaimFocus = true;

    }
    ImGui::PopItemWidth();

    ImGui::SetItemDefaultFocus();
    if (reclaimFocus)
        ImGui::SetKeyboardFocusHere(-1); // Focus on command line after clearing.
}

void ReplayAssistant::RenderWindow() {

    // https://github.com/ocornut/imgui/blob/e9743d85ddc0986c6babaa01fd9e641a47b282f3/imgui_demo.cpp#L6523-L6884
    // to do https://github.com/rmxbalanque/imgui-console/blob/master/src/imgui_console.cpp

    this->LogWindow();

    ImGui::Separator();

    this->InputBar();

}

void ReplayAssistant::RenderSettings() {
    LOG("settings render");
}

void ReplayAssistant::onLoad()
{
    _globalCvarManager = cvarManager;

    cvarManager->registerNotifier(replay_prepare, std::bind(&ReplayAssistant::OnCommand, this, std::placeholders::_1), "Starts/stops replay analysis", PERMISSION_REPLAY);
    cvarManager->registerNotifier(replay_prompt, std::bind(&ReplayAssistant::OnCommand, this, std::placeholders::_1), "Queries AI on current replay", PERMISSION_REPLAY);
    cvarManager->registerNotifier(replay_messages, std::bind(&ReplayAssistant::OnCommand, this, std::placeholders::_1), "Gets latest Assistant messages", PERMISSION_REPLAY);

    LOG("Loaded.");
}

void ReplayAssistant::OnReplayAssistant()
{
    ReplayServerWrapper gew = gameWrapper->GetGameEventAsReplay();
    LOG("Preparing replay...");
    replay_thread = std::thread(std::bind(&ReplayAssistant::PrepareReplay, this, gew.GetReplay().GetId().ToString()));
    replay_thread.detach();
}

void ReplayAssistant::OnPromptAssistant(std::vector<std::string> params)
{
    ReplayServerWrapper gew = gameWrapper->GetGameEventAsReplay();
    LOG("Prompting replay...");
    prompt_thread = std::thread(std::bind(&ReplayAssistant::PromptReplayAssistant, this, params));
    prompt_thread.detach();
}

void ReplayAssistant::OnCommand(std::vector<std::string> params)
{
    std::string command = params.at(0);
        
    if (command.compare(replay_prepare) == 0) {
        this->OnReplayAssistant();
    } else if (command.compare(replay_prompt) == 0) {
        params.erase(params.begin());
        this->OnPromptAssistant(params);
    }
    else if (command.compare(replay_messages) == 0) {
        this->checkMessages();
    }
}

void ReplayAssistant::checkMessages()
{

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"ReplayAssistant/1.0",                 // User agent
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

    std::wstring wThreadId(thread_id.begin(), thread_id.end());             // Convert std::string to std::wstring
    std::wstring fullPath = L"/messages/" + wThreadId;                      // Concatenate the path

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
    const std::string latest_thread_messages = responseStream.str();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    m_ConsoleSystem.clear();
    m_ConsoleSystem = this->split_string_on_comma(latest_thread_messages);

    LOG("Assistant thread messages...");// : \n" + latest_thread_messages + "\n");

}

void ReplayAssistant::PrepareReplay(const std::string& replayFileName)
{

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"ReplayAssistant/1.0",                 // User agent
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
    const std::string assistant_thread_id = responseStream.str();
    std::string assistant, thread;

    size_t delimiter_pos = assistant_thread_id.find('|');

    if (delimiter_pos != std::string::npos) {
        assistant = assistant_thread_id.substr(0, delimiter_pos);
        thread = assistant_thread_id.substr(delimiter_pos + 1);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    thread_id = thread;
    assistant_id = assistant;
    LOG("Assistant thread created: "+ assistant_thread_id);

}

void ReplayAssistant::PromptReplayAssistant(std::vector<std::string> params) {

    std::string query_text = joinWithUrlEncodedSpace(params);

    LOG("Waiting....");
    HINTERNET hSession = WinHttpOpen(
        L"ReplayAssistant/1.0",                 // User agent
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

    std::wstring wReplayAssistantId(assistant_id.begin(), assistant_id.end());
    std::wstring wThreadId(thread_id.begin(), thread_id.end());
    std::wstring wReplayQuery(query_text.begin(), query_text.end()); 

    std::wstring fullPath = L"/query/" + wReplayAssistantId + L"/" + wThreadId + L"/" + wReplayQuery;

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
        NULL,                                       // Request body
        0,                                          // Body length
        0,                                          // Total length
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
    const std::string prompt_replay = responseStream.str();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

//

    LOG("Replay prompted.");
}



std::vector<std::string> ReplayAssistant::split_string_on_spaces(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string word;

    while (ss >> word) {
        result.push_back(word);
    }

    return result;
}

std::string ReplayAssistant::joinWithUrlEncodedSpace(const std::vector<std::string>& params) {
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

std::vector<std::string> ReplayAssistant::split_string_on_comma(const std::string& input) {
    std::vector<std::string> result;

    // Remove the square brackets at the start and end of the input string
    std::string cleaned_input = input.substr(1, input.size() - 2);

    size_t pos = 0;
    std::string token;

    // Split by ", " delimiter
    while ((pos = cleaned_input.find("\", \"")) != std::string::npos) {
        token = cleaned_input.substr(0, pos); // Extract the part before ", "
        result.push_back(token); // Store in the result vector
        cleaned_input.erase(0, pos + 4); // Remove the processed part
    }

    // Add the last part (if any)
    if (!cleaned_input.empty()) {
        result.push_back(cleaned_input);
    }

    return result;
}