#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "RenderingTools/RenderingTools.h"
#include "NetcodeManager/NetcodeManager.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class TeamMultiBall: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow/*, public BakkesMod::Plugin::PluginWindow*/
{

	//std::shared_ptr<bool> enabled;
	std::shared_ptr<NetcodeManager> Netcode;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();
	void reloadPlugin();

	const int backWall = 5040;
	void bounceBall(BallWrapper ball);
	void onTick();

	bool ballsValid();
	void createAndIdentifyBalls(bool allowRecursion);
	uintptr_t blueBallAddr = NULL;
	uintptr_t orangeBallAddr = NULL;
	bool clientIsWaitingOnResize = false;
	bool hostIsWaitingOnResize = false;

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;

	void renderGame(CanvasWrapper canvas);

	void OnMessageReceived(const std::string& Message, PriWrapper Sender);
	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "TeamMultiBall";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
};

