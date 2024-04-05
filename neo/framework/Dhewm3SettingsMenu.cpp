
#include "Common.h"

#include "sys/sys_imgui.h"

#ifndef IMGUI_DISABLE

static bool dhewm3MenuOpen = false;

// called from D3::ImGuiHooks::NewFrame() (if this window is enabled)
void Com_DrawDhewm3SettingsMenu()
{
	bool showSettingsWindow = true;
	ImGui::Begin("dhewm3 Settings", &showSettingsWindow);

	if (ImGui::Button("Show ImGui Demo")) {
		D3::ImGuiHooks::OpenWindow( D3::ImGuiHooks::D3_ImGuiWin_Demo );
	}

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("SettingsTabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Avocado"))
		{
			ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Broccoli"))
		{
			ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Cucumber"))
		{
			ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}




	ImGui::End();
	if(!showSettingsWindow) {
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_Settings );
		dhewm3MenuOpen = false;
	}
}

void Com_Dhewm3Settings_f( const idCmdArgs &args )
{
	if ( !dhewm3MenuOpen ) {
		// TODO: if in SP game, pause

		D3::ImGuiHooks::OpenWindow( D3::ImGuiHooks::D3_ImGuiWin_Settings );

		dhewm3MenuOpen = true;

	} else {
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_Settings );

		// TODO: if in SP game, unpause
		dhewm3MenuOpen = false;
	}
}

#else // IMGUI_DISABLE - just a stub function

void Com_Dhewm3Settings_f( const idCmdArgs &args ) {}

#endif
