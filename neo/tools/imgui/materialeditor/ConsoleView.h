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
#ifndef CONSOLEVIEW_H_
#define CONSOLEVIEW_H_

#include "../util/ImGui_IdWidgets.h"
#include "sys/sys_imgui.h"

#include "MaterialEditor.h"
#include "../util/SyntaxRichEditCtrl.h"

class TextEditor;

namespace ImGuiTools {

/**
* View in the Material Editor that functions as a Doom III
* console. It allows users to view console output as well as issue
* console commands to the engine.
*/
class ConsoleView
{
public:

	TextEditor *		editConsole;
	idStr				editInput;

	idStr				consoleStr;
	idStrList			consoleHistory;
	idStr				currentCommand;
	int					currentHistoryPosition;
	bool				saveCurrentCommand;

public:
						ConsoleView();
						~ConsoleView();

	void				Init();

	// Public Operations
	void				AddText( const char* msg );
	void				SetConsoleText( const idStr &text );
	void				ExecuteCommand( const idStr &cmd = "" );

	void				Draw( const ImVec2 &pos, const ImVec2 &size );

private:
	int					limitConsoleLines;
	bool				selectEditInput;
	int					editInputLastKeyDownTime;

private:
	bool				PreTranslateMessage();
};

}

#endif /* !CONSOLEVIEW_H_ */
