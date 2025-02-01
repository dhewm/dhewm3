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

#include "PDAEditor.h"

#include "sys/sys_imgui.h"
#include "../util/ImGui_IdWidgets.h"

#include "idlib/math/Vector.h"

#include "framework/FileSystem.h"
#include "framework/Game.h"
#include "framework/DeclPDA.h"

namespace ImGuiTools {

/////////////////////////////////////////////////////////////////////////////
// PDAEditor

PDAEditor& PDAEditor::Instance()
{
	static PDAEditor instance;
	return instance;
}

PDAEditor::PDAEditor()
{
	isShown = false;
}

void PDAEditor::Reset() {
	// Indicate the PDA dialog is opened
	com_editors |= EDITOR_PDA;
	
	pdaListSel = -1;
	emailListSel = -1;
	audioListSel = -1;
	videoListSel = -1;

	windowTitle = "PDA Editor";

	addPDADlg.Reset();
	editEmailDlg.Reset();
	editAudioDlg.Reset();
	editVideoDlg.Reset();

	PopulatePDAList();
}

void PDAEditor::Draw() {
	int i, num;
	bool selected;
	bool showTool;

	showTool = isShown;

	idStr windowName = windowTitle + "###PDA Editor";
	if ( ImGui::Begin( windowName, &showTool, ImGuiWindowFlags_AlwaysAutoResize)) //, ImGuiWindowFlags_ShowBorders ) )
	{
		if ( ImGui::BeginTable( "table", 3 ) )
		{
			ImGui::PushID( "pda" );
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text( "PDAs:" );
			if ( ImGui::BeginListBox( "##PDAList", ImVec2( 300, 200 ) ) ) {
				num = pdaList.Num();
				for ( i = 0; i < num; i++ ) {
					ImGui::PushID( i );

					selected = ( i == pdaListSel );

					if ( ImGui::Selectable( pdaList[i].c_str(), selected ) ) {
						OnSelChangePDA( i );
					}

					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}

					ImGui::PopID();
				}
				ImGui::EndListBox();
			}
			if ( ImGui::Button( "Add" ) ) {
				OnBtnClickedPDAAdd();
			}
			/*
			* FIXME: decl manager does not support deletion
			ImGui::SameLine();
			if ( ImGui::Button( "Del" ) ) {
				OnBtnClickedPDADel();
			}
			*/

			if ( ImGui::BeginPopupModal( "PDAAdd", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted = addPDADlg.Draw();

				if ( accepted ) {
					addPDADlg.name.ToLower();
					idDecl* decl = declManager->CreateNewDecl( DECL_PDA, addPDADlg.name, "newpdas/" + addPDADlg.name + ".pda" );
					decl->ReplaceSourceFileText();
					decl->Invalidate();
					PopulatePDAList();
					int index = pdaList.FindIndex( addPDADlg.name );
					OnSelChangePDA( index );
				}

				ImGui::EndPopup();
			}

			/*
			* FIXME: decl manager does not support deletion
			if ( ImGui::BeginPopupModal( "PDADel", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				int index;

				index = pdaListSel;
				assert( index >= 0 );

				idStr &name = pdaList[ index ];

				ImGui::Text("Are you sure you want to delete PDA %s?", name.c_str() );

				if ( ImGui::Button( "OK" ) ) {
					ImGui::CloseCurrentPopup();

					const idDeclPDA* pda = dynamic_cast< const idDeclPDA * >( declManager->DeclByIndex( DECL_PDA, index ) );
					if ( pda ) {
						// FIXME: doesn't work, we need to delete the decl object itself somehow
						//fileSystem->RemoveFile( pda->GetFileName() );
						//PopulatePDAList();
					}
				}
				ImGui::SameLine();
				if ( ImGui::Button( "Cancel" ) ) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			*/

			ImGui::PopID();

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID( "pdaEdit" );
			ImGui::SetNextItemWidth( 300 );
			if ( ImGui::InputTextStr( "Full Name", &fullName ) ) {

			}
			ImGui::SetNextItemWidth( 300 );
			if ( ImGui::InputTextStr( "Short Name", &shortName ) ) {

			}
			ImGui::SetNextItemWidth( 300 );
			if ( ImGui::InputTextStr( "Title", &title ) ) {

			}
			ImGui::SetNextItemWidth( 300 );
			if ( ImGui::InputTextStr( "Post (Location)", &post ) ) {

			}
			ImGui::SetNextItemWidth( 300 );
			if ( ImGui::InputTextStr( "Security Level", &security ) ) {

			}
			ImGui::SetNextItemWidth( 50 );
			if ( ImGui::Button( "Rand" ) ) {
				OnBtnClickedRandom();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth( 255 );
			if ( ImGui::InputTextStr( "ID number", &idnum ) ) {

			}
			ImGui::PopID();

			ImGui::PushID( "audio" );
			ImGui::Dummy( ImVec2(0, 20) );
			ImGui::Text( "Audio Logs:" );
			if (ImGui::BeginListBox( "##audioList", ImVec2( 300, 200 ) ) ) {
				num = audioList.Num();
				for ( i = 0; i < num; i++ ) {
					ImGui::PushID( i );

					selected = ( i == audioListSel );
					if ( ImGui::Selectable( audioList[i].c_str(), selected ) ) {
						audioListSel = i;
					}

					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}

					ImGui::PopID();
				}
				ImGui::EndListBox();
			}
			if ( ImGui::Button( "Add" ) ) {
				OnBtnClickedAudioAdd();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Edit" ) ) {
				OnBtnClickedAudioEdit();
			}
			/*
			* FIXME: decl manager does not support deletion
			ImGui::SameLine();
			if ( ImGui::Button( "Delete" ) ) {
				OnBtnClickedAudioDel();
			}
			*/
			if ( ImGui::BeginPopupModal( "AudioAdd", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted;

				accepted = editAudioDlg.Draw();
				if ( accepted ) {
					OnAudioAdd();
				}

				ImGui::EndPopup();
			}
			if ( ImGui::BeginPopupModal( "AudioEdit", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted = editAudioDlg.Draw();

				if ( accepted ) {
					OnAudioEdit();
				}

				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::TableSetColumnIndex(2);
			ImGui::PushID( "email" );
			ImGui::Text( "Email" );
			if ( ImGui::BeginListBox( "##emailList", ImVec2( 300, 200 ) ) ) {
				num = emailList.Num();
				for ( i = 0; i < num; i++ ) {
					ImGui::PushID( i );

					selected = ( i == emailListSel );
					if ( ImGui::Selectable( emailList[i].c_str(), selected ) ) {
						emailListSel = i;
					}

					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}

					ImGui::PopID();
				}
				ImGui::EndListBox();
			}
			if ( ImGui::Button( "Add" ) ) {
				OnBtnClickedEmailAdd();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Edit" ) ) {
				OnBtnClickedEmailEdit();
			}
			/*
			* FIXME: decl manager does not support deletion
			ImGui::SameLine();
			if ( ImGui::Button( "Delete" ) ) {
				OnBtnClickedEmailDel();
			}
			*/
			if ( ImGui::BeginPopupModal( "EmailAdd", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted;

				accepted = editEmailDlg.Draw();
				if ( accepted ) {
					OnEmailAdd();
				}

				ImGui::EndPopup();
			}
			if ( ImGui::BeginPopupModal( "EmailEdit", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted = editEmailDlg.Draw();

				if ( accepted ) {
					OnEmailEdit();
				}

				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::PushID("video");
			ImGui::Dummy(ImVec2(0, 20));
			ImGui::Text("Videos");
			if ( ImGui::BeginListBox( "##videoList", ImVec2( 300, 200 ) ) ) {
				num = videoList.Num();
				for ( i = 0; i < num; i++ ) {
					selected = ( i == videoListSel );
					if ( ImGui::Selectable( videoList[i].c_str(), selected ) ) {
						videoListSel = i;
					}

					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndListBox();
			}
			if ( ImGui::Button( "Add" ) ) {
				OnBtnClickedVideoAdd();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Edit" ) ) {
				OnBtnClickedVideoEdit();
			}
			/*
			* FIXME: decl manager does not support deletion
			ImGui::SameLine();
			if ( ImGui::Button( "Delete" ) ) {
				OnBtnClickedVideoDel();
			}
			*/

			if ( ImGui::BeginPopupModal( "VideoAdd", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted;

				accepted = editVideoDlg.Draw();
				if ( accepted ) {
					OnVideoAdd();
				}

				ImGui::EndPopup();
			}
			if ( ImGui::BeginPopupModal( "VideoEdit", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
				bool accepted = editVideoDlg.Draw();

				if ( accepted ) {
					OnVideoEdit();
				}

				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::EndTable();
		}

		ImGui::BeginDisabled( !saveButtonEnabled );
		if ( ImGui::Button( "Save" ) ) {
			OnBtnClickedSave();
			showTool = false;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			showTool = false;
		}
	}
	ImGui::End();

	if ( isShown && !showTool )
	{
		isShown = showTool;
		impl::SetReleaseToolMouse( false );
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_PDAEditor );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDialogPDAEditor message handlers

void PDAEditor::OnDestroy() {

	//com_editors &= ~EDITOR_PDA;

	isShown = false;
}

void PDAEditor::PopulatePDAList()
{
	int i;
	int num = declManager->GetNumDecls(DECL_PDA);
	pdaList.Resize( num );
	pdaList.Clear();
	for ( i=0; i < num; i++ ) {
		const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, i) );
		pdaList.Append( pda->GetName() );
	}
	pdaListSel = -1;

	fullName.Clear();
	shortName.Clear();
	post.Clear();
	title.Clear();
	security.Clear();
	idnum.Clear();
}

void PDAEditor::OnSelChangePDA( int index )
{
	int i, num;

	if ( index < 0 ) {
		pdaListSel = -1;
		emailListSel = -1;
		audioListSel = -1;
		videoListSel = -1;
		windowTitle = "PDA Editor";
		saveButtonEnabled = false;
		return;
	}

	pdaListSel = index;

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex( DECL_PDA, index ) );
	if ( !pda ) {
		pdaListSel = -1;
		emailListSel = -1;
		audioListSel = -1;
		videoListSel = -1;
		return;
	}

	windowTitle = idStr::Format( "PDA Editor - %s", pda->GetName() );

	idFile *file = fileSystem->OpenFileAppend( pda->GetFileName() );
	if ( file ) {
		fileSystem->CloseFile( file );
		saveButtonEnabled = true;
	} else {
		windowTitle += " [Read Only]";
		saveButtonEnabled = false;
	}

	num = pda->GetNumEmails();
	emailList.Resize( num );
	emailList.Clear();
	emailListSel = -1;
	for ( i=0; i < num; i++ ) {
		emailList.Append( pda->GetEmailByIndex( i )->GetSubject() );
	}

	num = pda->GetNumAudios();
	audioList.Resize( num );
	audioList.Clear();
	audioListSel = -1;
	for ( i=0; i < num; i++ ) {
		audioList.Append( pda->GetAudioByIndex( i )->GetAudioName() );
	}

	num = pda->GetNumVideos();
	videoList.Resize( num );
	videoList.Clear();
	videoListSel = -1;
	for ( i=0; i < num; i++ ) {
		videoList.Append( pda->GetVideoByIndex( i )->GetVideoName() );
	}

	fullName = pda->GetFullName();
	shortName = pda->GetPdaName();
	post = pda->GetPost();
	title = pda->GetTitle();
	security = pda->GetSecurity();
	idnum = pda->GetID();
}

void PDAEditor::OnBtnClickedSave()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pdaConst = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );
	if ( pdaConst ) {
		idDeclPDA *pda = const_cast<idDeclPDA *>(pdaConst);

		idStr declText = "\n";
		declText += "pda ";
		declText += pda->GetName();
		declText += " {\n";

		declText += "\tname    \t\t\"" + shortName + "\"\n";
		declText += "\tfullname\t\t\"" + fullName + "\"\n";
		declText += "\ticon    \t\t\"\"\n";
		declText += "\tid      \t\t\"" + idnum + "\"\n";
		declText += "\tpost    \t\t\"" + post + "\"\n";
		declText += "\ttitle   \t\t\"" + title + "\"\n";
		declText += "\tsecurity\t\t\"" + security + "\"\n";

		for ( int i = 0; i < pda->GetNumEmails(); i++ ) {
			declText += "\tpda_email\t\t\"";
			declText += pda->GetEmailByIndex(i)->GetName();
			declText += "\"\n";
		}

		for ( int i = 0; i < pda->GetNumAudios(); i++ ) {
			declText += "\tpda_audio\t\t\"";
			declText += pda->GetAudioByIndex(i)->GetName();
			declText += "\"\n";
		}

		for ( int i = 0; i < pda->GetNumVideos(); i++ ) {
			declText += "\tpda_video\t\t\"";
			declText += pda->GetVideoByIndex(i)->GetName();
			declText += "\"\n";
		}

		declText += "}";

		pda->SetText( declText );
		pda->ReplaceSourceFileText();
		pda->Invalidate();
	}
}

DialogPDAAdd::DialogPDAAdd() {
	Reset();
}

void DialogPDAAdd::Reset() {
	name.Clear();
}

bool DialogPDAAdd::Draw() {
	ImGui::InputTextStr( "Name", &name );

	bool isAccepted = false;
	if ( ImGui::Button( "OK" ) ) {
		isAccepted = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if ( ImGui::Button( "Cancel" ) ) {
		isAccepted = false;
		ImGui::CloseCurrentPopup();
	}
	return isAccepted;
}


void PDAEditor::OnBtnClickedRandom()
{
	idnum = idStr::Format("%d-%02X", 1000+(rand()%8999), (rand()%255));
}

void PDAEditor::OnBtnClickedPDAAdd()
{
	addPDADlg.Reset();
	ImGui::OpenPopup( "PDAAdd" );
}

void PDAEditor::OnBtnClickedPDADel()
{
	ImGui::OpenPopup( "PDADel" );
}

void PDAEditor::OnBtnClickedEmailAdd()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );

	if ( pda ) {
		idStr name;

		// Search for an unused name
		int newIndex = pda->GetNumEmails();
		do {
			name = idStr::Format("%s_email_%d", pda->GetName(), newIndex++);
		} while ( declManager->FindType(DECL_EMAIL, name, false) != NULL );

		editEmailDlg.Reset();
		editEmailDlg.SetName( name );
		ImGui::OpenPopup( "EmailAdd" );
	}
}

void PDAEditor::OnEmailAdd()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA*>( declManager->DeclByIndex( DECL_PDA, index ) );
	if ( !pda ) {
		return;
	}

	idStr &name = editEmailDlg.GetName();
	idDeclEmail *email = static_cast<idDeclEmail *>( declManager->CreateNewDecl( DECL_EMAIL, name, pda->GetFileName() ) );
	email->SetText( editEmailDlg.GetDeclText() );
	email->ReplaceSourceFileText();
	email->Invalidate();

	pda->AddEmail( name );

	// Get it again to reparse
	const idDeclEmail *emailConst = static_cast<const idDeclEmail *>( declManager->FindType( DECL_EMAIL, name ) );
	emailList.Append( emailConst->GetSubject() );

	// Save the pda to include this email in the list
	// This has a side-effect of saving any other changes, but I don't really care right now
	OnBtnClickedSave();
}

void PDAEditor::OnBtnClickedEmailEdit()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( !pda ) {
		return;
	}

	index = emailListSel;
	if ( index < 0 ) {
		return;
	}

	editEmailDlg.SetEmail( pda->GetEmailByIndex( index ) );
	ImGui::OpenPopup( "EmailEdit" );
}

void PDAEditor::OnEmailEdit()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA*>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( !pda ) {
		return;
	}

	index = emailListSel;
	if ( index < 0 ) {
		return;
	}

	idDeclEmail* email = const_cast<idDeclEmail *>( pda->GetEmailByIndex( index ) );
	email->SetText( editEmailDlg.GetDeclText() );
	email->ReplaceSourceFileText();
	email->Invalidate();

	// Get it again to reparse
	email = const_cast<idDeclEmail *>( pda->GetEmailByIndex( index ) );

	emailList.RemoveIndex( index );
	emailList.Insert( email->GetSubject(), index );
}

void PDAEditor::OnBtnClickedEmailDel()
{
}

void PDAEditor::OnBtnClickedAudioAdd()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( pda ) {
		idStr name;

		// Search for an unused name
		int newIndex = pda->GetNumAudios();
		do {
			name = idStr::Format("%s_audio_%d", pda->GetName(), newIndex++);
		} while ( declManager->FindType(DECL_AUDIO, name, false) != NULL );

		editAudioDlg.Reset();
		editAudioDlg.SetName( name );
		ImGui::OpenPopup( "AudioAdd" );
	}
}

void PDAEditor::OnAudioAdd()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA*>( declManager->DeclByIndex( DECL_PDA, index ) );
	if ( !pda ) {
		return;
	}

