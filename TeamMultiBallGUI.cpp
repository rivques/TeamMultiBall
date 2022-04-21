#include "pch.h"
#include "TeamMultiBall.h"


std::string TeamMultiBall::GetPluginName() {
	return "TeamMultiBall";
}

void TeamMultiBall::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> TeamMultiBall
void TeamMultiBall::RenderSettings() {
	ImGui::TextUnformatted("TeamMultiBall plugin settings");
}

/*
// Do ImGui rendering here
void TeamMultiBall::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string TeamMultiBall::GetMenuName()
{
	return "TeamMultiBall";
}

// Title to give the menu
std::string TeamMultiBall::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void TeamMultiBall::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool TeamMultiBall::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool TeamMultiBall::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void TeamMultiBall::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void TeamMultiBall::OnClose()
{
	isWindowOpen_ = false;
}
*/
