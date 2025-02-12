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

#include "../../libs/ImGuiColorTextEdit/TextEditor.h"

#include "../util/ImGui_IdWidgets.h"

#include "ScriptEditor.h"

#include "framework/FileSystem.h"

#include "sys/sys_imgui.h"

namespace ImGuiTools {

static const int		TAB_SIZE = 4;

typedef struct scriptEventInfo_s {
	idStr		name;
	idStr		parms;
	idStr		help;
} scriptEventInfo_t;

static idList<scriptEventInfo_t> scriptEvents;

// DialogScriptEditor dialog
/*
static UINT FindDialogMessage = ::RegisterWindowMessage( FINDMSGSTRING );

toolTip_t DialogScriptEditor::toolTips[] = {
	{ IDOK, "save" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};


IMPLEMENT_DYNAMIC(DialogScriptEditor, CDialog)
*/

ScriptEditor& ScriptEditor::Instance()
{
	static ScriptEditor instance;
	return instance;
}

ScriptEditor::ScriptEditor()
	: isShown( false )
	, windowText()
	, statusBarText( "Script Editor" )
	, scriptEdit(NULL)
	, okButtonEnabled( false )
	, cancelButtonEnabled( true )
	, errorText()
	, gotoDlg()
	, findStr()
	, replaceStr()
	, matchCase( false )
	, matchWholeWords( false )
	, searchForward( true )
	, fileName()
	, firstLine( 0 )
{
	scriptEdit = new TextEditor();
}

void ScriptEditor::Reset() {
	windowText = "Script Editor###ScriptEditor";
	statusBarText.Clear();
	scriptEdit->SetText( std::string( "" ) );
	scriptEdit->SetTabSize( TAB_SIZE );
	okButtonEnabled = false;
	cancelButtonEnabled = true;
	errorText.Clear();
	findStr.Clear();
	replaceStr.Clear();
	matchCase = false;
	matchWholeWords = false;
	searchForward = true;
	fileName.Clear();
	firstLine = 0;

	UpdateStatusBar();
}

void ScriptEditor::Draw()
{
	bool showTool;
	bool clickedNew = false, clickedSelect = false;

	showTool = isShown;

	if ( ImGui::Begin( windowText.c_str(), &showTool, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar ) ) {
		impl::SetReleaseToolMouse(true);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					clickedNew = true;
				}

				if (ImGui::MenuItem("Open..", "Ctrl+O"))
				{
					clickedSelect = true;
				}

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					OnBnClickedOk();
				}

				if (ImGui::MenuItem("Close", "Ctrl+W"))
				{
					showTool = false;
				}

				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu( "Edit" ) ) {
				if ( ImGui::MenuItem( "Go To...", "Ctrl+G" ) ) {
					OnEditGoToLine();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (clickedNew) {
		}
		if (clickedSelect) {
		}

		scriptEdit->Render( "Text", ImVec2( 800, 600 ), false );
		if ( !scriptEdit->IsSaved() ) {
			okButtonEnabled = true;
		}
		UpdateStatusBar();

		ImGui::BeginDisabled( !okButtonEnabled );
		if ( ImGui::Button( "OK" ) ) {
			OnBnClickedOk();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled( !cancelButtonEnabled );
		if ( ImGui::Button( "Cancel" ) ) {
			showTool = false;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Go to")) {
			OnEditGoToLine();
		}

		if ( gotoDlg.Draw() ) {
			TextEditor::Coordinates coords( gotoDlg.GetLine() - 1 - firstLine, 0 );

			scriptEdit->SetCursorPosition( coords );
		}

		ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "%s", errorText.c_str() );
		ImGui::TextUnformatted( statusBarText.c_str() );
	}
	ImGui::End();

	if ( isShown && !showTool )
	{
		isShown = showTool;
		impl::SetReleaseToolMouse( false );
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_ScriptEditor );
	}
}

/*
================
DialogScriptEditor::PreTranslateMessage
================
*//*
BOOL DialogScriptEditor::PreTranslateMessage( MSG* pMsg ) {
	if ( WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST ) {
		if ( m_hAccel && ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}*/

/*
================
ScriptEditor::UpdateStatusBar
================
*/
void ScriptEditor::UpdateStatusBar( void ) {
	TextEditor::Coordinates coords = scriptEdit->GetCursorPosition();
	statusBarText = idStr::Format( "Line: %d, Column: %d", coords.mLine + 1, coords.mColumn + 1 );
}