	idStr &name = editAudioDlg.GetName();
	idDeclEmail *email = static_cast<idDeclEmail *>( declManager->CreateNewDecl( DECL_AUDIO, name, pda->GetFileName() ) );
	email->SetText( editAudioDlg.GetDeclText() );
	email->ReplaceSourceFileText();
	email->Invalidate();

	pda->AddAudio( name );

	// Get it again to reparse
	const idDeclAudio *audioConst = static_cast<const idDeclAudio *>( declManager->FindType( DECL_AUDIO, name ) );
	audioList.Append( audioConst->GetAudioName() );

	// Save the pda to include this audio in the list
	// This has a side-effect of saving any other changes, but I don't really care right now
	OnBtnClickedSave();
}

void PDAEditor::OnBtnClickedAudioEdit()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( !pda ) {
		return;
	}

	index = audioListSel;
	if ( index < 0 ) {
		return;
	}

	editAudioDlg.SetAudio( pda->GetAudioByIndex( index ) );
	ImGui::OpenPopup( "AudioEdit" );
}

void PDAEditor::OnAudioEdit() {
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA*>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( !pda ) {
		return;
	}

	index = audioListSel;
	if ( index < 0 ) {
		return;
	}

	idDeclAudio *audio = const_cast<idDeclAudio *>( pda->GetAudioByIndex( index ) );
	audio->SetText( editAudioDlg.GetDeclText() );
	audio->ReplaceSourceFileText();
	audio->Invalidate();

	// Get it again to reparse
	audio = const_cast<idDeclAudio *>( pda->GetAudioByIndex( index ) );

	audioList.RemoveIndex( index );
	audioList.Insert( audio->GetAudioName(), index );
}

