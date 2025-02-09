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
#include "sys/sys_imgui.h"

#include "framework/FileSystem.h"

#include "PathTreeCtrl.h"
#include "DeclBrowser.h"
#include "DeclEditor.h"
#include "DeclNew.h"

const int DECLTYPE_SHIFT			= 24;
const int DECLINDEX_MASK			= ( 1 << DECLTYPE_SHIFT ) - 1;
const int DECLTYPE_SCRIPT			= 126;
const int DECLTYPE_GUI				= 127;

#define GetIdFromTypeAndIndex( type, index )		( ( (int)type << DECLTYPE_SHIFT ) | index )
#define GetTypeFromId( id )							( (declType_t) ( (int)id >> DECLTYPE_SHIFT ) )
#define GetIndexFromId( id )						( (int)id & DECLINDEX_MASK )

namespace ImGuiTools {

/*
toolTip_t DialogDeclBrowser::toolTips[] = {
	{ IDC_DECLBROWSER_TREE, "decl browser" },
	{ IDC_DECLBROWSER_EDIT_SEARCH_NAMES, "search for declarations with matching name, use meta characters: *, ? and [abc...]" },
	{ IDC_DECLBROWSER_EDIT_SEARCH_TEXT, "search for declarations containing text" },
	{ IDC_DECLBROWSER_BUTTON_FIND, "find declarations matching the search strings" },
	{ IDC_DECLBROWSER_BUTTON_EDIT, "edit selected declaration" },
	{ IDC_DECLBROWSER_BUTTON_NEW, "create new declaration" },
	{ IDC_DECLBROWSER_BUTTON_RELOAD, "reload declarations" },
	{ IDOK, "ok" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};
*/

/*
================
DeclBrowser::DeclBrowser
================
*/
DeclBrowser::DeclBrowser()
	: isShown(false)
	, statusBarText()
	, declTree()
	, findNameStatic()
	, findTextStatic()
	, findNameEdit()
	, findTextEdit()
	, findButtonEnabled(false)
	, editButtonEnabled(false)
	, newButtonEnabled(false)
	, reloadButtonEnabled(false)
	, cancelButtonEnabled(false)
	, baseDeclTree()
	, numListedDecls(0)
	, findNameString()
	, findTextString()
{
}

DeclBrowser& DeclBrowser::Instance() {
	static DeclBrowser instance;
	return instance;
}

/*
================
DeclBrowser::Draw
================
*/
void DeclBrowser::Draw() {
	bool showTool;

	showTool = isShown;

	if ( ImGui::Begin( "Decl Browser", &showTool, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		impl::SetReleaseToolMouse( true );

		if ( ImGui::BeginChild( "Decl Tree", ImVec2( 300, 400 ) ) ) {
			declTree.Draw();
			ImGui::EndChild();
		}

		if ( ImGui::InputTextStr( "Search names", &findNameEdit ) ) {
		}
		if ( ImGui::InputTextStr( "Search text", &findTextEdit ) ) {
		}
		if ( ImGui::Button( "Find" ) ) {
			OnBnClickedFind();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Edit" ) ) {
			OnBnClickedEdit();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "New" ) ) {
			OnBnClickedNew();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Reload" ) ) {
			ReloadDeclarations();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Close" ) ) {
			showTool = false;
		}

	}
	ImGui::End();

	if ( isShown && !showTool )
	{
		isShown = showTool;
		impl::SetReleaseToolMouse( false );
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_DeclBrowser );
	}
}

/*
================
DeclBrowser::AddDeclTypeToTree
================
*/
template< class type >
int idListDeclSortCompare( const type *a, const type *b ) {
	return idStr::IcmpPath( (*a)->GetName(), (*b)->GetName() );
}

void DeclBrowser::AddDeclTypeToTree( declType_t type, const char *root, PathTreeCtrl &tree ) {
	int i;
	idList<const idDecl*> decls;
	idPathTreeStack stack;
	idStr rootStr, declName;

	decls.SetNum( declManager->GetNumDecls( type ) );
	for ( i = 0; i < decls.Num(); i++ ) {
		decls[i] = declManager->DeclByIndex( type, i, false );
	}
	decls.Sort( idListDeclSortCompare );

	rootStr = root;
	rootStr += "/";

	stack.PushRoot( NULL );

	for ( i = 0; i < decls.Num(); i++) {
		declName = rootStr + decls[i]->GetName();

		declName.BackSlashesToSlashes();
		declName.Strip(' ');

		tree.AddPathToTree( declName, GetIdFromTypeAndIndex( type, decls[i]->Index() ), stack );
	}
}

