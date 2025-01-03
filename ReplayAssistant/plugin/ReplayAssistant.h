#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "imgui_markdown.h"  

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class ReplayAssistant: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	std::thread replay_thread;
	std::thread prompt_thread;

	int groundDribbleResetKey;
	int airDribbleResetKey1;
	int airDribbleResetKey2;
	int airDribbleResetKey3;

	std::vector<std::string> playbackData;
	std::string joinWithUrlEncodedSpace(const std::vector<std::string>& params);
	std::vector<std::string> split_string_on_spaces(const std::string& input);
	std::vector<std::string> split_string_on_comma(const std::string& input);
	std::string m_Buffer = std::string(1000, '\0');  // Initialize with 1000 characters
	std::vector<std::string> m_ConsoleSystem;
	void onReplayLoaded();

public:
	void checkMessages();
	void onLoad() override;

	void InputBar();
	void LogWindow();
	void OnCommand(std::vector<std::string> params);
    void OnReplayAssistant();
	void OnPromptAssistant(std::vector<std::string> params);
    void PrepareReplay(const std::string& replayFilePath);
	void PromptReplayAssistant(std::vector<std::string> params);
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void RenderWindow() override; // Uncomment if you want to render your own plugin window
	static int InputCallback(ImGuiInputTextCallbackData* data)
	{
		//ReplayAssistant* coach = (ReplayAssistant*)data->UserData;
		//return coach->InputCallback(data); // Call the actual member function
		return 0; // Returning 0 means everything went fine, otherwise you can return a non-zero value to abort the input.
	}
};
