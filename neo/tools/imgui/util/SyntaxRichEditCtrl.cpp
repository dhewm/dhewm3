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

#include "sys/sys_imgui.h"

namespace ImGuiTools {
	
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

bool SyntaxRichEditCtrlKeyPress( void* data, bool ctrl, bool shift, bool alt ) {
	return reinterpret_cast<SyntaxRichEditCtrl*>(data)->OnChar(ctrl, shift, alt);
}

static keyWord_t defaultKeyWords[] = {
	{ NULL, vec3_origin, "" }
};

/*
BEGIN_MESSAGE_MAP(CSyntaxRichEditCtrl, CRichEditCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(EN_PROTECTED, OnProtected)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_LBN_SELCANCEL(IDC_LISTBOX_AUTOCOMPLETE, OnAutoCompleteListBoxChange)
	ON_LBN_SELCHANGE(IDC_LISTBOX_AUTOCOMPLETE, OnAutoCompleteListBoxChange)
	ON_LBN_DBLCLK(IDC_LISTBOX_AUTOCOMPLETE, OnAutoCompleteListBoxDblClk)
END_MESSAGE_MAP()
*/

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
	funcParmToolTipStart = -1;
	bracedSection[0] = -1;
	bracedSection[1] = -1;
	GetObjectMembers = NULL;
	GetFunctionParms = NULL;
	GetToolTip = NULL;
	//mousePoint.x = 0;
	//mousePoint.y = 0;
	//keyWordToolTip = NULL;
	//m_pchTip = NULL;
	//m_pwchTip = NULL;
}

/*
================
SyntaxRichEditCtrl::~SyntaxRichEditCtrl
================
*/
SyntaxRichEditCtrl::~SyntaxRichEditCtrl( void ) {
	FreeKeyWordsFromFile();
	//delete m_pchTip;
	//delete m_pwchTip;
	//m_DefaultFont->Release();
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
	scriptEdit->SetKeyPress( this, SyntaxRichEditCtrlKeyPress );

	/*
	// get the Rich Edit ITextDocument to use the wonky TOM interface
	IRichEditOle *ire = GetIRichEditOle();
	IUnknown *iu = (IUnknown *)ire;
	if ( iu == NULL || iu->QueryInterface( tom::IID_ITextDocument, (void**) &m_TextDoc ) != S_OK ) {
		m_TextDoc = NULL;
	}
	*/

	InitSyntaxHighlighting();

	SetFocus();

	//SetEventMask( GetEventMask() | ENM_CHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_PROTECTED );	// ENM_SCROLLEVENTS

	//EnableToolTips( TRUE );
	/*
	// create auto complete list box
	CRect rect( 0, 0, AUTOCOMPLETE_WIDTH, AUTOCOMPLETE_HEIGHT );
	autoCompleteListBox.Create( WS_DLGFRAME | WS_VISIBLE | WS_VSCROLL | LBS_SORT | LBS_NOTIFY, rect, this, IDC_LISTBOX_AUTOCOMPLETE );
	autoCompleteListBox.SetFont( GetParent()->GetFont() );
	autoCompleteListBox.ShowWindow( FALSE );

	// create function parameter tool tip
	funcParmToolTip.Create( WS_VISIBLE | WS_BORDER, rect, this, IDC_EDITBOX_FUNCPARMS );
	funcParmToolTip.SetFont( GetParent()->GetFont() );
	funcParmToolTip.ShowWindow( FALSE );
	*/
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
		if ( keyWords[i].description != '\0' ) {
			TextEditor::Identifier id;
			id.mDeclaration = keyWords[i].description;
			langDef.mIdentifiers.insert( std::make_pair( std::string( keyWords[i].keyWord ), id ) );
		} else {
			langDef.mKeywords.insert( std::string( keyWords[i].keyWord ) );
		}
	}

	langDef.mTokenize = TextEditor::LanguageDefinition::C().mTokenize;

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
SyntaxRichEditCtrl::SetDefaultColor
================
*/
void SyntaxRichEditCtrl::SetDefaultColor( const idVec3 &color ) {
	defaultColor = color;
}