/*
================
ScriptEditor::InitScriptEvents
================
*/
void ScriptEditor::InitScriptEvents( void ) {
	int index;
	idParser src;
	idToken token;
	idStr whiteSpace;
	scriptEventInfo_t info;

	if ( !src.LoadFile( "script/doom_events.script" ) ) {
		return;
	}

	scriptEvents.Clear();

	while( src.ReadToken( &token ) ) {
		if ( token == "scriptEvent" ) {

			src.GetLastWhiteSpace( whiteSpace );
			index = whiteSpace.Find( "//" );
			if ( index != -1 ) {
				info.help = whiteSpace.Right( whiteSpace.Length() - index );
				info.help.Replace( "\r", "" );
				info.help.Replace( "\n", "\r\n" );
			} else {
				info.help = "";
			}

			src.ExpectTokenType( TT_NAME, 0, &token );

			info.parms = token;

			src.ExpectTokenType( TT_NAME, 0, &token );

			info.name = token;

			src.ExpectTokenString( "(" );

			info.parms += " " + info.name + "(";
			while( src.ReadToken( &token ) && token != ";" ) {
				info.parms.Append( " " + token );
			}

			scriptEvents.Append( info );
		}
	}
}

/*
================
GetScriptEvents
================
*/
bool GetScriptEvents( const char *objectName, idList<idStr> &listBox ) {
	for ( int i = 0; i < scriptEvents.Num(); i++ ) {
		listBox.Append( scriptEvents[i].name );
	}
	return true;
}

/*
================
GetFunctionParms
================
*/
bool GetFunctionParms( const char *funcName, idStr &parmString ) {
	for ( int i = 0; i < scriptEvents.Num(); i++ ) {
		if ( scriptEvents[i].name.Cmp( funcName ) == 0 ) {
			parmString = scriptEvents[i].parms;
			return true;
		}
	}
	return false;
}

/*
================
GetToolTip
================
*/
bool GetToolTip( const char *name, idStr &string ) {
	for ( int i = 0; i < scriptEvents.Num(); i++ ) {
		if ( scriptEvents[i].name.Cmp( name ) == 0 ) {
			string = scriptEvents[i].help + scriptEvents[i].parms;
			return true;
		}
	}
	return false;
}