/*
================
DeclBrowser::AddScriptsToTree
================
*/
void DeclBrowser::AddScriptsToTree( PathTreeCtrl &tree ) {
	int i;
	idPathTreeStack stack;
	idStr scriptName;
	idFileList *files;

	files = fileSystem->ListFilesTree( "script", ".script", true );

	stack.PushRoot( NULL );

	for ( i = 0; i < files->GetNumFiles(); i++) {
		scriptName = files->GetFile( i );

		scriptName.BackSlashesToSlashes();
		scriptName.StripFileExtension();

		tree.AddPathToTree( scriptName, GetIdFromTypeAndIndex( DECLTYPE_SCRIPT, i ), stack );
	}

	fileSystem->FreeFileList( files );
}

/*
================
DeclBrowser::AddGUIsToTree
================
*/
void DeclBrowser::AddGUIsToTree( PathTreeCtrl &tree ) {
	int i;
	idPathTreeStack stack;
	idStr scriptName;
	idFileList *files;

	files = fileSystem->ListFilesTree( "guis", ".gui", true );

	stack.PushRoot( NULL );

	for ( i = 0; i < files->GetNumFiles(); i++) {
		scriptName = files->GetFile( i );

		scriptName.BackSlashesToSlashes();
		scriptName.StripFileExtension();

		tree.AddPathToTree( scriptName, GetIdFromTypeAndIndex( DECLTYPE_GUI, i ), stack );
	}

	fileSystem->FreeFileList( files );
}

/*
================
DeclBrowser::InitBaseDeclTree
================
*/
void DeclBrowser::InitBaseDeclTree( void ) {
	int i;

	numListedDecls = 0;
	baseDeclTree.DeleteAllItems();

	for ( i = 0; i < declManager->GetNumDeclTypes(); i++ ) {
		AddDeclTypeToTree( (declType_t)i, declManager->GetDeclNameFromType( (declType_t)i ), baseDeclTree );
	}

	AddScriptsToTree( baseDeclTree );
	AddGUIsToTree( baseDeclTree );
}

/*
================
DeclBrowser::GetDeclName
================
*/
void DeclBrowser::GetDeclName( PathTreeNode *item, idStr &typeName, idStr &declName ) const {
	PathTreeNode *parent;
	idStr itemName;

	declName.Clear();
	for( parent = declTree.GetParentItem( item ); parent; parent = declTree.GetParentItem( parent ) ) {
		itemName = declTree.GetItemText( item );
		declName = itemName + "/" + declName;
		item = parent;
	}
	declName.Strip( '/' );
	typeName = declTree.GetItemText( item );
}

/*
================
DeclBrowser::GetDeclFromTreeItem
================
*/
const idDecl *DeclBrowser::GetDeclFromTreeItem( PathTreeNode *item ) const {
	int id, index;
	declType_t type;
	const idDecl *decl;

	if ( declTree.GetChildItem( item ) ) {
		return NULL;
	}

	id = declTree.GetItemData( item );
	type = GetTypeFromId( id );
	index = GetIndexFromId( id );

	if ( type < 0 || type >= declManager->GetNumDeclTypes() ) {
		return NULL;
	}

	decl = declManager->DeclByIndex( type, index, false );

	return decl;
}

/*
================
DeclBrowser::GetSelectedDecl
================
*/
const idDecl *DeclBrowser::GetSelectedDecl( void ) const {
	return GetDeclFromTreeItem( declTree.GetSelectedItem() );
}

