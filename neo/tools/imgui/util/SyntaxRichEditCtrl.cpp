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

#include "SyntaxRichEditCtrl.h"

#include "framework/FileSystem.h"
#include "framework/Game.h"

#include "sys/sys_imgui.h"

namespace ImGuiTools {

const int MOUSEWHEEL_DELTA				= 120;

const int AUTOCOMPLETE_WIDTH			= 200;
const int AUTOCOMPLETE_HEIGHT			= 180;
const int AUTOCOMPLETE_OFFSET			= 16;

const int FUNCPARMTOOLTIP_WIDTH			= 16;
const int FUNCPARMTOOLTIP_HEIGHT		= 20;
const int FUNCPARMTOOLTIP_OFFSET		= 16;
/*
const COLORREF DEFAULT_BACK_COLOR				= SRE_COLOR_WHITE;
const COLORREF INVALID_BACK_COLOR				= SRE_COLOR_WHITE - 2;
const COLORREF MULTILINE_COMMENT_BACK_COLOR		= SRE_COLOR_WHITE - 1;
*/
//#define IDC_LISTBOX_AUTOCOMPLETE		700
//#define IDC_EDITBOX_FUNCPARMS			701

void SyntaxRichEditCtrlKeyPress( void* data, bool ctrl, bool shift, bool alt, int chr ) {
	reinterpret_cast<SyntaxRichEditCtrl*>(data)->OnChar(ctrl, shift, alt, chr);
}
bool SyntaxRichEditCtrlKeyDown( void *data ) {
	return reinterpret_cast<SyntaxRichEditCtrl*>(data)->OnKeyDown();
}
bool SyntaxRichEditMouseButtonDown( void *data ) {
	return reinterpret_cast<SyntaxRichEditCtrl*>(data)->OnMouseButtonDown();
}
bool SyntaxRichEditToolTipNotify( void *data, const char *ident, char *output, size_t outputLength ) {
	return reinterpret_cast<SyntaxRichEditCtrl*>(data)->OnToolTipNotify( ident, output, outputLength );
}

static keyWord_t defaultKeyWords[] = {
	{ NULL, vec3_origin, "" }
};

/*
================
SyntaxRichEditCtrl::SyntaxRichEditCtrl
================
*/
SyntaxRichEditCtrl::SyntaxRichEditCtrl( void )
	: scriptEdit( NULL )
	, scriptEditPos( 0.0f, 0.0f )
	, scriptEditSize( 400.0f, 400.0f )
	, errorText()
	, findDlg()
	, gotoDlg()
	, msgBoxDlg()
	, findStr()
	, replaceStr()
	, matchCase( false )
	, matchWholeWords( false )
	, searchForward( true )
	, firstLine( 0 )
{
	//m_TextDoc = NULL;
	keyWords = defaultKeyWords;
	keyWordColors = NULL;
	keyWordLengths = NULL;
	caseSensitive = false;
	allowPathNames = true;
	keyWordAutoCompletion = true;
	//updateRange.cpMin = 0;
	//updateRange.cpMax = 0;
	updateSyntaxHighlighting = true;
	stringColorIndex = 0;
	stringColorLine = -1;
	autoCompleteStart = -1;
	autoCompleteListBoxSel = -1;
	autoCompleteLastKeyDownTime = 0;
	autoCompleteInput.Clear();
	autoCompleteListBoxPos = ImVec2( 0.0f, 0.0f );
	autoCompleteListBoxSize = ImVec2( 0.0f, 0.0f );
	funcParmToolTipStart = -1;
	funcParmToolTipPos = ImVec2( 0.0f, 0.0f );
	funcParmToolTipSize = ImVec2( 0.0f, 0.0f );
	GetObjectMembers = NULL;
	GetFunctionParms = NULL;
	GetToolTip = NULL;
}

/*
================
SyntaxRichEditCtrl::~SyntaxRichEditCtrl
================
*/
SyntaxRichEditCtrl::~SyntaxRichEditCtrl( void ) {
	FreeKeyWordsFromFile();
}

/*
================
SyntaxRichEditCtrl::SetCharType
================
*/
void SyntaxRichEditCtrl::SetCharType( int first, int last, int type ) {
	for ( int i = first; i <= last; i++ ) {
		charType[i] = type;
	}
}

/*
================
SyntaxRichEditCtrl::InitSyntaxHighlighting
================
*/
void SyntaxRichEditCtrl::InitSyntaxHighlighting( void ) {
	SetCharType( 0x00, 0xFF, CT_PUNCTUATION );
	SetCharType( '\0', ' ', CT_WHITESPACE );
	SetCharType( '/', '/', CT_COMMENT );
	SetCharType( '\"', '\"', CT_STRING );
	SetCharType( '\'', '\'', CT_LITERAL );
	SetCharType( 'a', 'z', CT_NAME );
	SetCharType( 'A', 'Z', CT_NAME );
	SetCharType( '_', '_', CT_NAME );
	SetCharType( '#', '#', CT_NAME );
	SetCharType( '0', '9', CT_NUMBER );
}

/*
================
SyntaxRichEditCtrl::Init
================
*/
void SyntaxRichEditCtrl::Init( void ) {
	scriptEdit = new TextEditor();
	scriptEdit->SetHandlers( this, SyntaxRichEditCtrlKeyPress, SyntaxRichEditCtrlKeyDown, SyntaxRichEditMouseButtonDown, SyntaxRichEditToolTipNotify );

	InitSyntaxHighlighting();

	SetFocus();

	// create auto complete list box
	autoCompleteListBoxPos = ImVec2( 0.0f, 0.0f );
	autoCompleteListBoxSize = ImVec2( AUTOCOMPLETE_WIDTH, AUTOCOMPLETE_HEIGHT );
	autoCompleteStart = -1;
	
	// create function parameter tool tip
	funcParmToolTipPos = ImVec2( 0.0f, 0.0f );
	funcParmToolTipSize = ImVec2( FUNCPARMTOOLTIP_WIDTH, FUNCPARMTOOLTIP_HEIGHT );
	funcParmToolTipStart = -1;
}

void SyntaxRichEditCtrl::Draw()
{
	bool clickedNew = false, clickedSelect = false;

	scriptEditPos = ImGui::GetCursorPos();
	scriptEditSize = ImVec2( 800, 600 );

	scriptEdit->Render( "Text", scriptEditSize, false );

	if ( ImGui::Button( "Go to" ) ) {
		OnEditGoToLine();
	}
	ImGui::SameLine();
	if ( ImGui::Button( "Find" ) ) {
		idStr selText = scriptEdit->GetSelectedText().c_str();

		findDlg.Start( selText, false );
	}
	ImGui::SameLine();
	if ( ImGui::Button( "Replace" ) ) {
		idStr selText = scriptEdit->GetSelectedText().c_str();

		findDlg.Start( selText, true );
	}

	if ( gotoDlg.Draw( scriptEditPos, scriptEditSize ) ) {
		TextEditor::Coordinates coords( gotoDlg.GetLine() - 1 - firstLine, 0 );

		scriptEdit->SetCursorPosition( coords );
		SetFocus();
	}
	FindReplaceDialog::command_t findReplaceResult = findDlg.Draw( scriptEditPos, scriptEditSize );
	OnFindDialogMessage( findReplaceResult );

	if ( msgBoxDlg.Draw( scriptEditPos, scriptEditSize ) ) {
		SetFocus();
	}

	if ( autoCompleteStart >= 0 ) {
		ImVec2 oldCursorPos = ImGui::GetCursorPos();
		ImVec2 newPos = ImVec2( scriptEditPos.x + autoCompleteListBoxPos.x, scriptEditPos.y + autoCompleteListBoxPos.y );

		ImGui::SetCursorPos( newPos );

		if ( ImGui::BeginChild( "###AutoCompleteListBox", autoCompleteListBoxSize, ImGuiChildFlags_Borders ) ) {
			for ( int i = 0; i < autoCompleteListBoxFiltered.Num(); i++ ) {
				bool selected = ( i == autoCompleteListBoxSel );

				ImGui::PushID( i );
				if ( ImGui::Selectable( autoCompleteListBox[autoCompleteListBoxFiltered[i]].c_str(), selected ) ) {
					// steal focus back from the auto-complete list box
					SetFocus();

					// insert text
					autoCompleteListBoxSel = i;
					AutoCompleteInsertText();
					AutoCompleteHide();
				}
				ImGui::SetItemKeyOwner( ImGuiKey_MouseWheelY );
				if ( selected ) {
					ImGui::SetItemDefaultFocus();
					ImGui::SetScrollHereY();
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
		ImGui::SetCursorPos( oldCursorPos );
	}
	if ( funcParmToolTipStart >= 0 ) {
		ImVec2 oldCursorPos = ImGui::GetCursorPos();
		ImVec2 newPos = ImVec2( scriptEditPos.x + funcParmToolTipPos.x, scriptEditPos.y + funcParmToolTipPos.y );

		ImGui::SetCursorPos( newPos );

		if ( ImGui::BeginChild( "###FuncParmToolTip", funcParmToolTipSize ) ) {
			ImGui::TextUnformatted( funcParmToolTip.c_str() );
		}
		ImGui::EndChild();
		ImGui::SetCursorPos( oldCursorPos );
	}

	ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "%s", errorText.c_str() );
}


/*
================
SyntaxRichEditCtrl::FindKeyWord
================
*/
ID_INLINE int SyntaxRichEditCtrl::FindKeyWord( const char *keyWord, int length ) const {
	int i, hash;

	if ( caseSensitive ) {
		hash = idStr::Hash( keyWord, length );
	} else {
		hash = idStr::IHash( keyWord, length );
	}
	for ( i = keyWordHash.First( hash ); i != -1; i = keyWordHash.Next( i ) ) {
		if ( length != keyWordLengths[i] ) {
			continue;
		}
		if ( caseSensitive ) {
			if ( idStr::Cmpn( keyWords[i].keyWord, keyWord, length ) != 0 ) {
				continue;
			}
		} else {
			if ( idStr::Icmpn( keyWords[i].keyWord, keyWord, length ) != 0 ) {
				continue;
			}
		}
		return i;
	}
	return -1;
}

/*
================
SyntaxRichEditCtrl::SetKeyWords
================
*/
void SyntaxRichEditCtrl::SetKeyWords( const keyWord_t kws[] ) {
	int i, numKeyWords, hash;

	keyWords = kws;

	for ( numKeyWords = 0; keyWords[numKeyWords].keyWord; numKeyWords++ ) {
	}

	delete keyWordColors;
	keyWordColors = new idVec3[numKeyWords];

	for ( i = 0; i < numKeyWords; i++ ) {
		keyWordColors[i] = keyWords[i].color;
	}

	delete keyWordLengths;
	keyWordLengths = new int[numKeyWords];

	for ( i = 0; i < numKeyWords; i++ ) {
		keyWordLengths[i] = idStr::Length( keyWords[i].keyWord );
	}

	keyWordHash.Clear( 1024, 1024 );
	for ( i = 0; i < numKeyWords; i++ ) {
		if ( caseSensitive ) {
			hash = idStr::Hash( keyWords[i].keyWord, keyWordLengths[i] );
		} else {
			hash = idStr::IHash( keyWords[i].keyWord, keyWordLengths[i] );
		}
		keyWordHash.Add( hash, i );
	}

	TextEditor::LanguageDefinition langDef;

	for ( i = 0; i < numKeyWords; i++ ) {
		if ( keyWords[i].description ) {
			TextEditor::Identifier id;
			id.mDeclaration = keyWords[i].description;
			langDef.mIdentifiers.insert( std::make_pair( std::string( keyWords[i].keyWord ), id ) );
		} else {
			langDef.mKeywords.insert( std::string( keyWords[i].keyWord ) );
		}
	}

	langDef.mTokenize = TextEditor::LanguageDefinition::CPlusPlus().mTokenize;

	langDef.mCommentStart = "/*";
	langDef.mCommentEnd = "*/";
	langDef.mSingleLineComment = "//";

	langDef.mCaseSensitive = caseSensitive;
	langDef.mAutoIndentation = true;

	langDef.mName = "doomscript"; // TODO: change?

	scriptEdit->SetLanguageDefinition( langDef );
}

/*
================
SyntaxRichEditCtrl::LoadKeyWordsFromFile
================
*/
bool SyntaxRichEditCtrl::LoadKeyWordsFromFile( const char *fileName ) {
	idParser src;
	idToken token, name, description;
	byte red, green, blue;
	keyWord_t keyword;

	if ( !src.LoadFile( fileName ) ) {
		return false;
	}

	FreeKeyWordsFromFile();

	while( src.ReadToken( &token ) ) {
		if ( token.Icmp( "keywords" ) == 0 ) {
			src.ExpectTokenString( "{" );
			while( src.ReadToken( &token ) ) {
				if ( token == "}" ) {
					break;
				}
				if ( token == "{" ) {

					// parse name
					src.ExpectTokenType( TT_STRING, 0, &name );
					src.ExpectTokenString( "," );

					// parse color
					src.ExpectTokenString( "(" );
					src.ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
					red = token.GetIntValue();
					src.ExpectTokenString( "," );
					src.ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
					green = token.GetIntValue();
					src.ExpectTokenString( "," );
					src.ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
					blue = token.GetIntValue();
					src.ExpectTokenString( ")" );
					src.ExpectTokenString( "," );

					// parse description
					src.ExpectTokenType( TT_STRING, 0, &description );
					src.ExpectTokenString( "}" );

					keyword.keyWord = Mem_CopyString( name );
					keyword.color = idVec3( red / 255.0f, green / 255.0f, blue / 255.0f );
					keyword.description = Mem_CopyString( description );

					keyWordsFromFile.Append( keyword );
				}
			}
		} else {
			src.SkipBracedSection();
		}
	}

	keyword.keyWord = NULL;
	keyword.color = idVec3( 1.0f, 1.0f, 1.0f );
	keyword.description = NULL;
	keyWordsFromFile.Append( keyword );

	SetKeyWords( keyWordsFromFile.Ptr() );

	return true;
}

/*
================
SyntaxRichEditCtrl::FreeKeyWordsFromFile
================
*/
void SyntaxRichEditCtrl::FreeKeyWordsFromFile( void ) {
	for ( int i = 0; i < keyWordsFromFile.Num(); i++ ) {
		Mem_Free( const_cast<char *>( keyWordsFromFile[i].keyWord ) );
		Mem_Free( const_cast<char *>( keyWordsFromFile[i].description ) );
	}
	keyWordsFromFile.Clear();
}

/*
================
SyntaxRichEditCtrl::SetStringColor
================
*/
void SyntaxRichEditCtrl::SetStringColor( const idVec3 &color, const idVec3 &altColor ) {
	stringColor[0] = color;
	if ( altColor == vec3_origin ) {
		stringColor[1] = color;
	} else {
		stringColor[1] = altColor;
	}
}

/*
================
SyntaxRichEditCtrl::SetObjectMemberCallback
================
*/
void SyntaxRichEditCtrl::SetObjectMemberCallback( objectMemberCallback_t callback ) {
	GetObjectMembers = callback;
}

/*
================
SyntaxRichEditCtrl::SetFunctionParmCallback
================
*/
void SyntaxRichEditCtrl::SetFunctionParmCallback( toolTipCallback_t callback ) {
	GetFunctionParms = callback;
}

/*
================
SyntaxRichEditCtrl::SetToolTipCallback
================
*/
void SyntaxRichEditCtrl::SetToolTipCallback( toolTipCallback_t callback ) {
	GetToolTip = callback;
}

/*
================
SyntaxRichEditCtrl::SetCaseSensitive
================
*/
void SyntaxRichEditCtrl::SetCaseSensitive( bool caseSensitive ) {
	this->caseSensitive = caseSensitive;
}

/*
================
SyntaxRichEditCtrl::AllowPathNames
================
*/
void SyntaxRichEditCtrl::AllowPathNames( bool allow ) {
	allowPathNames = allow;
}

/*
================
SyntaxRichEditCtrl::EnableKeyWordAutoCompletion
================
*/
void SyntaxRichEditCtrl::EnableKeyWordAutoCompletion( bool enable ) {
	keyWordAutoCompletion = enable;
}

/*
================
SyntaxRichEditCtrl::GetCursorPos
================
*/
void SyntaxRichEditCtrl::GetCursorPos( int &line, int &column, int &character ) const {
	TextEditor::Coordinates coords = scriptEdit->GetCursorPosition();
	line = coords.mLine;
	column = coords.mColumn;
	character = 0;
}

/*
================
SyntaxRichEditCtrl::GetText
================
*/
void SyntaxRichEditCtrl::GetText( idStr &text ) const {
	text = scriptEdit->GetText().c_str();
}

/*
================
SyntaxRichEditCtrl::SetText
================
*/
void SyntaxRichEditCtrl::SetText( const char *text ) {
	scriptEdit->SetText( std::string( text ) );
}

/*
================
SyntaxRichEditCtrl::SetReadOnly
================
*/
void SyntaxRichEditCtrl::SetReadOnly( bool readOnly ) {
	scriptEdit->SetReadOnly( readOnly );
}

/*
================
SyntaxRichEditCtrl::GetReadOnly
================
*/
bool SyntaxRichEditCtrl::GetReadOnly() {
	return scriptEdit->IsReadOnly();
}

/*
================
SyntaxRichEditCtrl::IsEdited
================
*/
bool SyntaxRichEditCtrl::IsEdited() const {
	return scriptEdit->CanUndo();
}

bool SyntaxRichEditCtrl::CanCopy() {
	return scriptEdit->HasSelection();
}

void SyntaxRichEditCtrl::Copy() {
	scriptEdit->Copy();
}

bool SyntaxRichEditCtrl::CanCut() {
	return !scriptEdit->IsReadOnly() && scriptEdit->HasSelection();
}

void SyntaxRichEditCtrl::Cut() {
	scriptEdit->Cut();
}

bool SyntaxRichEditCtrl::CanPaste() {
	const char *clipboardText = ImGui::GetClipboardText();

	return !scriptEdit->IsReadOnly() && clipboardText && *clipboardText;
}

void SyntaxRichEditCtrl::Paste() {
	scriptEdit->Paste();
}

bool SyntaxRichEditCtrl::CanUndo() {
	return scriptEdit->CanUndo();
}

void SyntaxRichEditCtrl::Undo() {
	scriptEdit->Undo();
}

bool SyntaxRichEditCtrl::CanRedo() {
	return scriptEdit->CanRedo();
}

void SyntaxRichEditCtrl::Redo() {
	scriptEdit->Redo();
}

bool SyntaxRichEditCtrl::CanDelete() {
	return !scriptEdit->IsReadOnly();
}

void SyntaxRichEditCtrl::Delete() {
	scriptEdit->Delete();
}



/*
================
SyntaxRichEditCtrl::SetFocus
================
*/
void SyntaxRichEditCtrl::SetFocus() {
	scriptEdit->Focus();
}

/*
================
SyntaxRichEditCtrl::AutoCompleteInsertText
================
*/
void SyntaxRichEditCtrl::AutoCompleteInsertText( void ) {
	TextEditor::Coordinates sel;
	int index;

	index = autoCompleteListBoxSel;
	if ( index >= 0 && index < autoCompleteListBoxFiltered.Num() ) {
		sel = scriptEdit->GetCursorPosition();
		idStr text = autoCompleteListBox[autoCompleteListBoxFiltered[index]];

		TextEditor::Coordinates autoCompleteCoords = TextEditor::Coordinates( sel.mLine, autoCompleteStart );
		scriptEdit->SetSelection( autoCompleteCoords, sel );
		scriptEdit->ReplaceSelection( text.c_str() );
	}
}

/*
================
SyntaxRichEditCtrl::AutoCompleteUpdate
================
*/
void SyntaxRichEditCtrl::AutoCompleteUpdate() {
	TextEditor::Coordinates sel, autoCompleteCoords;
	int index = -1;

	sel = scriptEdit->GetCursorPosition();

	autoCompleteCoords = TextEditor::Coordinates( sel.mLine, autoCompleteStart );
	autoCompleteInput = scriptEdit->GetText( autoCompleteCoords, sel ).c_str();

	int i, num;

	num = autoCompleteListBox.Num();
	autoCompleteListBoxFiltered.Clear();
	for( i = 0; i < num; i++ ) {
		if ( !autoCompleteInput.Length() || autoCompleteListBox[ i ].Find( autoCompleteInput.c_str(), false ) >= 0 ) {
			autoCompleteListBoxFiltered.Append(i);
			if ( index == -1 ) {
				index = autoCompleteListBoxFiltered.Num() - 1;
			}
		}
	}

	autoCompleteListBoxSel = index;
}

/*
================
SyntaxRichEditCtrl::AutoCompleteShow
================
*/
void SyntaxRichEditCtrl::AutoCompleteShow( int columnIndex ) {
	ImVec2 point;
	float top, left;

	if ( !autoCompleteListBox.Num() ) {
		return;
	}

	autoCompleteStart = columnIndex;
	point = scriptEdit->GetCursorScreenCoordinates();
	
	if ( point.y + AUTOCOMPLETE_HEIGHT + AUTOCOMPLETE_OFFSET < scriptEditSize.y ) {
		top = point.y + AUTOCOMPLETE_OFFSET;
	} else {
		top = point.y - AUTOCOMPLETE_HEIGHT - AUTOCOMPLETE_OFFSET;
	}
	left = point.x;
	autoCompleteListBoxPos = ImVec2( left, top );
	autoCompleteListBoxSize = ImVec2( AUTOCOMPLETE_WIDTH, AUTOCOMPLETE_HEIGHT );
	
	AutoCompleteUpdate();
}

/*
================
SyntaxRichEditCtrl::AutoCompleteHide
================
*/
void SyntaxRichEditCtrl::AutoCompleteHide( void ) {
	autoCompleteStart = -1;
	autoCompleteInput.Clear();
}

/*
================
SyntaxRichEditCtrl::ToolTipShow
================
*/
void SyntaxRichEditCtrl::ToolTipShow( int charIndex, const char *string ) {
	ImVec2 point;
	float top, left;

	funcParmToolTipStart = charIndex;
	funcParmToolTip = string;
	// set tooltip size based on text size
	float sizeX = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, funcParmToolTip.c_str(), nullptr, nullptr).x;
	float sizeY = Min( (float)FUNCPARMTOOLTIP_HEIGHT, ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).y );
	point = scriptEdit->GetCursorScreenCoordinates();

	if ( point.y + sizeY + FUNCPARMTOOLTIP_OFFSET < scriptEditSize.y ) {
		top = point.y + FUNCPARMTOOLTIP_OFFSET;
	} else {
		top = point.y - sizeY - FUNCPARMTOOLTIP_OFFSET;
	}
	left = point.x;
	funcParmToolTipPos = ImVec2( left, top );
	funcParmToolTipSize = ImVec2( sizeX, sizeY );
}

/*
================
SyntaxRichEditCtrl::ToolTipHide
================
*/
void SyntaxRichEditCtrl::ToolTipHide( void ) {
	funcParmToolTipStart = -1;
}

/*
================
SyntaxRichEditCtrl::GetNameBeforeCurrentSelection
================
*/
bool SyntaxRichEditCtrl::GetNameBeforeCurrentSelection( idStr &name, int &charIndex ) const {
	idStr buffer = scriptEdit->GetWordBeforeCursor().c_str();

	if ( buffer.Length() > 0 ) {
		name = buffer;
		charIndex = scriptEdit->GetCursorPosition().mColumn;
		return true;
	}

	return false;
}

/*
================
SyntaxRichEditCtrl::OnToolTipNotify
================
*/
bool SyntaxRichEditCtrl::OnToolTipNotify( const char *ident, char *toolTip, size_t toolTipSize ) {
	idStr str;
	idStr name = ident;

	toolTip[0] = '\0';
	if ( GetToolTip == NULL || !GetToolTip( name.c_str(), str)) {

		int keyWordIndex = FindKeyWord( name.c_str(), name.Length() );

		if ( keyWordIndex != -1 && keyWords[keyWordIndex].description[0] != '\0' ) {
			str = keyWords[keyWordIndex].description;
		} else {
			str = name.c_str();
		}
	}
	idStr::Copynz( toolTip, str.c_str(), toolTipSize );

	return str.Length() > 0;
}

/*
================
SyntaxRichEditCtrl::OnEditGoToLine
================
*/
void SyntaxRichEditCtrl::OnEditGoToLine() {
	TextEditor::Coordinates coords = scriptEdit->GetCursorPosition();

	gotoDlg.Start( firstLine + 1, firstLine + scriptEdit->GetTotalLines(), coords.mLine + 1 );
}

/*
================
SyntaxRichEditCtrl::OnEditFindNext
================
*/
void SyntaxRichEditCtrl::OnEditFindNext() {
	if ( scriptEdit->FindNext( findStr.c_str(), matchCase, matchWholeWords, searchForward ) ) {
		SetFocus();
	} else {
		msgBoxDlg.Start( "The specified text was not found.", false, false );
	}

}

/*
================
SyntaxRichEditCtrl::OnFindDialogMessage
================
*/
void SyntaxRichEditCtrl::OnFindDialogMessage( FindReplaceDialog::command_t command ) {
	switch ( command ) {
	case FindReplaceDialog::command_t::FIND_NEXT:
	case FindReplaceDialog::command_t::FIND_PREV:
	{
		findStr = findDlg.GetFindString();
		matchCase = findDlg.MatchCase();
		matchWholeWords = findDlg.MatchWholeWord();
		searchForward = ( command == FindReplaceDialog::command_t::FIND_NEXT );
		OnEditFindNext();
		break;
	}
	case FindReplaceDialog::command_t::FIND_ALL:
		break;
	case FindReplaceDialog::command_t::REPLACE_NEXT:
	{
		replaceStr = findDlg.GetReplaceString();
		idStr selection = scriptEdit->GetSelectedText().c_str();

		if ( selection.Length() && selection == findStr ) {
			scriptEdit->ReplaceSelection( replaceStr.c_str() );
		}

		findStr = findDlg.GetFindString();
		matchCase = findDlg.MatchCase();
		matchWholeWords = findDlg.MatchWholeWord();
		searchForward = true;
		OnEditFindNext();
		break;
	}
	case FindReplaceDialog::command_t::REPLACE_ALL:
	{
		replaceStr = findDlg.GetReplaceString();
		findStr = findDlg.GetFindString();
		matchCase = findDlg.MatchCase();
		matchWholeWords = findDlg.MatchWholeWord();

		int numReplaces = scriptEdit->ReplaceAll( findStr.c_str(), replaceStr.c_str(), matchCase, matchWholeWords );
		if (numReplaces == 0) {
			msgBoxDlg.Start( "The specified text was not found.", false, false );
		} else {
			msgBoxDlg.Start( va( "Replaced %d occurances.", numReplaces ), false, false );
		}
		break;
	}
	case FindReplaceDialog::command_t::DONE:
		SetFocus();
		break;
	case FindReplaceDialog::command_t::NONE:
	default:
		break;
	}
}

/*
================
SyntaxRichEditCtrl::OnKeyDown
================
*/
bool SyntaxRichEditCtrl::OnKeyDown() {

	if ( autoCompleteStart >= 0 ) {
		int timeEnd = Sys_Milliseconds();
		int elapsed = timeEnd - autoCompleteLastKeyDownTime;
		int sel;
		int keydownTime = 200;

		if ( ImGui::IsKeyDown( ImGuiKey_UpArrow ) ) {
			if ( elapsed > keydownTime ) {
				sel = Max( 0, autoCompleteListBoxSel - 1 );
				autoCompleteListBoxSel = sel;
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_DownArrow ) ) {
			if ( elapsed > keydownTime ) {
				sel = Min( autoCompleteListBoxFiltered.Num() - 1, autoCompleteListBoxSel + 1 );
				autoCompleteListBoxSel = sel;
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_PageUp ) ) {
			if ( elapsed > keydownTime ) {
				sel = Max( 0, autoCompleteListBoxSel - 10 );
				autoCompleteListBoxSel = sel;
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_PageDown ) ) {
			if ( elapsed > keydownTime ) {
				sel = Min( autoCompleteListBoxFiltered.Num() - 1, autoCompleteListBoxSel + 10 );
				autoCompleteListBoxSel = sel;
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_Home ) ) {
			if ( elapsed > keydownTime ) {
				autoCompleteListBoxSel = 0;
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_End ) ) {
			if ( elapsed > keydownTime ) {
				autoCompleteListBoxSel = autoCompleteListBoxFiltered.Num() - 1;
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_Enter ) || ImGui::IsKeyDown( ImGuiKey_Tab ) ) {
			if ( elapsed > keydownTime ) {
				AutoCompleteInsertText();
				AutoCompleteHide();
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_Escape ) ) {
			if ( elapsed > keydownTime ) {
				AutoCompleteHide();
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
		if ( ImGui::IsKeyDown( ImGuiKey_LeftArrow )
			|| ImGui::IsKeyDown( ImGuiKey_RightArrow )
			|| ImGui::IsKeyDown( ImGuiKey_Insert )
			|| ImGui::IsKeyDown( ImGuiKey_Delete ) ) {
			if ( elapsed > keydownTime ) {
				autoCompleteLastKeyDownTime = timeEnd;
			}
			return true;
		}
	}

	if ( ImGui::IsKeyChordPressed( ImGuiKey_ModCtrl | ImGuiKey_G ) ) {
		OnEditGoToLine();

		return true;
	} else if ( ImGui::IsKeyChordPressed( ImGuiKey_ModCtrl | ImGuiKey_F ) ) {
		idStr selText = scriptEdit->GetSelectedText().c_str();

		findDlg.Start( selText, false );

		return true;
	} else if ( ImGui::IsKeyChordPressed( ImGuiKey_ModCtrl | ImGuiKey_H ) ) {
		idStr selText = scriptEdit->GetSelectedText().c_str();

		findDlg.Start( selText, true );

		return true;
	}

	return false;
}

/*
================
SyntaxRichEditCtrl::OnChar
================
*/
void SyntaxRichEditCtrl::OnChar( bool ctrl, bool shift, bool alt, int nChar ) {
	// if the auto-complete list box is up
	if ( autoCompleteStart >= 0 ) {
		TextEditor::Coordinates sel;

		if ( charType[nChar] == CT_NAME ) {
			AutoCompleteUpdate();
			return;
		} else if ( nChar == 0x08/*backspace*/) {
			sel = scriptEdit->GetCursorPosition();
			if ( sel.mColumn > autoCompleteStart ) {
				AutoCompleteUpdate();
			} else {
				AutoCompleteHide();
			}
			return;
		} else {
			AutoCompleteHide();
		}
	}

	// if the function parameter tool tip is up
	if ( funcParmToolTipStart >= 0 ) {
		TextEditor::Coordinates sel;

		if ( nChar == ')' || nChar == 0x1b/*escape*/ ) {
			ToolTipHide();
		} else if ( nChar == 0x08/*backspace*/ ) {
			sel = scriptEdit->GetCursorPosition();
			if ( sel.mColumn < funcParmToolTipStart ) {
				ToolTipHide();
			}
		}
	}

	// show keyword auto-completion
	if ( keyWordAutoCompletion && charType[nChar] == CT_NAME && funcParmToolTipStart < 0 ) {
		TextEditor::Coordinates sel;
		int line, column, length, i;

		sel = scriptEdit->GetCursorPosition();
		
		line = sel.mLine;
		column = sel.mColumn;
		length = scriptEdit->GetCurrentLineText().length();
		
		if ( column <= 1 || scriptEdit->PeekLeftIsWhiteSpace( 2 ) ) {
			if ( column >= length-1 || scriptEdit->PeekLeftIsWhiteSpace( 0 ) ) {

				autoCompleteListBox.Clear();
				for ( i = 0; keyWords[i].keyWord; i++ ) {
					autoCompleteListBox.Append( keyWords[i].keyWord );
				}
				autoCompleteListBox.Sort();
				AutoCompleteShow( sel.mColumn + 1 );
			}
		}
		return;
	}

	// show object member auto-completion
	if ( nChar == '.' && GetObjectMembers && funcParmToolTipStart < 0 ) {
		int charIndex;
		idStr name;

		if ( GetNameBeforeCurrentSelection( name, charIndex ) ) {
			autoCompleteListBox.Clear();
			if ( GetObjectMembers( name, autoCompleteListBox ) ) {
				AutoCompleteShow( charIndex );
			}
		}
		return;
	}
	
	// show function parameter tool tip
	if ( nChar == '(' && GetFunctionParms ) {
		int charIndex;
		idStr name;

		if ( GetNameBeforeCurrentSelection( name, charIndex ) ) {
			idStr parmString;
			if ( GetFunctionParms( name, parmString ) ) {
				ToolTipShow( charIndex, parmString );
			}
		}
		return;
	}
}

/*
================
SyntaxRichEditCtrl::OnMouseButtonDown
================
*/
bool SyntaxRichEditCtrl::OnMouseButtonDown() {

	if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && autoCompleteStart >= 0 ) {
		AutoCompleteHide();
		return true;
	}

	return false;
}

/*
================
SyntaxRichEditCtrl::OnMouseWheel
================
*/
void SyntaxRichEditCtrl::OnMouseWheel( float wheel ) {
	if ( autoCompleteStart >= 0 ) {
		int sel;

		if ( wheel > 0  ) {
			sel = Max( 0, autoCompleteListBoxSel - (int)( wheel / MOUSEWHEEL_DELTA ) );
		} else {
			sel = Min( autoCompleteListBoxFiltered.Num() - 1, autoCompleteListBoxSel - (int)( wheel / MOUSEWHEEL_DELTA ) );
		}
		autoCompleteListBoxSel = sel;
		return;
	}
}

}