void PDAEditor::OnBtnClickedAudioDel()
{
}

void PDAEditor::OnBtnClickedVideoAdd()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( pda ) {
		idStr name;

		// Search for an unused name
		int newIndex = pda->GetNumVideos();
		do {
			name = idStr::Format("%s_video_%d", pda->GetName(), newIndex++);
		} while ( declManager->FindType(DECL_VIDEO, name, false) != NULL );

		editVideoDlg.Reset();
		editVideoDlg.SetName( name );
		ImGui::OpenPopup( "VideoAdd" );
	}
}

void PDAEditor::OnVideoAdd() {
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA*>( declManager->DeclByIndex( DECL_PDA, index ) );
	if ( !pda ) {
		return;
	}

	idStr &name = editVideoDlg.GetName();
	idDeclVideo *video = static_cast<idDeclVideo *>( declManager->CreateNewDecl( DECL_VIDEO, name, pda->GetFileName() ) );
	video->SetText( editVideoDlg.GetDeclText() );
	video->ReplaceSourceFileText();
	video->Invalidate();

	pda->AddVideo( name );

	// Get it again to reparse
	const idDeclVideo *videoConst = static_cast<const idDeclVideo *>( declManager->FindType( DECL_VIDEO, name ) );
	videoList.Append( videoConst->GetVideoName() );

	// Save the pda to include this video in the list
	// This has a side-effect of saving any other changes, but I don't really care right now
	OnBtnClickedSave();
}