/*
================
DeclBrowser::EditSelected
================
*/
void DeclBrowser::EditSelected( void ) const {
	int id, index;
	idDict spawnArgs;
	const idDecl *decl;
	declType_t type;
	PathTreeNode *item;

	item = declTree.GetSelectedItem();

	if ( declTree.GetChildItem( item ) ) {
		return;
	}

	id = declTree.GetItemData( item );
	type = GetTypeFromId( id );
	index = GetIndexFromId( id );

	switch( type ) {
		case DECL_AF: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "articulatedFigure", decl->GetName() );
			//AFEditorInit( &spawnArgs );
			break;
		}
		case DECL_PARTICLE: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "model", decl->GetName() );
			//ParticleEditorInit( &spawnArgs );
			break;
		}
		case DECL_PDA: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "pda", decl->GetName() );
			//PDAEditorInit( &spawnArgs );
			break;
		}
		case DECLTYPE_SCRIPT:
		case DECLTYPE_GUI: {
			idStr typeName, declName;
			GetDeclName( item, typeName, declName );
			/*
			ScriptEditor* scriptEditor;
			scriptEditor = new ScriptEditor;
			scriptEditor->Create( IDD_DIALOG_SCRIPTEDITOR, GetParent() );
			scriptEditor->OpenFile( typeName + "/" + declName + ( ( type == DECLTYPE_SCRIPT ) ? ".script" : ".gui" ) );
			scriptEditor->ShowWindow( SW_SHOW );
			scriptEditor->SetFocus();
			*/
			break;
		}
		default: {
			decl = declManager->DeclByIndex( type, index, false );
			/*
			DeclEditor *declEditor;
			declEditor = new DeclEditor;
			declEditor->Create( IDD_DIALOG_DECLEDITOR, GetParent() );
			declEditor->LoadDecl( const_cast<idDecl *>( decl ) );
			declEditor->ShowWindow( SW_SHOW );
			declEditor->SetFocus();
			*/
			break;
		}
	}
}

/*
================
DeclBrowserCompareDecl
================
*/
bool DeclBrowserCompareDecl( void *data, PathTreeNode *item, const char *name ) {
	return reinterpret_cast<DeclBrowser *>(data)->CompareDecl( item, name );
}

/*
================
DeclBrowser::CompareDecl
================
*/
bool DeclBrowser::CompareDecl( PathTreeNode *item, const char *name ) const {
	if ( findNameString.Length() ) {
		if ( !idStr::Filter( findNameString, name, false ) ) {
			return false;
		}
	}

	if ( findTextString.Length() ) {
		int id, index;
		declType_t type;

		id = declTree.GetItemData( item );
		type = GetTypeFromId( id );
		index = GetIndexFromId( id );

		if ( type == DECLTYPE_SCRIPT || type == DECLTYPE_GUI ) {
			// search for the text in the script or gui
			idStr text;
			void *buffer;
			if ( fileSystem->ReadFile( idStr( name ) + ( ( type == DECLTYPE_SCRIPT ) ? ".script" : ".gui" ), &buffer ) == -1 ) {
				return false;
			}
			text = (char *) buffer;
			fileSystem->FreeFile( buffer );
			if ( text.Find( findTextString, false ) == -1 ) {
				return false;
			}
		} else {
			// search for the text in the decl
			const idDecl *decl = declManager->DeclByIndex( type, index, false );
			char *declText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
			decl->GetText( declText );
			if ( idStr::FindText( declText, findTextString, false ) == -1 ) {
				return false;
			}
		}
	}

	return true;
}

/*
================
DeclBrowser::Reset
================
*/
void DeclBrowser::Reset()  {

	//GetClientRect( initialRect );

	//baseDeclTree.Create( 0, initialRect, this, IDC_DECLBROWSER_BASE_TREE );

	InitBaseDeclTree();

	findNameString = "*";
	findNameEdit = findNameString;

	findTextString = "";
	findTextEdit = findTextString;

	numListedDecls = baseDeclTree.SearchTree( DeclBrowserCompareDecl, this, declTree );

	statusBarText = va( "%d decls listed", numListedDecls );
}

/*
================
DeclBrowser::ReloadDeclarations
================
*/
void DeclBrowser::ReloadDeclarations( void ) {
	InitBaseDeclTree();
	OnBnClickedFind();
}

/*
BEGIN_MESSAGE_MAP(DialogDeclBrowser, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, IDC_DECLBROWSER_TREE, OnTreeSelChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_DECLBROWSER_TREE, OnTreeDblclk)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_FIND, OnBnClickedFind)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_NEW, OnBnClickedNew)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_RELOAD, OnBnClickedReload)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()
*/

// DeclBrowser message handlers

