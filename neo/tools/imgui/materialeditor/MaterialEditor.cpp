
/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/sys_imgui.h"
#include "../util/RegistryOptions.h"
#include "MEMainFrame.h"

namespace ImGuiTools {

MEMainFrame* meMainFrame = NULL;

/**
* Initializes the material editor tool.
*/
void MaterialEditorInit( void ) {
	com_editors = EDITOR_MATERIAL;

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	meMainFrame = new MEMainFrame;

	meMainFrame->OnCreateClient();

	impl::SetReleaseToolMouse( true );

	D3::ImGuiHooks::OpenWindow(D3::ImGuiHooks::D3_ImGuiWin_MaterialEditor);
}

void MaterialEditorSetActiveWindow( MaterialEditorWindow_t window ) {
	if ( !meMainFrame ) {
		return;
	}

	meMainFrame->SetActiveWindow( window );
}

/**
* Called every frame by the doom engine to allow the material editor to process messages.
*/
void MaterialEditorDraw( void ) {
	bool showTool;

	if ( !meMainFrame ) {
		return;
	}

	showTool = meMainFrame != NULL;

	if ( ImGui::Begin( "Material Editor", &showTool, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar ) ) {
		impl::SetReleaseToolMouse( true );

		meMainFrame->Draw();
	}
	ImGui::End();

	if ( meMainFrame && !showTool )
	{
		MaterialEditorShutdown();
		impl::SetReleaseToolMouse( false );
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_MaterialEditor );
	}
}

/**
* Called by the doom engine when the material editor needs to be destroyed.
*/
void MaterialEditorShutdown( void ) {

	meMainFrame->OnDestroy();

	delete meMainFrame;

	meMainFrame = NULL;
	com_editors = 0;
}

/**
* Allows the doom engine to reflect console output to the material editors console.
*/
void MaterialEditorPrintConsole( const char *msg ) {
	// meMainFrame can be null when starting immediately from commandline.
	if(meMainFrame && com_editors & EDITOR_MATERIAL)
		meMainFrame->PrintConsoleMessage(msg);
}

}