void PDAEditor::OnBtnClickedVideoEdit()
{
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( !pda ) {
		return;
	}

	index = videoListSel;
	if ( index < 0 ) {
		return;
	}

	editVideoDlg.SetVideo( pda->GetVideoByIndex( index ) );
	ImGui::OpenPopup( "VideoEdit" );
}

void PDAEditor::OnVideoEdit() {
	int index = pdaListSel;
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA*>( declManager->DeclByIndex( DECL_PDA, index ) );

	if ( !pda ) {
		return;
	}

	index = videoListSel;
	if ( index < 0 ) {
		return;
	}

	idDeclVideo *video = const_cast<idDeclVideo *>( pda->GetVideoByIndex( index ) );
	video->SetText( editVideoDlg.GetDeclText() );
	video->ReplaceSourceFileText();
	video->Invalidate();

	// Get it again to reparse
	video = const_cast<idDeclVideo *>( pda->GetVideoByIndex( index ) );

	videoList.RemoveIndex( index );
	videoList.Insert( video->GetVideoName(), index );
}

void PDAEditor::OnBtnClickedVideoDel()
{
}




DialogPDAEditEmail::DialogPDAEditEmail()
{
	Reset();
}

bool DialogPDAEditEmail::Draw()
{
	ImGui::InputTextStr( "To", &to );
	ImGui::InputTextStr( "From", &from );
	ImGui::InputTextStr( "Date", &date );
	ImGui::InputTextStr( "Subject", &subject );
	ImGui::InputTextMultilineStr( "Body", &body );

	bool isAccepted = false;

	if ( ImGui::Button( "OK" ) ) {
		isAccepted = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if ( ImGui::Button( "Cancel" ) ) {
		isAccepted = false;
		ImGui::CloseCurrentPopup();
	}

	return isAccepted;
}

DialogPDAEditAudio::DialogPDAEditAudio()
{
	Reset();
}

bool DialogPDAEditAudio::Draw()
{
	ImGui::InputTextStr("Wave", &wave);
	ImGui::InputTextStr("Audio Name", &audioName);
	ImGui::InputTextMultilineStr("Info", &info);
	ImGui::InputTextStr("Preview", &preview);

	bool isAccepted = false;

	if ( ImGui::Button( "OK" ) ) {
		isAccepted = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if ( ImGui::Button( "Cancel" ) ) {
		isAccepted = false;
		ImGui::CloseCurrentPopup();
	}

	return isAccepted;
}

DialogPDAEditVideo::DialogPDAEditVideo()
{
	Reset();
}

bool DialogPDAEditVideo::Draw()
{
	ImGui::InputTextStr("Video Name", &videoName);
	ImGui::InputTextMultilineStr("Info", &info);
	ImGui::InputTextStr("Roq", &video);
	ImGui::InputTextStr("Wave", &audio);
	ImGui::InputTextStr("Preview", &preview);

	bool isAccepted = false;

	if ( ImGui::Button("OK" ) ) {
		isAccepted = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if ( ImGui::Button( "Cancel" ) ) {
		isAccepted = false;
		ImGui::CloseCurrentPopup();
	}

	return isAccepted;
}

/////////////////////////////////////////////////////////////////////////////
// DialogPDAEditEmail message handlers

void DialogPDAEditEmail::Reset()
{
	to.Clear();
	from.Clear();
	date.Clear();
	subject.Clear();
	body.Clear();

	name.Clear();
}

void DialogPDAEditEmail::SetName( idStr &_name )
{
	name = _name;
}

idStr &DialogPDAEditEmail::GetName()
{
	return name;
}

void DialogPDAEditEmail::SetEmail( const idDeclEmail *email )
{
	to = email->GetTo();
	from = email->GetFrom();
	date = email->GetDate();
	subject = email->GetSubject();
	body = email->GetBody();

	name = email->GetName();
}

idStr DialogPDAEditEmail::GetDeclText()
{
	idStr mungedBody = body;
	mungedBody.Replace("\n", "\\n\"\n\"");

	idStr declText;
	declText += "\n";
	declText += "email " + name + " {\n";
	declText += "\tto     \t\t\"" + to + "\"\n";
	declText += "\tfrom   \t\t\"" + from + "\"\n";
	declText += "\tdate   \t\t\"" + date + "\"\n";
	declText += "\tsubject\t\t\"" + subject + "\"\n";
	declText += "\ttext {\n";
	declText += "\"" + mungedBody + "\"\n";
	declText += "\t}\n";
	declText += "}";

	return declText;
}

/////////////////////////////////////////////////////////////////////////////
// DialogPDAEditAudio message handlers

void DialogPDAEditAudio::Reset()
{
	wave.Clear();
	audioName.Clear();
	info.Clear();
	preview.Clear();

	name.Clear();
}

void DialogPDAEditAudio::SetName(idStr& _name)
{
	name = _name;
}

idStr &DialogPDAEditAudio::GetName()
{
	return name;
}

void DialogPDAEditAudio::SetAudio( const idDeclAudio *_audio )
{
	wave = _audio->GetWave();
	audioName = _audio->GetAudioName();
	info = _audio->GetInfo();
	preview = _audio->GetPreview();

	name = _audio->GetName();
}

idStr DialogPDAEditAudio::GetDeclText()
{
	idStr escapedInfo = info;
	escapedInfo.Replace("\n", "\\n");

	idStr declText;

	declText += "\n";
	declText += "audio " + name + " {\n";
	declText += "\tname     \t\t\"" + audioName + "\"\n";
	declText += "\tinfo   \t\t\"" + escapedInfo + "\"\n";
	declText += "\tpreview   \t\t\"" + preview + "\"\n";
	declText += "\taudio\t\t\"" + wave + "\"\n";
	declText += "}";

	return declText;
}

/////////////////////////////////////////////////////////////////////////////
// DialogPDAEditVideo message handlers

void DialogPDAEditVideo::Reset()
{
	preview.Clear();
	video.Clear();
	videoName.Clear();
	info.Clear();
	audio.Clear();

	name.Clear();
}

void DialogPDAEditVideo::SetName(idStr& _name)
{
	name = _name;
}

idStr& DialogPDAEditVideo::GetName()
{
	return name;
}

void DialogPDAEditVideo::SetVideo(const idDeclVideo* _video)
{
	video = _video->GetRoq();
	audio = _video->GetWave();
	videoName = _video->GetVideoName();
	info = _video->GetInfo();
	preview = _video->GetPreview();

	name = _video->GetName();
}

idStr DialogPDAEditVideo::GetDeclText()
{
	idStr escapedInfo = info;
	escapedInfo.Replace("\n", "\\n");

	idStr declText;
	declText += "\n";
	declText += "video " + name + " {\n";
	declText += "\tname     \t\t\"" + videoName + "\"\n";
	declText += "\tinfo   \t\t\"" + escapedInfo + "\"\n";
	declText += "\tvideo   \t\t\"" + video + "\"\n";
	declText += "\taudio\t\t\"" + audio + "\"\n";
	declText += "\tpreview\t\t\"" + preview + "\"\n";
	declText += "}";

	return declText;
}

}
