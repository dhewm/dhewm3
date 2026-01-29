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

#include "ConsoleView.h"

#define EDIT_HEIGHT 25

namespace ImGuiTools {

/**
* Constructor for ConsoleView.
*/
ConsoleView::ConsoleView()
	: editConsole(NULL)
	, limitConsoleLines(100)
	, editInputLastKeyDownTime(0)
{
}

/**
* Destructor for ConsoleView.
*/
ConsoleView::~ConsoleView() {
}

void ConsoleView::Init() {
	editConsole = new TextEditor();
	editConsole->SetReadOnly( true );
	editConsole->SetShowLineNumber( false );
	editConsole->SetShowCurrentLine( false );
	editConsole->SetColorizerEnable( false );
	editConsole->SetShowWhitespaces( false );

	limitConsoleLines = 100;
}

/**
* Adds text to the end of the console output window.
* @param msg The text to append.
* \todo: BMatt Nerve: Fix scroll code so the output window will scroll as text
* is added if the cursor is at the end of the window.
*/
void ConsoleView::AddText( const char *msg ) {

	idStr work;

	work = msg;
	work.RemoveColors();

	auto lines = editConsole->GetTextLines();

	if ( lines.size() > limitConsoleLines ) {
		int numDelete = lines.size() - limitConsoleLines;
		lines.erase( lines.begin(), lines.begin() + numDelete );
	}
	for ( int i = 0; i < work.Length(); ) {
		int j = work.Find('\n', i);
		if (j == -1) {
			break;
		}
		idStr line;
		work.Mid(i, j - i, line);

		lines.push_back( std::string( line.c_str() ) );
		i = j + 1;
	}	

	TextEditor::Coordinates sel = TextEditor::Coordinates( (int)lines.size() - 1, 0 );
	editConsole->SetTextLines( lines );
	editConsole->SetCursorPosition( sel );
}

/**
* Replaces the text in the console window with the specified text.
* @param text The text to place in the console window.
*/
void ConsoleView::SetConsoleText ( const idStr &text ) {
	editInput = text;
}

/**
* Executes the specified console command. If the command is passed
* as a parameter then it is executed otherwise the command in the
* input box is executed.
* @param cmd The text to execute. If this string is empty then the
* input edit box text is used.
*/
void ConsoleView::ExecuteCommand ( const idStr &cmd ) {

	idStr str;
	if ( cmd.Length() > 0 ) {
		str = cmd;
	} else {
		str = editInput;
	}

	if ( str != "" ) {
		editInput.Clear();
		common->Printf( "%s\n", str.c_str() );

		// avoid adding multiple identical commands in a row
		int index = consoleHistory.Num();

		if ( index == 0 || str != consoleHistory[index-1]) {
			// keep the history to 16 commands, removing the oldest command
			if ( consoleHistory.Num () > 16 ) {
				consoleHistory.RemoveIndex ( 0 );
			}
			currentHistoryPosition = consoleHistory.Append( str );
		} else {
			currentHistoryPosition = consoleHistory.Num() - 1;
		}

		currentCommand.Clear ();

		bool propogateCommand = true;

		// process some of our own special commands
		if ( str.Icmp( "clear" ) == 0 ) {
			editConsole->SetText( std::string( "" ) );
		}
		else if ( str.Icmp ( "edit" ) == 0) {
			propogateCommand = false;
		}
		if ( propogateCommand ) {
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, str );
		}
	}
}

void ConsoleView::Draw( const ImVec2 &pos, const ImVec2 &size ) {
	editConsole->Render( "###Lines", ImVec2( size.x, size.y-EDIT_HEIGHT ) );
	if ( ImGui::InputTextStr( "###Input", &editInput, ImGuiInputTextFlags_ElideLeft ) ) {
	}
	ImGui::SetItemDefaultFocus();
	ImGui::SetItemKeyOwner( ImGuiKey_Enter );
	ImGui::SetItemKeyOwner( ImGuiKey_UpArrow );
	ImGui::SetItemKeyOwner( ImGuiKey_DownArrow );
	ImGui::SetItemKeyOwner( ImGuiKey_Tab );
	ImGui::SetItemKeyOwner( ImGuiKey_PageUp );
	ImGui::SetItemKeyOwner( ImGuiKey_PageDown );

	// keep auto focus on the input box
	if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1);

	PreTranslateMessage();
	if ( selectEditInput ) {
		selectEditInput = false;
	}
	if ( ImGui::IsWindowFocused() ) {
		MaterialEditorSetActiveWindow( ME_WINDOW_CONSOLE );
	}
}

/**
* Handles keyboard input to process the "Enter" key to execute
* commands and command history.
*/
bool ConsoleView::PreTranslateMessage() {
	int timeEnd = Sys_Milliseconds();
	int elapsed = timeEnd - editInputLastKeyDownTime;
	int keydownTime = 200;

	if ( ImGui::IsKeyDown( ImGuiKey_Enter ) ) {
		if ( elapsed > keydownTime ) {
			this->ExecuteCommand();
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}

	if ( ImGui::IsKeyDown( ImGuiKey_UpArrow ) ) {
		if ( elapsed > keydownTime ) {
			// save off the current in-progress command so we can get back to it
			if ( saveCurrentCommand == true ) {
				currentCommand = editInput;
				saveCurrentCommand = false;
			}

			if ( consoleHistory.Num () > 0 ) {
				editInput = consoleHistory[currentHistoryPosition];

				selectEditInput = true;
			}

			if ( currentHistoryPosition > 0 ) {
				--currentHistoryPosition;
			}
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}

	if ( ImGui::IsKeyDown( ImGuiKey_DownArrow ) ) {
		if ( elapsed > keydownTime ) {
			int selLocation = 0;
			if ( currentHistoryPosition < consoleHistory.Num () - 1 ) {
				++currentHistoryPosition;
				editInput = consoleHistory[currentHistoryPosition];
				selLocation = consoleHistory[currentHistoryPosition].Length();
			}
			else {
				editInput = currentCommand;
				selLocation = currentCommand.Length();
				currentCommand.Clear ();
				saveCurrentCommand = true;
			}

			selectEditInput = true;
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}
	if ( ImGui::IsKeyDown( ImGuiKey_Tab ) ) {
		if ( elapsed > keydownTime ) {
			common->Printf ( "Command History\n----------------\n" );
			for ( int i = 0 ; i < consoleHistory.Num ();i++ )
			{
				common->Printf ( "[cmd %d]:  %s\n" , i , consoleHistory[i].c_str() );
			}
			common->Printf ( "----------------\n" );
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}
	if ( ImGui::IsKeyDown( ImGuiKey_PageDown ) ) {
		if ( elapsed > keydownTime ) {
			editConsole->MoveDown( 10 );
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}

	if ( ImGui::IsKeyDown( ImGuiKey_PageUp ) ) {
		if ( elapsed > keydownTime ) {
			editConsole->MoveUp( 10 );
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}

	if ( ImGui::IsKeyDown( ImGuiKey_Home ) ) {
		if ( elapsed > keydownTime ) {
			editConsole->MoveTop();
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}

	if ( ImGui::IsKeyDown( ImGuiKey_End ) ) {
		if ( elapsed > keydownTime ) {
			editConsole->MoveBottom();
			editInputLastKeyDownTime = timeEnd;
		}
		return true;
	}

	return false;
}

}
