#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class AICoachBakkesPlugin: public BakkesMod::Plugin::BakkesModPlugin
	//,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	std::thread server_thread;
	std::vector<std::string> playbackData;
	std::string TrimString(const std::string& input);


public:
	void onLoad() override;
	void OnDroppedBall(std::string eventName);
	void OnCommand(std::vector<std::string> params);
	void OnRecordTick();
	void AskAnthropic(std::string prompt);

};
