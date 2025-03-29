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

#include "framework/FileSystem.h"

#include "PathTreeCtrl.h"
#include "DeclNew.h"
#include "DeclBrowser.h"

namespace ImGuiTools {

/*
================
DeclNew::DeclNew
================
*/
DeclNew::DeclNew()
	: typeList()
	, typeListSel(-1)
	, nameEdit()
	, fileEdit()
	, errorText()
	, declTree(NULL)
	, defaultType()
	, defaultName()
	, newDecl(NULL)
	, state(DONE)
	, selectDlg( DECL_AF, "af", ".af", "AFs" )
{
}

/*
================
DeclNew::InitTypeList
================
*/
void DeclNew::InitTypeList( void ) {
	int i;

	typeList.Clear();
	for ( i = 0; i < declManager->GetNumDeclTypes(); i++ ) {
		typeList.Append( declManager->GetDeclNameFromType( (declType_t)i ) );
	}
	typeListSel = -1;
}

/*
================
DeclNew::Reset
================
*/
void DeclNew::Reset()  {
	InitTypeList();

	typeListSel = typeList.FindIndex( defaultType );
	nameEdit = defaultName;
	fileEdit = defaultFile;
	newDecl = NULL;
	state = DONE;
}

void DeclNew::Start() {
	Reset();

	state = NAME;

	ImGui::OpenPopup("New Declaration");
}

bool DeclNew::Draw() {
	if ( state == DONE ) {
		return false;
	}

	bool accepted = false;
	bool canceled = false;

	if ( ImGui::BeginPopupModal( "New Declaration", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "%s", errorText.c_str() );

		const char *previewValue = typeListSel == -1 ? NULL : typeList[typeListSel].c_str();

		if ( ImGui::BeginCombo( "Type##typeListSelect", previewValue ) ) {
			ImGui::SetItemTooltip( "select the declaration type to create" );

			for ( int i = 0; i < typeList.Num(); i++ ) {
				bool selected = ( i == typeListSel );

				ImGui::PushID( i );
				if ( ImGui::Selectable( typeList[i].c_str(), selected ) ) {
					typeListSel = i;
				}
				if ( selected ) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}

		if ( ImGui::InputTextStr( "Name", &nameEdit ) ) {

		}
		ImGui::SetItemTooltip( "enter a name for the new declaration" );
		if ( ImGui::InputTextStr( "File", &fileEdit ) ) {

		}
		ImGui::SetItemTooltip( "enter the name of the file to add the declaration to" );
		ImGui::SameLine();
		if ( ImGui::Button( "..." ) ) {
			//selectDlg.Start();
			OnBnClickedFile();
		}
		ImGui::SetItemTooltip( "select existing file to add the declaration to" );

		if ( selectDlg.Draw() ) {
			// accepted
			//fileEdit = fileSystem->OSPathToRelativePath( dlgFile.m_ofn.lpstrFile );
			fileEdit = selectDlg.GetDecl()->GetFileName();
		}

		if ( ImGui::Button( "OK" ) ) {
			OnBnClickedOk();
			if ( newDecl ) {
				accepted = true;
				state = DONE;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SetItemTooltip( "create new declaration" );
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			accepted = false;
			state = DONE;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemTooltip( "cancel" );

		ImGui::EndPopup();
	}

	return accepted;
}

// DeclNew message handlers

/*
================
DeclNew::OnBnClickedFile
================
*/
void DeclNew::OnBnClickedFile() {
	idStr typeName, folder, ext;
	idStr path;

	errorText.Clear();

	if ( typeListSel == -1 ) {
		errorText = "Select a declaration type first.";
		return;
	}

	declType_t type = declManager->GetDeclTypeFromName( typeName );
	if ( type >= declManager->GetNumDeclTypes() ) {
		errorText = "Unknown declaration type.";
		return;
	}

	switch( type ) {
		// FIXME: SteelStorm2 has a _v1 suffix for materials, def and fx - why?
		case DECL_TABLE:		folder = "materials";	ext = ".mtr";					break;
		case DECL_MATERIAL:		folder = "materials";	ext = ".mtr";					break;
		case DECL_SKIN:			folder = "skins";		ext = ".skin";					break;
		case DECL_SOUND:		folder = "sound";		ext = ".sndshd";				break;
		case DECL_ENTITYDEF:	folder = "def";			ext = ".def|.decl";				break;
		case DECL_MODELDEF:		folder = "def";			ext = ".def";					break;
		case DECL_FX:			folder = "fx";			ext = ".fx";					break;
		case DECL_PARTICLE:		folder = "particles";	ext = ".prt";					break;
		case DECL_AF:			folder = "af";			ext = ".af";					break;
		default:				folder = "def";			ext = ".decl";					break;
	}

	path = fileSystem->RelativePathToOSPath( folder );
	path += "\\*";

	selectDlg.Start();
}

/*
================
DeclNew::OnBnClickedOk
================
*/
void DeclNew::OnBnClickedOk() {
	idStr typeName, declName, fileName;

	errorText.Clear();

	if ( !declTree ) {
		errorText = "No declaration tree available.";
		return;
	}

	if ( typeListSel == -1 ) {
		errorText = "No declaration type selected.";
		return;
	}
	typeName = typeList[typeListSel];

	declName = nameEdit;
	if ( declName.Length() == 0 ) {
		errorText = "No declaration name specified.";
		return;
	}

	fileName = fileEdit;
	if ( fileName.Length() == 0 ) {
		errorText = "No file name specified.";
		return;
	}

	if ( declTree->FindItem( idStr( typeName + "/" + declName ) ) ) {
		errorText = "Declaration already exists.";
		return;
	}

	declType_t type = declManager->GetDeclTypeFromName( typeName );
	if ( type >= declManager->GetNumDeclTypes() ) {
		errorText = "Unknown declaration type.";
		return;
	}

	newDecl = declManager->CreateNewDecl( type, declName, fileName );
}

/*
================
DeclNew::OnBnClickedCancel
================
*/
void DeclNew::OnBnClickedCancel() {

}

}
