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

#include "PathTreeCtrl.h"
#include "DeclEditor.h"
#include "DeclBrowser.h"

namespace ImGuiTools {

static const int TAB_SIZE = 4;

// DeclEditor dialog

/*
toolTip_t DialogDeclEditor::toolTips[] = {
	{ IDC_DECLEDITOR_BUTTON_TEST, "test decl" },
	{ IDOK, "save decl" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};
*/

/*
================
DeclEditor::DeclEditor
================
*/
DeclEditor::DeclEditor()
	: isShown(false)
	, windowText()
	, statusBarText()
	, declEdit()
	, testButtonEnabled(false)
	, okButtonEnabled(false)
	, cancelButtonEnabled(true)
	, errorText()
	, findStr()
	, replaceStr()
	, matchCase(false)
	, matchWholeWords(false)
	, searchForward(true)
	, decl(NULL)
	, firstLine(0)
{
}

/*
================
DeclEditor::TestDecl
================
*/
bool DeclEditor::TestDecl( const idStr &declText ) {
	idLexer src( LEXFL_NOSTRINGCONCAT );
	idToken token;
	int indent;

	errorText.Clear();

	src.LoadMemory( declText, declText.Length(), "decl text" );

	indent = 0;
	while( src.ReadToken( &token ) ) {
		if ( token == "{" ) {
			indent++;
		} else if ( token == "}" ) {
			indent--;
		}
	}

	if ( indent < 0 ) {
		errorText = "Missing opening brace!";
		return false;
	}
	if ( indent > 0 ) {
		errorText = "Missing closing brace!";
		return false;
	}
	return true;
}

/*
================
DeclEditor::UpdateStatusBar
================
*/
void DeclEditor::UpdateStatusBar( void ) {
	/*int line, column, character;

	if ( decl ) {
		declEdit.GetCursorPos( line, column, character );
		statusBar.SetWindowText( va( "Line: %d, Column: %d, Character: %d", decl->GetLineNum() + line, column, character ) );
	}*/
}

/*
================
DeclEditor::Reset
================
*/
void DeclEditor::Reset() {

	//declEdit.Init();
	windowText.Clear();
	declEdit.Clear();

	testButtonEnabled = false;
	okButtonEnabled = false;

	UpdateStatusBar();
}

/*
================
DeclEditor::Start
================
*/
void DeclEditor::Start( idDecl *decl ) {
	int numLines = 0;
	int numCharsPerLine = 0;
	int maxCharsPerLine = 0;
	idStr declText;
	//CRect rect;

	this->decl = decl;

	ShowIt( true );

	switch( decl->GetType() ) {
		case DECL_ENTITYDEF:
			//declEdit.SetStringColor( SRE_COLOR_BLUE, SRE_COLOR_DARK_CYAN );
			//declEdit.LoadKeyWordsFromFile( "editors/entity.def" );
			break;
		case DECL_MATERIAL:
			//declEdit.LoadKeyWordsFromFile( "editors/material.def" );
			break;
		case DECL_SKIN:
			//declEdit.LoadKeyWordsFromFile( "editors/skin.def" );
			break;
		case DECL_SOUND:
			//declEdit.LoadKeyWordsFromFile( "editors/sound.def" );
			break;
		case DECL_FX:
			//declEdit.LoadKeyWordsFromFile( "editors/fx.def" );
			break;
		case DECL_PARTICLE:
			//declEdit.LoadKeyWordsFromFile( "editors/particle.def" );
			break;
		case DECL_AF:
			//declEdit.LoadKeyWordsFromFile( "editors/af.def" );
			break;
		case DECL_TABLE:
			//declEdit.LoadKeyWordsFromFile( "editors/table.def" );
			break;
		case DECL_MODELDEF:
			//declEdit.LoadKeyWordsFromFile( "editors/model.def" );
			break;
		default:
			//declEdit.LoadKeyWordsFromFile( va( "editors/%s.def", declManager->GetDeclNameFromType( decl->GetType() ) ) );
			break;
	}

	firstLine = decl->GetLineNum();

	char *localDeclText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
	decl->GetText( localDeclText );
	declText = localDeclText;

	declEdit = declText;

	for( const char *ptr = declText.c_str(); *ptr; ptr++ ) {
		if ( *ptr == '\r' ) {
			if ( numCharsPerLine > maxCharsPerLine ) {
				maxCharsPerLine = numCharsPerLine;
			}
			numCharsPerLine = 0;
			numLines++;
		} else if ( *ptr == '\t' ) {
			numCharsPerLine += TAB_SIZE;
		} else {
			numCharsPerLine++;
		}
	}

	windowText = va( "Declaration Editor (%s, line %d)", decl->GetFileName(), decl->GetLineNum() );
	
	/*
	float scaling_factor = Win_GetWindowScalingFactor(GetSafeHwnd());

	rect.left = initialRect.left;
	rect.right = rect.left + (maxCharsPerLine * FONT_WIDTH + 32) *scaling_factor;
	rect.top = initialRect.top;
	rect.bottom = rect.top + (numLines * (FONT_HEIGHT+8) + 24 + 56)* scaling_factor;
	if ( rect.right < initialRect.right ) {
		rect.right = initialRect.right;
	} else if ( rect.right - rect.left > (1024 * scaling_factor) ) {
		rect.right = rect.left + (1024 * scaling_factor);
	}
	if ( rect.bottom < initialRect.bottom ) {
		rect.bottom = initialRect.bottom;
	} else if ( rect.bottom - rect.top > (768 * scaling_factor)  ) {
		rect.bottom = rect.top + (768 * scaling_factor);
	}
	MoveWindow( rect );
	*/

	testButtonEnabled = false;
	okButtonEnabled = false;

	UpdateStatusBar();

	//declEdit.SetFocus();

	ImGui::OpenPopup( "Declaration Editor" );
}

bool DeclEditor::Draw() {
	bool accepted = false;

	if ( !isShown ) {
		return false;
	}

	if ( ImGui::BeginPopup( "Declaration Editor" ) ) {

		if ( ImGui::InputTextMultilineStr( "Text", &declEdit, ImVec2( 500, 500 ) ) ) {
			testButtonEnabled = true;
			okButtonEnabled = true;
		}

		ImGui::BeginDisabled( !testButtonEnabled );
		if ( ImGui::Button( "Test" ) ) {
			OnBnClickedTest();
		}
		ImGui::EndDisabled();

		ImGui::BeginDisabled( !okButtonEnabled );
		if ( ImGui::Button( "Save" ) ) {
			if ( OnBnClickedOk() ) {
				accepted = true;
				ImGui::CloseCurrentPopup();
			}
		}
		bool subPopupAccepted = false;
		if ( ImGui::BeginPopup( "Warning saving" ) ) {
			ImGui::TextUnformatted( va( "Declaration file %s has been modified outside of the editor.\r\nReload declarations and save?", decl->GetFileName() ) );

			if ( ImGui::Button( "OK" ) ) {
				OnBnClickedOkAccepted();
				subPopupAccepted = true;
				ImGui::CloseCurrentPopup();
			}
			if ( ImGui::Button( "Cancel" ) ) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		if ( subPopupAccepted ) {
			accepted = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled( !cancelButtonEnabled );
		if ( ImGui::Button( "Cancel" ) ) {
			OnBnClickedCancel();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::EndPopup();
	}

	if ( accepted )
	{
		isShown = false;
	}

	return accepted;
}

// DeclEditor message handlers

/*
================
DeclEditor::OnEditGoToLine
================
*/
void DeclEditor::OnEditGoToLine() {
	/*
	DialogGoToLine goToLineDlg;

	goToLineDlg.SetRange( firstLine, firstLine + declEdit.GetLineCount() - 1 );
	if ( goToLineDlg.DoModal() != IDOK ) {
		return;
	}
	declEdit.GoToLine( goToLineDlg.GetLine() - firstLine );
	*/
}

/*
================
DeclEditor::OnEditFind
================
*/
void DeclEditor::OnEditFind() {
	/*
	idStr selText = declEdit.GetSelText();
	if ( selText.GetLength() ) {
		findStr = selText;
	}

	if ( !findDlg ) {
		// create find/replace dialog
		findDlg = new CFindReplaceDialog();  // Must be created on the heap
		findDlg->Create( TRUE, findStr, "", FR_DOWN, this );
	}*/
}

/*
================
DeclEditor::OnEditFindNext
================
*/
void DeclEditor::OnEditFindNext() {
	/*
	if ( declEdit.FindNext( findStr, matchCase, matchWholeWords, searchForward ) ) {
		declEdit.SetFocus();
	} else {
		AfxMessageBox( "The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0 );
	}
	*/
}

/*
================
DeclEditor::OnEditReplace
================
*/
void DeclEditor::OnEditReplace() {
	/*
	idStr selText = declEdit.GetSelText();
	if ( selText.GetLength() ) {
		findStr = selText;
	}

	// create find/replace dialog
	if ( !findDlg ) {
		findDlg = new CFindReplaceDialog();  // Must be created on the heap
		findDlg->Create( FALSE, findStr, "", FR_DOWN, this );
	}
	*/
}

/*
================
DeclEditor::OnFindDialogMessage
================
*//*
LRESULT DeclEditor::OnFindDialogMessage( WPARAM wParam, LPARAM lParam ) {
	if ( findDlg == NULL ) {
		return 0;
	}

	if ( findDlg->IsTerminating() ) {
		findDlg = NULL;
		return 0;
	}

	if( findDlg->FindNext() ) {
		findStr = findDlg->GetFindString();
		matchCase = findDlg->MatchCase() != FALSE;
		matchWholeWords = findDlg->MatchWholeWord() != FALSE;
		searchForward = findDlg->SearchDown() != FALSE;

		OnEditFindNext();
	}

	if ( findDlg->ReplaceCurrent() ) {
		long selStart, selEnd;

		replaceStr = findDlg->GetReplaceString();

		declEdit.GetSel( selStart, selEnd );
		if ( selEnd > selStart ) {
			declEdit.ReplaceSel( replaceStr, TRUE );
		}
	}

	if ( findDlg->ReplaceAll() ) {
		replaceStr = findDlg->GetReplaceString();
		findStr = findDlg->GetFindString();
		matchCase = findDlg->MatchCase() != FALSE;
		matchWholeWords = findDlg->MatchWholeWord() != FALSE;

		int numReplaces = declEdit.ReplaceAll( findStr, replaceStr, matchCase, matchWholeWords );
		if ( numReplaces == 0 ) {
			AfxMessageBox( "The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0 );
		} else {
			AfxMessageBox( va( "Replaced %d occurances.", numReplaces ), MB_OK | MB_ICONINFORMATION, 0 );
		}
	}

	return 0;
}*/

/*
================
DeclEditor::OnEnInputEdit
================
*//*
void DeclEditor::OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	MSGFILTER* msgFilter = (MSGFILTER*)pNMHDR;

	if ( msgFilter->msg != 512 && msgFilter->msg != 33 ) {
		UpdateStatusBar();
	}

	*pResult = 0;
}*/

/*
================
DeclEditor::OnBnClickedTest
================
*/
void DeclEditor::OnBnClickedTest() {
	idStr declText;

	if ( decl ) {

		declText = declEdit; //declEdit.GetText( declText );

		if ( !TestDecl( declText ) ) {
			return;
		}

		char *oldDeclText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
		decl->GetText( oldDeclText );
		decl->SetText( declText );
		decl->Invalidate();
		declManager->DeclByIndex( decl->GetType(), decl->Index(), true );
		decl->SetText( oldDeclText );
		decl->Invalidate();
		common->Printf( "tested %s\n", decl->GetName() );

		testButtonEnabled = false;
	}
}

/*
================
DeclEditor::OnBnClickedOk
================
*/
bool DeclEditor::OnBnClickedOk() {
	idStr declText;

	if ( decl ) {

		declText = declEdit; // declEdit.GetText( declText );

		if ( !TestDecl( declText ) ) {
			return false;
		}

		if ( decl->SourceFileChanged() ) {
			ImGui::OpenPopup( "Warning saving" );
			return false;
		}

		decl->SetText( declText );
		if ( !decl->ReplaceSourceFileText() ) {
			errorText = va( "Couldn't save: %s.\r\nMake sure the declaration file is not read-only.", decl->GetFileName() );
			return false;
		}
		decl->Invalidate();
	}

	okButtonEnabled = false;
	return true;
}

void DeclEditor::OnBnClickedOkAccepted() {
	idStr declText;

	if ( decl ) {

		declText = declEdit; // declEdit.GetText( declText );

		declManager->Reload( false );
		DeclBrowser::Instance().ReloadDeclarations();

		decl->SetText( declText );
		if ( !decl->ReplaceSourceFileText() ) {
			errorText = va( "Couldn't save: %s.\r\nMake sure the declaration file is not read-only.", decl->GetFileName() );
			return;
		}
		decl->Invalidate();
	}
	
	okButtonEnabled = false;
}

/*
================
DeclEditor::OnBnClickedCancel
================
*/
void DeclEditor::OnBnClickedCancel() {
	if ( okButtonEnabled ) {
		/*
		if ( MessageBox( "Cancel changes?", "Cancel", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
			return;
		}*/
	}
	//OnCancel();
}

}
