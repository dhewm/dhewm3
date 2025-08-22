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
#include "DeclNew.h"
#include "DeclEditor.h"
#include "DeclBrowser.h"

const int DECLTYPE_SHIFT			= 24;
const int DECLINDEX_MASK			= ( 1 << DECLTYPE_SHIFT ) - 1;
const int DECLTYPE_SCRIPT			= 126;
const int DECLTYPE_GUI				= 127;

#define GetIdFromTypeAndIndex( type, index )		( ( (int)type << DECLTYPE_SHIFT ) | index )
#define GetTypeFromId( id )							( (declType_t) ( (int)id >> DECLTYPE_SHIFT ) )
#define GetIndexFromId( id )						( (int)id & DECLINDEX_MASK )

namespace ImGuiTools {

bool DeclBrowserOnToolTipNotify( void *data, TreeNode *item, idStr &tooltipText ) {
	return reinterpret_cast<DeclBrowser *>(data)->OnToolTipNotify( item, tooltipText );
}
void DeclBrowserOnTreeSelChanged( void *data, bool doubleClicked ) {
	reinterpret_cast<DeclBrowser *>(data)->OnTreeSelChanged( doubleClicked );
}
void DeclBrowserOnTreeContextMenu( void *data, TreeNode *item ) {
	// nop
}

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
	, declNewDlg()
	, declEditorDlg()
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
			ImGui::SetItemTooltip( "decl browser" );
			declTree.Draw( DeclBrowserOnToolTipNotify, DeclBrowserOnTreeSelChanged, DeclBrowserOnTreeContextMenu, this );
			ImGui::EndChild();
		}

		if ( ImGui::InputTextStr( "Search names", &findNameEdit ) ) {
		}
		ImGui::SetItemTooltip( "search for declarations with matching name, use meta characters: *, ? and [abc...]" );
		if ( ImGui::InputTextStr( "Search text", &findTextEdit ) ) {
		}
		ImGui::SetItemTooltip( "search for declarations containing text" );
		if ( ImGui::Button( "Find" ) ) {
			OnBnClickedFind();
		}
		ImGui::SetItemTooltip( "find declarations matching the search strings" );
		ImGui::SameLine();
		if ( ImGui::Button( "Edit" ) ) {
			OnBnClickedEdit();
		}
		ImGui::SetItemTooltip( "edit selected declaration" );
		ImGui::SameLine();
		if ( ImGui::Button( "New" ) ) {
			OnBnClickedNew();
		}
		ImGui::SetItemTooltip( "create new declaration" );
		ImGui::SameLine();
		if ( ImGui::Button( "Reload" ) ) {
			ReloadDeclarations();
		}
		ImGui::SetItemTooltip( "reload declarations" );
		ImGui::SameLine();
		if ( ImGui::Button( "Close" ) ) {
			showTool = false;
		}

		if ( declNewDlg.Draw() ) {
			OnBnClickedNewAccepted();
		}

		if ( declEditorDlg.Draw() ) {
			// nothing to do
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
void DeclBrowser::GetDeclName( TreeNode *item, idStr &typeName, idStr &declName ) const {
	TreeNode *parent;
	idStr itemName;

	declName.Clear();
	for( parent = declTree.GetParentItem( item ); parent && parent != declTree.GetRootItem(); parent = declTree.GetParentItem( parent ) ) {
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
const idDecl *DeclBrowser::GetDeclFromTreeItem( TreeNode *item ) const {
	int id, index;
	declType_t type;
	const idDecl *decl;

	if ( !item ) {
		return NULL;
	}

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
void DeclBrowser::EditSelected( void ) {
	int id, index;
	idDict spawnArgs;
	const idDecl *decl;
	declType_t type;
	TreeNode *item;

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
			ImGuiTools::AfEditorInit(); // TODO: pass spawnArgs
			break;
		}
		case DECL_PARTICLE: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "model", decl->GetName() );
			ImGuiTools::ParticleEditorInit( &spawnArgs );
			break;
		}
		case DECL_PDA: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "pda", decl->GetName() );
			ImGuiTools::PDAEditorInit( &spawnArgs );
			break;
		}
		case DECLTYPE_SCRIPT:
		case DECLTYPE_GUI: {
			idStr typeName, declName;
			GetDeclName( item, typeName, declName );
			idStr fileName = typeName + "/" + declName + ( ( type == DECLTYPE_SCRIPT ) ? ".script" : ".gui" );
			spawnArgs.Set( "script", fileName );
			ImGuiTools::ScriptEditorInit( &spawnArgs );
			break;
		}
		default: {
			decl = declManager->DeclByIndex( type, index, false );
			
			declEditorDlg.Start( const_cast<idDecl *>( decl ) );
			break;
		}
	}
}

/*
================
DeclBrowserCompareDecl
================
*/
bool DeclBrowserCompareDecl( void *data, TreeNode *item, const char *name ) {
	return reinterpret_cast<DeclBrowser *>(data)->CompareDecl( item, name );
}

/*
================
DeclBrowser::CompareDecl
================
*/
bool DeclBrowser::CompareDecl( TreeNode *item, const char *name ) const {
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

// DeclBrowser message handlers

/*
================
DeclBrowser::OnToolTipNotify
================
*/
bool DeclBrowser::OnToolTipNotify( TreeNode *item, idStr &tooltipText ) const {
	if ( item ) {
		const idDecl *decl = GetDeclFromTreeItem( item );

		if ( !decl ) {
			return false;
		}

		tooltipText = va( "%s, line: %d", decl->GetFileName(), decl->GetLineNum() );

		return true;
	}

	return false;
}

/*
================
DeclBrowser::OnTreeSelChanged
================
*/
void DeclBrowser::OnTreeSelChanged( bool doubleClicked ) {

	const idDecl *decl = GetSelectedDecl();
	if ( decl ) {
		statusBarText = va( "%d decls listed    -    %s, line: %d", numListedDecls, decl->GetFileName(), decl->GetLineNum() );
		findNameEdit = va( "%s/%s", declManager->GetDeclNameFromType( decl->GetType() ), decl->GetName() );
	} else {
		TreeNode *item = declTree.GetSelectedItem();
		idStr typeName, declName;
		GetDeclName( item, typeName, declName );
		findNameEdit = va( "%s/%s*", typeName.c_str(), declName.c_str() );
		statusBarText = va( "%d decls listed", numListedDecls );
	}

	if ( doubleClicked ) {
		OnBnClickedEdit();
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
	TreeNode *item;
	idStr typeName, declName;
	const idDecl *decl;

	declNewDlg.SetDeclTree( &baseDeclTree );

	item = declTree.GetSelectedItem();
	if ( item ) {
		GetDeclName( item, typeName, declName );
		declNewDlg.SetDefaultType( typeName );
		declNewDlg.SetDefaultName( declName );
	}

	decl = GetSelectedDecl();
	if ( decl ) {
		declNewDlg.SetDefaultFile( decl->GetFileName() );
	}

	declNewDlg.Start();
}

void DeclBrowser::OnBnClickedNewAccepted() {
	const idDecl *decl = declNewDlg.GetNewDecl();

	if ( decl ) {
		idStr declName = declManager->GetDeclNameFromType( decl->GetType() );
		declName += "/";
		declName += decl->GetName();

		int id = GetIdFromTypeAndIndex( decl->GetType(), decl->Index() );

		baseDeclTree.InsertPathIntoTree( declName, id );
		TreeNode *item = declTree.InsertPathIntoTree( declName, id );
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

}