/*
================
ScriptEditor::OpenFile
================
*/
void ScriptEditor::OpenFile( const char *fileName ) {
	int numLines = 0;
	int numCharsPerLine = 0;
	int maxCharsPerLine = 0;
	idStr scriptText, extension;
	//CRect rect;
	void *buffer;

	//scriptEdit.Init();
	//scriptEdit.AllowPathNames( false );
	scriptEdit->SetText(std::string(""));

	idStr( fileName ).ExtractFileExtension( extension );

	if ( extension.Icmp( "script" ) == 0 ) {
		InitScriptEvents();
		scriptEdit->SetLanguageDefinition( TextEditor::LanguageDefinition::CPlusPlus() );
		/*
		scriptEdit.SetCaseSensitive( true );
		scriptEdit.LoadKeyWordsFromFile( "editors/script.def" );
		scriptEdit.SetObjectMemberCallback( GetScriptEvents );
		scriptEdit.SetFunctionParmCallback( GetFunctionParms );
		scriptEdit.SetToolTipCallback( GetToolTip );
		*/
	} else if ( extension.Icmp( "gui" ) == 0 ) {
		/*
		scriptEdit.SetStringColor(SRE_COLOR_DARK_CYAN, SRE_COLOR_LIGHT_BROWN);
		scriptEdit.LoadKeyWordsFromFile( "editors/gui.def" );
		*/
		scriptEdit->SetLanguageDefinition( TextEditor::LanguageDefinition::C() );
	}

	if ( fileSystem->ReadFile( fileName, &buffer ) == -1 ) {
		errorText = "Unable to read the selected file";
		return;
	}
	scriptText = (char *) buffer;
	fileSystem->FreeFile( buffer );

	this->fileName = fileName;

	scriptEdit->SetText( scriptText.c_str() );

	for( const char *ptr = scriptText.c_str(); *ptr; ptr++ ) {
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

	windowText = va( "Script Editor (%s)###ScriptEditor", fileName );
	/*
	rect.left = initialRect.left;
	rect.right = rect.left + maxCharsPerLine * FONT_WIDTH + 32;
	rect.top = initialRect.top;
	rect.bottom = rect.top + numLines * (FONT_HEIGHT+8) + 24 + 56;
	if ( rect.right < initialRect.right ) {
		rect.right = initialRect.right;
	} else if ( rect.right - rect.left > 1024 ) {
		rect.right = rect.left + 1024;
	}
	if ( rect.bottom < initialRect.bottom ) {
		rect.bottom = initialRect.bottom;
	} else if ( rect.bottom - rect.top > 768 ) {
		rect.bottom = rect.top + 768;
	}
	MoveWindow( rect );
	*/
	okButtonEnabled = false;
	cancelButtonEnabled = true;

	UpdateStatusBar();

	//scriptEdit.SetFocus();
}

// ScriptEditor message handlers

#define BORDER_SIZE			0
#define BUTTON_SPACE		4
#define TOOLBAR_HEIGHT		24

/*
================
ScriptEditor::OnEditGoToLine
================
*/
void ScriptEditor::OnEditGoToLine() {
	TextEditor::Coordinates coords = scriptEdit->GetCursorPosition();

	gotoDlg.Start( firstLine + 1, firstLine + scriptEdit->GetTotalLines(), coords.mLine + 1 );

	/*DialogGoToLine goToLineDlg;

	goToLineDlg.SetRange( firstLine, firstLine + scriptEdit.GetLineCount() - 1 );
	if ( goToLineDlg.DoModal() != IDOK ) {
		return;
	}
	scriptEdit.GoToLine( goToLineDlg.GetLine() - firstLine );
	*/
}

/*
================
ScriptEditor::OnEditFind
================
*/
void ScriptEditor::OnEditFind() {
	idStr selText = scriptEdit->GetSelectedText().c_str();
	if ( selText.Length() ) {
		findStr = selText;
	}

	/*
	// create find/replace dialog
	if ( !findDlg ) {
		findDlg = new CFindReplaceDialog();  // Must be created on the heap
		findDlg->Create( TRUE, findStr, "", FR_DOWN, this );
	}
	*/
}

/*
================
ScriptEditor::OnEditFindNext
================
*/
void ScriptEditor::OnEditFindNext() {
	
	if ( searchForward ) {
		//scriptEdit->( findStr.c_str(), findStr.Length(), matchCase ); // TODO: implement whole word match
	} else {
		// TODO: implement search backward
	}
}

/*
================
ScriptEditor::OnEditReplace
================
*/
void ScriptEditor::OnEditReplace() {
	/*
	* TODO: get selected text?
	idStr selText = scriptEdit->GetSelText();
	if ( selText.Length() ) {
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
ScriptEditor::OnFindDialogMessage
================
*//*
LRESULT ScriptEditor::OnFindDialogMessage( WPARAM wParam, LPARAM lParam ) {
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

		scriptEdit.GetSel( selStart, selEnd );
		if ( selEnd > selStart ) {
			scriptEdit.ReplaceSel( replaceStr, TRUE );
		}
	}

	if ( findDlg->ReplaceAll() ) {
		replaceStr = findDlg->GetReplaceString();
		findStr = findDlg->GetFindString();
		matchCase = findDlg->MatchCase() != FALSE;
		matchWholeWords = findDlg->MatchWholeWord() != FALSE;

		int numReplaces = scriptEdit.ReplaceAll( findStr, replaceStr, matchCase, matchWholeWords );
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
DialogScriptEditor::OnEnChangeEdit
================
*//*
void ScriptEditor::OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	okButtonEnabled = true;
}*/

/*
================
ScriptEditor::OnEnInputEdit
================
*//*
void ScriptEditor::OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	MSGFILTER *msgFilter = (MSGFILTER *)pNMHDR;

	if ( msgFilter->msg != 512 && msgFilter->msg != 33 ) {
		UpdateStatusBar();
	}

	*pResult = 0;
}*/

/*
================
ScriptEditor::OnBnClickedOk
================
*/
void ScriptEditor::OnBnClickedOk() {
	idStr scriptText;

	common->Printf( "Writing \'%s\'...\n", fileName.c_str() );

	scriptText = scriptEdit->GetText().c_str();

	if ( fileSystem->WriteFile( fileName, scriptText, scriptText.Length(), "fs_devpath" ) == -1 ) {
		errorText = va( "Couldn't save: %s", fileName.c_str() );
		return;
	}

	okButtonEnabled = false;
}

/*
================
ScriptEditor::OnBnClickedCancel
================
*/
void ScriptEditor::OnBnClickedCancel() {
	if ( okButtonEnabled ) {
		//if ( MessageBox( "Cancel changes?", "Cancel", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
			return;
		//}
	}
	//OnCancel();
}

}
