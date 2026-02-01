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

#include "../util/ImGui_IdWidgets.h"

#include "FindDialog.h"

#include "MEMainFrame.h"

namespace ImGuiTools {

/**
* Constructor for FindDialog.
*/
FindDialog::FindDialog(MEMainFrame *pParent)
    : visible(false)
    , focus(false)
    , message(0)
{
    registry.Init("MaterialEditor_Find");
	parent = pParent;
}

/**
* Destructor for FindDialog.
*/
FindDialog::~FindDialog() {
}

/**
* Creates and instance of the find dialog.
*/
void FindDialog::Start() {
    visible = true;
    focus = true;
    message = 0;
    LoadFindSettings();
}

bool FindDialog::Draw( const ImVec2 &pos, const ImVec2 &size ) {
    bool accepted = false;

    if (!visible) {
        return accepted;
    }

    ImVec2 oldCursorPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos( pos );
    // FIXME: find a better color
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(50, 50, 50, 255));
    bool ret = ImGui::BeginChild( "Find", size, ImGuiChildFlags_Borders | ImGuiChildFlags_FrameStyle );
    if ( ret ) {

        if ( ImGui::InputTextStr( "Find What", &searchData.searchText ) ) {
            message = 0;
        }
		if ( focus ) {
			ImGui::SetKeyboardFocusHere( -1 );
			focus = false;
		}

        bool nameOnly = searchData.nameOnly;
        if ( ImGui::Checkbox( "Name Only", &nameOnly ) ) {
            searchData.nameOnly = nameOnly;
            message = 0;
        }

        ImGui::Text( "Look In:" );
        if ( ImGui::RadioButton( "Current Material", &searchData.searchScope, 0 ) ) {
            message = 0;
        }
        ImGui::SameLine();
        if ( ImGui::RadioButton( "Open Materials", &searchData.searchScope, 1 ) ) {
            message = 0;
        }
        ImGui::SameLine();
        if ( ImGui::RadioButton( "All Materials", &searchData.searchScope, 2 ) ) {
            message = 0;
        }

        if ( ImGui::Button( "Find Next" ) ) {
            OnBnClickedFindNext();
        }
        ImGui::SameLine();
        if ( ImGui::Button( "Cancel" ) ) {
            OnCancel();
            visible = false;
            message = 0;
            accepted = true;
        }

        if ( message ) {
            ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "Unable to find '%s'", searchData.searchText.c_str() );
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::SetCursorPos( oldCursorPos );

    return accepted;
}

void FindDialog::UnableToFind() {
    message = 1;
}

/**
* Triggers a search based on the parameters in the dialog.
*/
void FindDialog::OnBnClickedFindNext() {

	searchData.searched = false;
	parent->FindNext(&searchData);
}

/**
* Saves the search parameters and closes the find dialog.
*/
void FindDialog::OnCancel()
{
	SaveFindSettings();

	parent->CloseFind();
}

/**
* Loads the search parameters from the registry and makes sure the controls are properly
* initialized.
*/
void FindDialog::LoadFindSettings() {
	registry.Load();

	searchData.searchText = registry.GetString("searchText");
	searchData.nameOnly = (int)registry.GetFloat("nameOnly");
	searchData.searchScope = (int)registry.GetFloat("searchScope");

	//registry.GetWindowPlacement("findDialog", GetSafeHwnd());
}

/**
* Saves the search parameters to the registry.
*/
void FindDialog::SaveFindSettings() {

	registry.SetString("searchText", searchData.searchText);
	registry.SetFloat("nameOnly", searchData.nameOnly);
	registry.SetFloat("searchScope", searchData.searchScope);

	//registry.SetWindowPlacement("findDialog", GetSafeHwnd());

	registry.Save();
}

} // namespace ImGuiTools