/*
================
SyntaxRichEditCtrl::SetCommentColor
================
*/
void SyntaxRichEditCtrl::SetCommentColor( const idVec3 &color ) {
	singleLineCommentColor = color;
	multiLineCommentColor = color;
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
SyntaxRichEditCtrl::SetLiteralColor
================
*/
void SyntaxRichEditCtrl::SetLiteralColor( const idVec3 &color ) {
	literalColor = color;
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
SyntaxRichEditCtrl::GetBackColor
================
*/
idVec3 SyntaxRichEditCtrl::GetBackColor( int charIndex ) const {
	/*tom::ITextRange* range;
	tom::ITextFont *font;
	long backColor;

	m_TextDoc->Range( charIndex, charIndex, &range );
	range->get_Font( &font );

	font->get_BackColor( &backColor );

	font->Release();
	range->Release();

	return backColor;*/
	return vec3_origin;
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
SyntaxRichEditCtrl::IsEdited
================
*/
bool SyntaxRichEditCtrl::IsEdited() const {
	return scriptEdit->CanUndo();
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
	/*long selStart, selEnd;
	int index;

	index = autoCompleteListBox.GetCurSel();
	if ( index >= 0 ) {
		CString text;
		autoCompleteListBox.GetText( index, text );
		GetSel( selStart, selEnd );
		selStart = autoCompleteStart;
		SetSel( selStart, selEnd );
		ReplaceSel( text, TRUE );
	}*/
}

/*
================
SyntaxRichEditCtrl::AutoCompleteUpdate
================
*/
void SyntaxRichEditCtrl::AutoCompleteUpdate( void ) {
	/*long selStart, selEnd;
	int index;
	idStr text;

	GetSel( selStart, selEnd );
	GetText( text, autoCompleteStart, selStart );
	index = autoCompleteListBox.FindString( -1, text );
	if ( index >= 0 && index < autoCompleteListBox.GetCount() ) {
		autoCompleteListBox.SetCurSel( index );
	}*/
}

/*
================
SyntaxRichEditCtrl::AutoCompleteShow
================
*/
void SyntaxRichEditCtrl::AutoCompleteShow( int charIndex ) {
	/*CPoint point;
	CRect rect;

	autoCompleteStart = charIndex;
	point = PosFromChar( charIndex );
	GetClientRect( rect );
	if ( point.y < rect.bottom - AUTOCOMPLETE_OFFSET - AUTOCOMPLETE_HEIGHT ) {
		rect.top = point.y + AUTOCOMPLETE_OFFSET;
		rect.bottom = point.y + AUTOCOMPLETE_OFFSET + AUTOCOMPLETE_HEIGHT;
	} else {
		rect.top = point.y - AUTOCOMPLETE_HEIGHT;
		rect.bottom = point.y;
	}
	rect.left = point.x;
	rect.right = point.x + AUTOCOMPLETE_WIDTH;
	autoCompleteListBox.MoveWindow( &rect );
	autoCompleteListBox.ShowWindow( TRUE );
	AutoCompleteUpdate();*/
}

/*
================
SyntaxRichEditCtrl::AutoCompleteHide
================
*/
void SyntaxRichEditCtrl::AutoCompleteHide( void ) {
	/*autoCompleteStart = -1;
	autoCompleteListBox.ShowWindow( FALSE );*/
}

/*
================
SyntaxRichEditCtrl::ToolTipShow
================
*/
void SyntaxRichEditCtrl::ToolTipShow( int charIndex, const char *string ) {
	/*CPoint point, p1, p2;
	CRect rect;

	funcParmToolTipStart = charIndex;
	funcParmToolTip.SetWindowText( string );
	p1 = funcParmToolTip.PosFromChar( 0 );
	p2 = funcParmToolTip.PosFromChar( strlen( string ) - 1 );
	point = PosFromChar( charIndex );
	GetClientRect( rect );
	if ( point.y < rect.bottom - FUNCPARMTOOLTIP_OFFSET - FUNCPARMTOOLTIP_HEIGHT ) {
		rect.top = point.y + FUNCPARMTOOLTIP_OFFSET;
		rect.bottom = point.y + FUNCPARMTOOLTIP_OFFSET + FUNCPARMTOOLTIP_HEIGHT;
	} else {
		rect.top = point.y - FUNCPARMTOOLTIP_HEIGHT;
		rect.bottom = point.y;
	}
	rect.left = point.x;
	rect.right = point.x + FUNCPARMTOOLTIP_WIDTH + p2.x - p1.x;
	funcParmToolTip.MoveWindow( &rect );
	funcParmToolTip.ShowWindow( TRUE );*/
}

/*
================
SyntaxRichEditCtrl::ToolTipHide
================
*/
void SyntaxRichEditCtrl::ToolTipHide( void ) {
	funcParmToolTipStart = -1;
	//funcParmToolTip.ShowWindow( FALSE );
}

/*
================
SyntaxRichEditCtrl::BracedSectionStart
================
*/
bool SyntaxRichEditCtrl::BracedSectionStart( char braceStartChar, char braceEndChar ) {
	/*long selStart, selEnd;
	int brace, i;
	idStr text;

	GetSel( selStart, selEnd );
	GetText( text, 0, GetTextLength() );

	for ( brace = 1, i = selStart; i < text.Length(); i++ ) {
		if ( text[i] == braceStartChar ) {
			brace++;
		} else if ( text[i] == braceEndChar ) {
			brace--;
			if ( brace == 0 ) {
				break;
			}
		}
	}
	if ( brace == 0 ) {
		bracedSection[0] = selStart - 1;
		bracedSection[1] = i;
		BracedSectionShow();
	}

	return ( brace == 0 );*/
	return false;
}

/*
================
SyntaxRichEditCtrl::BracedSectionEnd
================
*/
bool SyntaxRichEditCtrl::BracedSectionEnd( char braceStartChar, char braceEndChar ) {
	/*long selStart, selEnd;
	int brace, i;
	idStr text;

	GetSel( selStart, selEnd );
	GetText( text, 0, GetTextLength() );

	for ( brace = 1, i = Min( selStart-2, (long)text.Length()-1 ); i >= 0; i-- ) {
		if ( text[i] == braceStartChar ) {
			brace--;
			if ( brace == 0 ) {
				break;
			}
		} else if ( text[i] == braceEndChar ) {
			brace++;
		}
	}

	if ( brace == 0 ) {
		bracedSection[0] = i;
		bracedSection[1] = selStart - 1;
		BracedSectionAdjustEndTabs();
		BracedSectionShow();
	}

	return ( brace == 0 );*/
	return false;
}

/*
================
SyntaxRichEditCtrl::BracedSectionAdjustEndTabs
================
*/
void SyntaxRichEditCtrl::BracedSectionAdjustEndTabs( void ) {
	/*int line, lineIndex, length, column, numTabs, i;
	char buffer[1024];
	idStr text;

	line = LineFromChar( bracedSection[0] );
	length = GetLine( line, buffer, sizeof( buffer ) );
	for ( numTabs = 0; numTabs < length; numTabs++ ) {
		if ( !idStr::CharIsTab( buffer[numTabs] ) ) {
			break;
		}
		text.Append( '\t' );
	}

	line = LineFromChar( bracedSection[1] );
	lineIndex = LineIndex( line );
	length = GetLine( line, buffer, sizeof( buffer ) );
	column = bracedSection[1] - lineIndex;
	for ( i = 0; i < column; i++ ) {
		if ( charType[buffer[i]] != CT_WHITESPACE ) {
			return;
		}
	}

	ReplaceText( lineIndex, lineIndex + column, text );

	bracedSection[1] += numTabs - column;
	SetSel( bracedSection[1]+1, bracedSection[1]+1 );*/
}

/*
================
SyntaxRichEditCtrl::BracedSectionShow
================
*/
void SyntaxRichEditCtrl::BracedSectionShow( void ) {
	/*for (int i = 0; i < 2; i++) {
		if ( bracedSection[i] >= 0 ) {
			SetColor( bracedSection[i], bracedSection[i] + 1, braceHighlightColor, DEFAULT_BACK_COLOR, true );
		}
	}*/
}

/*
================
SyntaxRichEditCtrl::BracedSectionHide
================
*/
void SyntaxRichEditCtrl::BracedSectionHide( void ) {
	/*for (int i = 0; i < 2; i++) {
		if ( bracedSection[i] >= 0 ) {
			SetColor( bracedSection[i], bracedSection[i] + 1, defaultColor, DEFAULT_BACK_COLOR, false );
			bracedSection[i] = -1;
		}
	}*/
}

/*
================
SyntaxRichEditCtrl::GetNameBeforeCurrentSelection
================
*/
bool SyntaxRichEditCtrl::GetNameBeforeCurrentSelection( idStr &name, int &charIndex ) const {
	/*long selStart, selEnd;
	int line, column, length;
	char buffer[1024];

	GetSel( selStart, selEnd );
	charIndex = selStart;
	line = LineFromChar( selStart );
	length = GetLine( line, buffer, sizeof( buffer ) );
	column = selStart - LineIndex( line ) - 1;
	do {
		buffer[column--] = '\0';
	} while( charType[buffer[column]] == CT_WHITESPACE );
	for ( length = 0; length < column; length++ ) {
		if ( charType[buffer[column-length-1]] != CT_NAME ) {
			break;
		}
	}
	if ( length > 0 ) {
		name = buffer + column - length;
		return true;
	}
	return false;*/
	return false;
}

/*
================
SyntaxRichEditCtrl::GetNameForMousePosition
================
*/
bool SyntaxRichEditCtrl::GetNameForMousePosition( idStr &name ) const {
	/*int charIndex, startCharIndex, endCharIndex, type;
	idStr text;

	charIndex = CharFromPos( mousePoint );

	for ( startCharIndex = charIndex; startCharIndex > 0; startCharIndex-- ) {
		GetText( text, startCharIndex - 1, startCharIndex );
		type = charType[text[0]];
		if ( type != CT_NAME && type != CT_NUMBER ) {
			break;
		}
	}

	for ( endCharIndex = charIndex; endCharIndex < GetTextLength(); endCharIndex++ ) {
		GetText( text, endCharIndex, endCharIndex + 1 );
		type = charType[text[0]];
		if ( type != CT_NAME && type != CT_NUMBER ) {
			break;
		}
	}

	GetText( name, startCharIndex, endCharIndex );

	return ( endCharIndex > startCharIndex );*/
	return false;
}

/*
================
SyntaxRichEditCtrl::OnToolHitTest
================
*//*
INT_PTR SyntaxRichEditCtrl::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CRichEditCtrl::OnToolHitTest( point, pTI );

	pTI->hwnd = GetSafeHwnd();
	pTI->uId = (UINT_PTR)GetSafeHwnd();
	pTI->uFlags |= TTF_IDISHWND;
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	pTI->rect = CRect( point, point );
	pTI->rect.right += 100;
	pTI->rect.bottom += 20;
	return pTI->uId;
}*/

/*
================
CSyntaxRichEditCtrl::OnToolTipNotify
================
*//*
BOOL CSyntaxRichEditCtrl::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {

#ifdef _UNICODE
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
#else
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
#endif

	*pResult = 0;

	idStr name;

	if ( GetNameForMousePosition( name ) ) {
		CString toolTip;

		if ( GetToolTip == NULL || !GetToolTip( name, toolTip ) ) {

			int keyWordIndex = FindKeyWord( name, name.Length() );

			if ( keyWordIndex != -1 && keyWords[keyWordIndex].description[0] != '\0' ) {
				toolTip = keyWords[keyWordIndex].description;
			} else {
				toolTip = name.c_str();
			}
		}

		AFX_MODULE_THREAD_STATE *state = AfxGetModuleThreadState();

		// set max tool tip width to enable multi-line tool tips using "\r\n" for line breaks
		state->m_pToolTip->SetMaxTipWidth( 500 );

		// set the number of milliseconds after which the tool tip automatically disappears
		state->m_pToolTip->SetDelayTime( TTDT_AUTOPOP, 5000 + toolTip.GetLength() * 50 );

#ifndef _UNICODE
		if( pNMHDR->code == TTN_NEEDTEXTA ) {
			delete m_pchTip;
			m_pchTip = new TCHAR[toolTip.GetLength() + 2];
			lstrcpyn( m_pchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTW->lpszText = (WCHAR*)m_pchTip;
		} else {
			delete m_pwchTip;
			m_pwchTip = new WCHAR[toolTip.GetLength() + 2];
			_mbstowcsz( m_pwchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTW->lpszText = (WCHAR*)m_pwchTip;
		}
#else
		if( pNMHDR->code == TTN_NEEDTEXTA ) {
			delete m_pchTip;
			m_pchTip = new TCHAR[toolTip.GetLength() + 2];
			_wcstombsz( m_pchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTA->lpszText = (LPTSTR)m_pchTip;
		} else {
			delete m_pwchTip;
			m_pwchTip = new WCHAR[toolTip.GetLength() + 2];
			lstrcpyn( m_pwchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTA->lpszText = (LPTSTR) m_pwchTip;
		}
#endif

		return TRUE;
	}
	return FALSE;
}*/

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
		//AfxMessageBox( "The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0 );
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
			scriptEdit->DeleteSelection();
			scriptEdit->InsertText( replaceStr.c_str() );
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

		/*int numReplaces = scriptEdit->ReplaceAll(findStr, replaceStr, matchCase, matchWholeWords);
		if (numReplaces == 0) {
			AfxMessageBox("The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0);
		}
		else {
			AfxMessageBox(va("Replaced %d occurances.", numReplaces), MB_OK | MB_ICONINFORMATION, 0);
		}*/
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
SyntaxRichEditCtrl::OnGetDlgCode
================
*//*
UINT CSyntaxRichEditCtrl::OnGetDlgCode() {
	// get all keys, including tabs
	return DLGC_WANTALLKEYS | DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTMESSAGE | DLGC_WANTTAB;
}*/

/*
================
SyntaxRichEditCtrl::OnKeyDown
================
*//*
void CSyntaxRichEditCtrl::OnKeyDown( UINT nKey, UINT nRepCnt, UINT nFlags ) {

	if ( m_TextDoc == NULL ) {
		return;
	}

	if ( autoCompleteStart >= 0 ) {
		int sel;

		switch( nKey ) {
			case VK_UP: {		// up arrow
				sel = Max( 0, autoCompleteListBox.GetCurSel() - 1 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_DOWN: {		// down arrow
				sel = Min( autoCompleteListBox.GetCount() - 1, autoCompleteListBox.GetCurSel() + 1 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_PRIOR: {	// page up key
				sel = Max( 0, autoCompleteListBox.GetCurSel() - 10 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_NEXT: {		// page down key
				sel = Min( autoCompleteListBox.GetCount() - 1, autoCompleteListBox.GetCurSel() + 10 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_HOME: {		// home key
				autoCompleteListBox.SetCurSel( 0 );
				return;
			}
			case VK_END: {
				autoCompleteListBox.SetCurSel( autoCompleteListBox.GetCount() - 1 );
				return;
			}
			case VK_RETURN:		// enter key
			case VK_TAB: {		// tab key
				AutoCompleteInsertText();
				AutoCompleteHide();
				return;
			}
			case VK_LEFT:		// left arrow
			case VK_RIGHT:		// right arrow
			case VK_INSERT:		// insert key
			case VK_DELETE: {	// delete key
				return;
			}
		}
	}

	BracedSectionHide();

	switch( nKey ) {
		case VK_TAB: {		// multi-line tabs
			long selStart, selEnd;

			GetSel( selStart, selEnd );

			// if multiple lines are selected add tabs to, or remove tabs from all of them
			if ( selEnd > selStart ) {
				CString text;

				text = GetSelText();

				if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) {
					if ( idStr::CharIsTab( text[0] ) ) {
						text.Delete( 0, 1 );
					}
					for ( int i = 0; i < text.GetLength() - 2; i++ ) {
						if ( idStr::CharIsNewLine( text[i] ) ) {
							do {
								i++;
							} while( idStr::CharIsNewLine( text[i] ) );
							if ( idStr::CharIsTab( text[i] ) ) {
								text.Delete( i, 1 );
							}
						}
					}
				} else {
					text.Insert( 0, '\t' );
					for ( int i = 0; i < text.GetLength() - 1; i++ ) {
						if ( idStr::CharIsNewLine( text[i] ) ) {
							do {
								i++;
							} while( idStr::CharIsNewLine( text[i] ) );
							text.Insert( i, '\t' );
						}
					}
				}

				ReplaceSel( text, TRUE );
				SetSel( selStart, selStart + text.GetLength() );
			} else {
				ReplaceSel( "\t", TRUE );
			}
			return;
		}
		case VK_RETURN: {	// auto-indentation
			long selStart, selEnd;
			int line, length, numTabs, i;
			char buffer[1024];
			idStr text;

			GetSel( selStart, selEnd );
			line = LineFromChar( selStart );
			length = GetLine( line, buffer, sizeof( buffer ) );
			for ( numTabs = 0; numTabs < length; numTabs++ ) {
				if ( !idStr::CharIsTab( buffer[numTabs] ) ) {
					break;
				}
			}
			bool first = true;
			for ( i = numTabs; i < length; i++ ) {
				if ( buffer[i] == '{' ) {
					numTabs++;
					first = false;
				} else if ( buffer[i] == '}' && !first ) {
					numTabs--;
				}
			}
			text = "\r\n";
			for ( i = 0; i < numTabs; i++ ) {
				text.Append( '\t' );
			}
			ReplaceSel( text, TRUE );
			return;
		}
	}

	m_TextDoc->Freeze( NULL );

	CRichEditCtrl::OnKeyDown( nKey, nRepCnt, nFlags );

	UpdateVisibleRange();

	m_TextDoc->Unfreeze( NULL );
}*/

/*
================
SyntaxRichEditCtrl::OnChar
================
*/
bool SyntaxRichEditCtrl::OnChar( bool ctrl, bool shift, bool alt ) {
	/*
	if ( nChar == VK_TAB ) {
		return;	// tab is handle in OnKeyDown
	}

	CRichEditCtrl::OnChar( nChar, nRepCnt, nFlags );

	// if the auto-complete list box is up
	if ( autoCompleteStart >= 0 ) {
		long selStart, selEnd;

		if ( charType[nChar] == CT_NAME ) {
			AutoCompleteUpdate();
			return;
		} else if ( nChar == VK_BACK ) {
			GetSel( selStart, selEnd );
			if ( selStart > autoCompleteStart ) {
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
		long selStart, selEnd;

		if ( nChar == ')' || nChar == VK_ESCAPE ) {
			ToolTipHide();
		} else if ( nChar == VK_BACK ) {
			GetSel( selStart, selEnd );
			if ( selStart < funcParmToolTipStart ) {
				ToolTipHide();
			}
		}
	}

	// show keyword auto-completion
	if ( keyWordAutoCompletion && charType[nChar] == CT_NAME && funcParmToolTipStart < 0 ) {
		long selStart, selEnd;
		int line, column, length, i;
		char buffer[1024];

		GetSel( selStart, selEnd );
		line = LineFromChar( selStart );
		length = GetLine( line, buffer, sizeof( buffer ) );
		column = selStart - LineIndex( line );
		if ( column <= 1 || charType[buffer[column-2]] == CT_WHITESPACE ) {
			if ( column >= length-1 || charType[buffer[column]] == CT_WHITESPACE ) {

				autoCompleteListBox.ResetContent();
				for ( i = 0; keyWords[i].keyWord; i++ ) {
					autoCompleteListBox.AddString( keyWords[i].keyWord );
				}
				AutoCompleteShow( selStart - 1 );
			}
		}
		return;
	}

	// highlight braced sections
	if ( nChar == '{' ) {
		BracedSectionStart( '{', '}' );
	} else if ( nChar == '}' ) {
		BracedSectionEnd( '{', '}' );
	} else if ( nChar == '(' ) {
		BracedSectionStart( '(', ')' );
	} else if ( nChar == ')' ) {
		BracedSectionEnd( '(', ')' );
	} else if ( nChar == '[' ) {
		BracedSectionStart( '[', ']' );
	} else if ( nChar == ']' ) {
		BracedSectionEnd( '[', ']' );
	} else if ( nChar == '<' ) {
		BracedSectionStart( '<', '>' );
	} else if ( nChar == '>' ) {
		BracedSectionEnd( '<', '>' );
	}

	// show object member auto-completion
	if ( nChar == '.' && GetObjectMembers && funcParmToolTipStart < 0 ) {
		int charIndex;
		CString name;

		if ( GetNameBeforeCurrentSelection( name, charIndex ) ) {
			autoCompleteListBox.ResetContent();
			if ( GetObjectMembers( name, autoCompleteListBox ) ) {
				AutoCompleteShow( charIndex );
			}
		}
		return;
	}

	// show function parameter tool tip
	if ( nChar == '(' && GetFunctionParms ) {
		int charIndex;
		CString name;

		if ( GetNameBeforeCurrentSelection( name, charIndex ) ) {
			CString parmString;
			if ( GetFunctionParms( name, parmString ) ) {
				ToolTipShow( charIndex, parmString );
			}
		}
		return;
	}*/
	bool result = true;

	if ( ctrl && ImGui::IsKeyPressed( ImGuiKey_G ) ) {
		OnEditGoToLine();
	} else if ( ctrl && ImGui::IsKeyPressed( ImGuiKey_F ) ) {
		idStr selText = scriptEdit->GetSelectedText().c_str();

		findDlg.Start( selText, false );
	} else if ( ctrl && ImGui::IsKeyPressed( ImGuiKey_H ) ) {
		idStr selText = scriptEdit->GetSelectedText().c_str();

		findDlg.Start( selText, true );
	}

	return result;
}

/*
================
SyntaxRichEditCtrl::OnLButtonDown
================
*//*
void SyntaxRichEditCtrl::OnLButtonDown( UINT nFlags, CPoint point ) {

	if ( autoCompleteStart >= 0 ) {
		AutoCompleteHide();
	}

	BracedSectionHide();

	CRichEditCtrl::OnLButtonDown( nFlags, point );
}*/

/*
================
SyntaxRichEditCtrl::OnMouseWheel
================
*//*
BOOL SyntaxRichEditCtrl::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt ) {
	if ( autoCompleteStart >= 0 ) {
		int sel;

		if ( zDelta > 0  ) {
			sel = Max( 0, autoCompleteListBox.GetCurSel() - ( zDelta / WHEEL_DELTA ) );
		} else {
			sel = Min( autoCompleteListBox.GetCount() - 1, autoCompleteListBox.GetCurSel() - ( zDelta / WHEEL_DELTA ) );
		}
		autoCompleteListBox.SetCurSel( sel );
		return TRUE;
	}

	m_TextDoc->Freeze( NULL );

	LineScroll( -3 * ( (int) zDelta ) / WHEEL_DELTA, 0 );

	UpdateVisibleRange();

	m_TextDoc->Unfreeze( NULL );

	return TRUE;
}*/

/*
================
SyntaxRichEditCtrl::OnMouseMove
================
*//*
void SyntaxRichEditCtrl::OnMouseMove( UINT nFlags, CPoint point ) {
	CRichEditCtrl::OnMouseMove( nFlags, point );

	if ( point != mousePoint ) {
		mousePoint = point;

		// remove tool tip and activate the tool tip control, otherwise
		// tool tips stop working until the mouse moves over another window first
		AFX_MODULE_THREAD_STATE *state = AfxGetModuleThreadState();
		state->m_pToolTip->Pop();
		state->m_pToolTip->Activate( TRUE );
	}
}*/

/*
================
SyntaxRichEditCtrl::OnSize
================
*//*
void SyntaxRichEditCtrl::OnSize( UINT nType, int cx, int cy ) {
	m_TextDoc->Freeze( NULL );

	CRichEditCtrl::OnSize( nType, cx, cy );

	m_TextDoc->Unfreeze( NULL );

	UpdateVisibleRange();
}*/

/*
================
SyntaxRichEditCtrl::OnVScroll
================
*//*
void SyntaxRichEditCtrl::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
	m_TextDoc->Freeze( NULL );

	CRichEditCtrl::OnVScroll( nSBCode, nPos, pScrollBar );

	SetFocus();

	UpdateVisibleRange();

	m_TextDoc->Unfreeze( NULL );
}*/

/*
================
SyntaxRichEditCtrl::OnProtected
================
*//*
void CSyntaxRichEditCtrl::OnProtected( NMHDR *pNMHDR, LRESULT *pResult ) {
	ENPROTECTED* pEP = (ENPROTECTED*)pNMHDR;

	*pResult = 0;

	updateRange = pEP->chrg;

	switch( pEP->msg ) {
		case WM_MOUSEMOVE: {
			break;
		}
		case WM_SETTEXT: {
			updateRange.cpMin = pEP->chrg.cpMin;
			updateRange.cpMax = pEP->chrg.cpMin + strlen( (LPCTSTR) pEP->lParam );
			break;
		}
		case WM_CUT: {
			break;
		}
		case WM_COPY: {
			break;
		}
		case WM_PASTE: {
			break;
		}
		case WM_CLEAR: {
			break;
		}
		case WM_UNDO: {
			break;
		}
		default: {
			break;
		}
	}
}*/

/*
================
SyntaxRichEditCtrl::OnChange
================
*//*
void SyntaxRichEditCtrl::OnChange() {
	long selStart, selEnd;

	if ( !updateSyntaxHighlighting ) {
		return;
	}

	GetSel( selStart, selEnd );
	selStart = Min( selStart, updateRange.cpMin );
	selEnd = Max( selEnd, updateRange.cpMax );

	HighlightSyntax( selStart, selEnd );

	// send EN_CHANGE notification to parent window
	NMHDR pNMHDR;
	pNMHDR.hwndFrom = GetSafeHwnd();
	pNMHDR.idFrom = GetDlgCtrlID();
	pNMHDR.code = EN_CHANGE;
	GetParent()->SendMessage( WM_NOTIFY, ( EN_CHANGE << 16 ) | GetDlgCtrlID(), (LPARAM)&pNMHDR );
}*/

/*
================
SyntaxRichEditCtrl::OnAutoCompleteListBoxChange
================
*//*
void SyntaxRichEditCtrl::OnAutoCompleteListBoxChange() {
	// steal focus back from the auto-complete list box
	SetFocus();
}*/

/*
================
SyntaxRichEditCtrl::OnAutoCompleteListBoxDblClk
================
*//*
void SyntaxRichEditCtrl::OnAutoCompleteListBoxDblClk() {
	// steal focus back from the auto-complete list box
	SetFocus();

	// insert current auto-complete selection
	AutoCompleteInsertText();
	AutoCompleteHide();
}*/

}
