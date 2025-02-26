/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

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

#pragma once

#include "sys/sys_imgui.h"

#include "idlib/Str.h"
#include "idlib/containers/List.h"
#include "idlib/math/Vector.h"

#include "framework/Game.h"

namespace ImGuiTools
{

/**
* Holds a list of booleans that correspond to a list of names. This
* class owns the list of bools, not the list of names.
*/
class MultiSelectWidget
{
public:
	MultiSelectWidget( const char** aNames, int* contentMapping, int aNumEntries );
	~MultiSelectWidget();

	void Update( int index, bool value );

	void UpdateWithBitFlags( int bitFlags );

public:
	const char**	names;
	int*			contentMapping;
	int				numEntries;
	bool*			selectables;
};

bool DoMultiSelect( MultiSelectWidget* widget, int* contents );

void HelpMarker( const char* desc );

bool StringListItemGetter( void* data, int index, const char** outText );

MultiSelectWidget MakePhysicsContentsSelector();

class ColorPicker {
public:
				ColorPicker( const char *label );

	bool		Button( const idVec4 &_color );
	bool		Draw();

	ID_INLINE const idVec4&		GetColor() { return color; }
	ID_INLINE void				SetColor( idVec4 &c ) {
		color = oldColor = c;
	}

private:
	const char *		label;
	idVec4				color;
	idVec4				oldColor;
};

class DeclNewSelect {
public:
						DeclNewSelect( declType_t declType, const char *directory, const char *extension, const char *label );

	void				Start();
	bool				Draw();

	ID_INLINE idDecl* GetDecl() { return dp; }

private:
	enum state_t { DONE = 0, NAME };

	declType_t			declType;
	const char *		directory;
	const char *		extension;
	const char *		label;
	int					fileSelection;
	idList<idStr>		files;
	idStr				fileName;
	idStr				name;
	idStr				errorText;
	idDecl *			dp;
	state_t				state;
};

class DeclSelect {
public:
						DeclSelect( declType_t declType, const char *label );

	void				Start( const char *name );
	bool				Draw();

	ID_INLINE idDecl*	GetDecl() { return dp; }

private:
	enum state_t { DONE = 0, NAME };

	declType_t			declType;
	const char *		label;
	int					listSel;
	idList<idStr>		list;
	idStr				name;
	idStr				errorText;
	idDecl *			dp;
	state_t				state;
};

class GoToLineDialog {
public:
						GoToLineDialog();

	void				Start( int firstLine, int lastLine, int line );
	ID_INLINE int		GetLine() const { return numberEdit; }
	bool				Draw( const ImVec2 &pos, const ImVec2 &size );

private:

	int					numberEdit;
	int					firstLine;
	int					lastLine;
	bool				waiting;
	bool				valid;
	bool				focus;
	idStr				caption;
};

class FindReplaceDialog {
public:
								FindReplaceDialog();

	enum command_t { NONE, DONE, FIND_NEXT, FIND_PREV, FIND_ALL, REPLACE_NEXT, REPLACE_ALL };

	void						Start( idStr &selection, bool replacement );
	command_t					Draw( const ImVec2 &pos, const ImVec2 &size );

	ID_INLINE const idStr &	GetReplaceString() const { return replace; };		// get replacement string
	ID_INLINE const idStr &	GetFindString() const { return find; };				// get find string
	ID_INLINE bool			MatchCase() const { return matchCase; };			// true if matching case
	ID_INLINE bool			MatchWholeWord() const { return matchWhole; };		// true if matching whole words only

private:
	idStr					replace;
	idStr					find;
	bool					matchCase;
	bool					matchWhole;
	bool					replacement;
	bool					valid;
	bool					visible;
	bool					focus;
};

class MessageBoxDialog {
public:
							MessageBoxDialog();

	void					Start( const char *message, bool choice, bool error );
	bool					Draw( const ImVec2 &pos, const ImVec2 &size );

	ID_INLINE bool			Result() const { return acked; };

private:
	idStr					message;
	bool					choice;
	bool					error;
	bool					visible;
	bool					acked;
	bool					focus;
};

} //namespace ImGuiTools


// #####################################
// Some ImGui Widgets (functions) that
//   we put in the ImGui namespace
// #####################################

class idVec3;

namespace ImGui {

bool DragVec3( const char* label, idVec3& v, float v_speed = 1.0f,
			   float v_min = 0.0f, float v_max = 0.0f,
			   const char* display_format = "%.1f",
			   float power = 1.0f, bool ignoreLabelWidth = true );

bool DragVec3fitLabel( const char* label, idVec3& v, float v_speed = 1.0f,
					   float v_min = 0.0f, float v_max = 0.0f,
					   const char* display_format = "%.1f", float power = 1.0f );


bool InputTextStr( const char* label, idStr* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr );

bool InputTextMultilineStr( const char* label, idStr* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr );

bool InputDialogName( const char *text, const char *label, idStr *str );

bool InputMessageBox( const char *text, const char* label, bool allowCancel = false );

bool ToggleButton( const char *label, bool *value, const ImVec2 &size );

} //namespace ImGui
