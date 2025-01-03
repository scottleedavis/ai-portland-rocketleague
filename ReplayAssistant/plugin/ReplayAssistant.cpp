#include "pch.h"
#include <fstream>
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <functional>

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

const std::string replay_prepare  = "replay_prepare";
const std::string replay_prompt   = "replay_prompt";
const std::string replay_messages = "replay_messages";


void ReplayAssistant::RenderSettings() {
    LOG("settings render");
}

void ReplayAssistant::onLoad()
{
    _globalCvarManager = cvarManager;

    cvarManager->registerNotifier(replay_prepare, std::bind(&ReplayAssistant::OnCommand, this, std::placeholders::_1), "Starts/stops replay analysis", PERMISSION_REPLAY);
    cvarManager->registerNotifier(replay_prompt, std::bind(&ReplayAssistant::OnCommand, this, std::placeholders::_1), "Queries AI on current replay", PERMISSION_REPLAY);
    cvarManager->registerNotifier(replay_messages, std::bind(&ReplayAssistant::OnCommand, this, std::placeholders::_1), "Gets latest Assistant messages", PERMISSION_REPLAY);
    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Countdown.BeginState", std::bind(&ReplayAssistant::onReplayLoaded, this));

    LOG("Loaded.");
}

void ReplayAssistant::onReplayLoaded() {

    if (gameWrapper->GetCurrentGameState().GetSecondsElapsed() < 1.0f) {
        m_ConsoleSystem.clear();
        prompt = "";
        thread_id = "";
        assistant_id = "";
        m_Buffer = std::string(1000, '\0');
    }
    if (gameWrapper->IsInReplay())
        this->Render();
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

    if (!gameWrapper->IsInReplay())
        return;

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

    LOG("Assistant thread messages...");
    if (!gameWrapper->IsInReplay())
        return;
    // Delay for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    replay_thread = std::thread(std::bind(&ReplayAssistant::checkMessages, this));
    replay_thread.detach();
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
    gameWrapper->Toast("Assistant created!", "You can now interact.", "cool", 5.0, ToastType_Info);

    if (!gameWrapper->IsInReplay())
        return;

    replay_thread = std::thread(std::bind(&ReplayAssistant::checkMessages, this));
    replay_thread.detach();

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

    LOG("Replay prompted.");

    m_Buffer = std::string(1000, '\0');
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
    std::string cleaned_input = input.substr(2, input.size());
    cleaned_input.erase(cleaned_input.length() - 3);

    std::string cleaned_input2;
    size_t pos = 0;

    // Loop through and replace literal \n (backslash + n) with "\", \""
    for (size_t i = 0; i < cleaned_input.size(); ++i) {
        if (cleaned_input[i] == '\\' && i + 1 < cleaned_input.size() && cleaned_input[i + 1] == 'n') {
            cleaned_input2 += "\", \"";  // Replace literal \n with ", "
            ++i;  // Skip over the 'n' in "\n"
        }
        else {
            cleaned_input2 += cleaned_input[i];  // Keep the character as is
        }
    }


    // Now, replace "user:" with "user:", " and "assistant:" with "assistant:", "
    std::string cleaned_input3;
    for (size_t i = 0; i < cleaned_input2.size(); ++i) {
        if (i + 4 < cleaned_input2.size() && cleaned_input2.substr(i, 5) == "user:") {
            cleaned_input3 += "user:\", \"";  // Replace "user:" with "user:", "
            i += 4; // Skip over "user:"
        }
        else if (i + 9 < cleaned_input2.size() && cleaned_input2.substr(i, 10) == "assistant:") {
            cleaned_input3 += "assistant:\", \"";  // Replace "assistant:" with "assistant:", "
            i += 9; // Skip over "assistant:"
        }
        else {
            cleaned_input3 += cleaned_input2[i];  // Keep the character as is
        }
    }

    // Now split the cleaned input string by the delimiter "\", \""
    std::string token;
    while ((pos = cleaned_input3.find("\", \"")) != std::string::npos) {
        token = cleaned_input3.substr(0, pos);  // Extract the part before "\", \""
        result.push_back(token);  // Store in the result vector
        cleaned_input3.erase(0, pos + 4);  // Remove the processed part (", \"")
    }

    if (!cleaned_input3.empty()) {
        result.push_back(cleaned_input3);
    }

    return result;
}

void MarkdownFormatCallback(const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_)
{

    ImGui::defaultMarkdownFormatCallback(markdownFormatInfo_, start_);

    switch (markdownFormatInfo_.type)
    {
    case ImGui::MarkdownFormatType::HEADING:
    {
        if (markdownFormatInfo_.level == 2)
        {
            if (start_)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            }
            else
            {
                ImGui::PopStyleColor();
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }
}


static ImGui::MarkdownConfig mdConfig;

static ImFont* H1 = NULL;
static ImFont* H2 = NULL;
static ImFont* H3 = NULL;

void Markdown(const std::string& markdown_)
{
    //mdConfig.linkCallback = LinkCallback;
    mdConfig.tooltipCallback = NULL;
    //mdConfig.imageCallback = ImageCallback;
    //mdConfig.linkIcon = ICON_FA_LINK;
    mdConfig.headingFormats[0] = { H1, true };
    mdConfig.headingFormats[1] = { H2, true };
    mdConfig.headingFormats[2] = { H3, false };
    mdConfig.userData = NULL;
    mdConfig.formatCallback = MarkdownFormatCallback;
    ImGui::Markdown(markdown_.c_str(), markdown_.length(), mdConfig);
}

void ReplayAssistant::LogWindow() {
    const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollRegion##", ImVec2(0, -footerHeightToReserve), false, 0))
    {
        ImGui::PushTextWrapPos();

        std::stringstream ss;
        for (const std::string& item : m_ConsoleSystem)
            ss << item << "\n";
        std::string joined_string = ss.str();
        Markdown(joined_string.c_str());

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
    if (ImGui::InputText("", &m_Buffer[0], m_Buffer.capacity(), inputTextFlags, ReplayAssistant::InputCallback, this))
    {
        if (!m_Buffer.empty())
        {
            std::string message = "user: " + m_Buffer;
            LOG(message);
            m_ConsoleSystem.push_back(message);

            this->OnPromptAssistant(this->split_string_on_spaces(m_Buffer));
            m_Buffer = std::string(1000, '\0');

        }

        reclaimFocus = true;

    }
    ImGui::PopItemWidth();

    ImGui::SetItemDefaultFocus();
    if (reclaimFocus)
        ImGui::SetKeyboardFocusHere(-1); 
}

void ReplayAssistant::RenderWindow() {

    if (ImGui::Button("Activate Assistant"))
    {
        m_ConsoleSystem.clear();
        prompt = "";
        thread_id = "";
        assistant_id = "";
        this->OnReplayAssistant();
        gameWrapper->Toast("Preparing Assistant...", "Lets go", "cool", 5.0, ToastType_Info);
    }
    //ImGui::SameLine();
    //if (ImGui::Button("Get Messages"))
    //{
    //    replay_thread = std::thread(std::bind(&ReplayAssistant::checkMessages, this));
    //    replay_thread.detach();
    //}
    ImGui::Separator();


    this->LogWindow();

    ImGui::Separator();

    this->InputBar();


}