/*
================
DialogDeclBrowser::OnToolTipNotify
================
*//*
BOOL DialogDeclBrowser::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	// need to handle both ANSI and UNICODE versions of the message
#ifdef _UNICODE
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
#else
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
#endif

	if ( pNMHDR->hwndFrom == declTree.GetSafeHwnd() ) {
		CString toolTip;
		const idDecl *decl = GetDeclFromTreeItem( (HTREEITEM) pNMHDR->idFrom );

		if ( !decl ) {
			return FALSE;
		}

		toolTip = va( "%s, line: %d", decl->GetFileName(), decl->GetLineNum() );

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

	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}*/

/*
================
DeclBrowser::OnTreeSelChanged
================
*/
void DeclBrowser::OnTreeSelChanged() {

	const idDecl *decl = GetSelectedDecl();
	if ( decl ) {
		statusBarText = va( "%d decls listed    -    %s, line: %d", numListedDecls, decl->GetFileName(), decl->GetLineNum() );
		findNameEdit = va( "%s/%s", declManager->GetDeclNameFromType( decl->GetType() ), decl->GetName() );
	} else {
		PathTreeNode *item = declTree.GetSelectedItem();
		idStr typeName, declName;
		GetDeclName( item, typeName, declName );
		findNameEdit = va( "%s/%s*", typeName.c_str(), declName.c_str() );
		statusBarText = va( "%d decls listed", numListedDecls );
	}
}

/*
================
DeclBrowser::OnTreeDblclk
================
*/
void DeclBrowser::OnTreeDblclk() {
	// post a message as if the edit button was clicked to make sure the editor gets focus
	//PostMessage( WM_COMMAND, ( BN_CLICKED << 16 ) | editButton.GetDlgCtrlID(), 0 );
}

/*
================
DeclBrowser::OnBnClickedFind
================
*/
void DeclBrowser::OnBnClickedFind() {
	idStr windowText;

	windowText = findNameEdit;
	findNameString = windowText;
	findNameString.Strip( ' ' );

	windowText = findTextEdit;
	findTextString = windowText;
	findTextString.Strip( ' ' );

	numListedDecls = baseDeclTree.SearchTree( DeclBrowserCompareDecl, this, declTree );

	statusBarText = va( "%d decls listed", numListedDecls );
}

/*
================
DeclBrowser::OnBnClickedEdit
================
*/
void DeclBrowser::OnBnClickedEdit() {
	EditSelected();
}

/*
================
DeclBrowser::OnBnClickedNew
================
*/
void DeclBrowser::OnBnClickedNew() {
	PathTreeNode *item;
	idStr typeName, declName;
	const idDecl *decl;
	DeclNew newDeclDlg;

	newDeclDlg.SetDeclTree( &baseDeclTree );

	item = declTree.GetSelectedItem();
	if ( item ) {
		GetDeclName( item, typeName, declName );
		newDeclDlg.SetDefaultType( typeName );
		newDeclDlg.SetDefaultName( declName );
	}

	decl = GetSelectedDecl();
	if ( decl ) {
		newDeclDlg.SetDefaultFile( decl->GetFileName() );
	}
	/*
	if ( newDeclDlg.DoModal() != IDOK ) {
		return;
	}
	*/
	decl = newDeclDlg.GetNewDecl();

	if ( decl ) {
		declName = declManager->GetDeclNameFromType( decl->GetType() );
		declName += "/";
		declName += decl->GetName();

		int id = GetIdFromTypeAndIndex( decl->GetType(), decl->Index() );

		baseDeclTree.InsertPathIntoTree( declName, id );
		item = declTree.InsertPathIntoTree( declName, id );
		declTree.SelectItem( item );

		EditSelected();
	}
}

/*
================
DeclBrowser::OnBnClickedReload
================
*/
void DeclBrowser::OnBnClickedReload() {

	declManager->Reload( false );

	ReloadDeclarations();
}

/*
================
DeclBrowser::OnBnClickedOk
================
*/
void DeclBrowser::OnBnClickedOk() {
	// with a modeless dialog once it is closed and re-activated windows seems
	// to enjoy mapping ENTER back to the default button ( OK ) even if you have
	// it NOT set as the default.. in this case use cancel button exit and ignore
	// default IDOK handling.
	// OnOK();
}

/*
================
DeclBrowser::OnBnClickedCancel
================
*/
void DeclBrowser::OnBnClickedCancel() {
	//OnCancel();
}

}
