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

#ifndef __SYNTAXRICHEDITCTR_H__
#define __SYNTAXRICHEDITCTR_H__

/*
===============================================================================

	Rich Edit Control with:

	- syntax highlighting
	- braced section highlighting
	- braced section auto-indentation
	- multi-line tabs
	- keyword auto-completion
	- object member auto-completion
	- keyword tool tip
	- function parameter tool tip

===============================================================================
*/

#include "../../edit_public.h"

#include "idlib/containers/StrList.h"

class TextEditor;

namespace ImGuiTools {

static const int		TAB_SIZE = 4;

static const idVec3		SRE_COLOR_BLACK	= idVec3(0.0f, 0.0f, 0.0f);
static const idVec3		SRE_COLOR_WHITE = idVec3(1.0f, 1.0f, 1.0f);
static const idVec3		SRE_COLOR_RED = idVec3(1.0f, 0.0f, 0.0f);
static const idVec3		SRE_COLOR_GREEN = idVec3(0.0f, 1.0f, 0.0f);
static const idVec3		SRE_COLOR_BLUE = idVec3(0.0f, 0.0f, 1.0f);
static const idVec3		SRE_COLOR_YELLOW = idVec3(1.0f, 1.0f, 0.0f);
static const idVec3		SRE_COLOR_MAGENTA = idVec3(1.0f, 0.0f, 1.0f);
static const idVec3		SRE_COLOR_CYAN = idVec3(0.0f, 1.0f, 1.0f);
static const idVec3		SRE_COLOR_ORANGE = idVec3(1.0f, 0.5f, 0.0f);
static const idVec3		SRE_COLOR_PURPLE = idVec3(0.59f, 0.0f, 0.59f);
static const idVec3		SRE_COLOR_PINK = idVec3(0.73f, 0.4f, 0.48f);
static const idVec3		SRE_COLOR_GREY = idVec3(0.33f, 0.33f, 0.33f);
static const idVec3		SRE_COLOR_BROWN = idVec3(0.4f, 0.35f, 0.07f);
static const idVec3		SRE_COLOR_LIGHT_GREY = idVec3(0.66f, 0.66f, 0.66f);
static const idVec3		SRE_COLOR_LIGHT_BROWN = idVec3(0.66f, 0.59f, 0.08f);
static const idVec3		SRE_COLOR_DARK_GREEN = idVec3(0.0f, 0.5f, 0.0f);
static const idVec3		SRE_COLOR_DARK_CYAN = idVec3(0.0f, 0.59f, 0.59f);
static const idVec3		SRE_COLOR_DARK_YELLOW = idVec3(0.86f, 0.78f, 0.07f);

typedef struct {
	const char *	keyWord;
	idVec3			color;
	const char *	description;
} keyWord_t;

typedef bool ( *objectMemberCallback_t )( const char* objectName, idStrList &listBox );
typedef bool ( *toolTipCallback_t )( const char* name, idStr &string );

class SyntaxRichEditCtrl {
public:
							SyntaxRichEditCtrl( void );
							~SyntaxRichEditCtrl( void );

	void					Init( void );
	void					Draw( void );

	void					SetCaseSensitive( bool caseSensitive );
	void					AllowPathNames( bool allow );
	void					EnableKeyWordAutoCompletion( bool enable );
	void					SetKeyWords( const keyWord_t kws[] );
	bool					LoadKeyWordsFromFile( const char *fileName );
	void					SetObjectMemberCallback( objectMemberCallback_t callback );
	void					SetFunctionParmCallback( toolTipCallback_t callback );
	void					SetToolTipCallback( toolTipCallback_t callback );

	void					SetStringColor( const idVec3 &color, const idVec3 &altColor = vec3_origin );

	void					GetCursorPos( int &line, int &column, int &character ) const;

	void					GetText( idStr &text ) const;
	void					SetText( const char *text );

	void					SetReadOnly( bool readOnly );
	bool					GetReadOnly();

	bool					CanCopy();
	void					Copy();
	bool					CanCut();
	void					Cut();
	bool					CanPaste();
	void					Paste();
	bool					CanUndo();
	void					Undo();
	bool					CanRedo();
	void					Redo();
	bool					CanDelete();
	void					Delete();

	bool					IsEdited() const;

	void					SetFocus();

public:
	bool					OnToolTipNotify( const char *ident, char *tooltip, size_t tooltipSize );
	void					OnChar( bool ctrl, bool shift, bool alt, int nChar );
	bool					OnKeyDown();
	bool					OnMouseButtonDown();
private:
	void					OnMouseWheel(float wheel);

	void					OnEditGoToLine();
	void					OnEditFindNext();
	void					OnFindDialogMessage( FindReplaceDialog::command_t command );

private:

	TextEditor *			scriptEdit;
	ImVec2					scriptEditPos;
	ImVec2					scriptEditSize;
	idStr					errorText;
	FindReplaceDialog		findDlg;
	GoToLineDialog			gotoDlg;
	MessageBoxDialog		msgBoxDlg;
	idStr					findStr;
	idStr					replaceStr;
	bool					matchCase;
	bool					matchWholeWords;
	bool					searchForward;
	int						firstLine;

	// settings
	//CHARFORMAT2				defaultCharFormat;
	idVec3					defaultColor;
	idVec3					singleLineCommentColor;
	idVec3					multiLineCommentColor;
	idVec3					stringColor[2];
	idVec3					literalColor;
	idVec3					braceHighlightColor;

	typedef enum {
		CT_WHITESPACE,
		CT_COMMENT,
		CT_STRING,
		CT_LITERAL,
		CT_NUMBER,
		CT_NAME,
		CT_PUNCTUATION
	} charType_t;

	int						charType[256];

	idList<keyWord_t>		keyWordsFromFile;
	const keyWord_t *		keyWords;
	int *					keyWordLengths;
	idVec3 *				keyWordColors;
	idHashIndex				keyWordHash;

	bool					caseSensitive;
	bool					allowPathNames;
	bool					keyWordAutoCompletion;

	objectMemberCallback_t	GetObjectMembers;
	toolTipCallback_t		GetFunctionParms;
	toolTipCallback_t		GetToolTip;

	bool					updateSyntaxHighlighting;
	int						stringColorIndex;
	int						stringColorLine;

	int						autoCompleteStart;
	ImVec2					autoCompleteListBoxPos;
	ImVec2					autoCompleteListBoxSize;
	idStrList				autoCompleteListBox;
	idList<int>				autoCompleteListBoxFiltered;
	int						autoCompleteListBoxSel;
	idStr					autoCompleteInput;
	int						autoCompleteLastKeyDownTime;

	int						funcParmToolTipStart;
	ImVec2					funcParmToolTipPos;
	ImVec2					funcParmToolTipSize;
	idStr					funcParmToolTip;

private:
	void					InitSyntaxHighlighting( void );
	void					SetCharType( int first, int last, int type );

	void					FreeKeyWordsFromFile( void );
	int						FindKeyWord( const char *keyWord, int length ) const;

	bool					GetNameBeforeCurrentSelection( idStr &name, int &charIndex ) const;

	void					AutoCompleteInsertText( void );
	void					AutoCompleteUpdate( void );
	void					AutoCompleteShow( int columnIndex );
	void					AutoCompleteHide( void );

	void					ToolTipShow( int charIndex, const char *string );
	void					ToolTipHide( void );
};

}

#endif /* !__SYNTAXRICHEDITCTR_H__ */
