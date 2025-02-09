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

#include "framework/FileSystem.h"

#include "DeclBrowser.h"
#include "DeclNew.h"

namespace ImGuiTools {

/*
toolTip_t DialogDeclNew::toolTips[] = {
	{ IDC_DECLNEW_COMBO_NEW_TYPE, "select the declaration type to create" },
	{ IDC_DECLNEW_EDIT_NEW_NAME, "enter a name for the new declaration" },
	{ IDC_DECLNEW_EDIT_NEW_FILE, "enter the name of the file to add the declaration to" },
	{ IDC_DECLNEW_BUTTON_NEW_FILE, "select existing file to add the declaration to" },
	{ IDOK, "create new declaration" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};*/


/*
================
DeclNew::DialogDeclNew
================
*/
DeclNew::DeclNew()
	: declTree(NULL)
	, newDecl(NULL)
{
}

/*
================
DeclNew::DoDataExchange
================
*//*
void DialogDeclNew::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogDeclNew)
	DDX_Control(pDX, IDC_DECLNEW_COMBO_NEW_TYPE, typeList);
	DDX_Control(pDX, IDC_DECLNEW_EDIT_NEW_NAME, nameEdit);
	DDX_Control(pDX, IDC_DECLNEW_EDIT_NEW_FILE, fileEdit);
	DDX_Control(pDX, IDC_DECLNEW_BUTTON_NEW_FILE, fileButton);
	DDX_Control(pDX, IDOK, okButton);
	DDX_Control(pDX, IDCANCEL, cancelButton);
	//}}AFX_DATA_MAP
}*/

/*
================
DeclNew::InitTypeList
================
*/
void DeclNew::InitTypeList( void ) {
	int i;

	typeList.Clear();
	for ( i = 0; i < declManager->GetNumDeclTypes(); i++ ) {
		typeList.Append( declManager->GetDeclNameFromType( (declType_t)i ) );
	}
}

/*
================
DeclNew::Reset
================
*/
void DeclNew::Reset()  {

	InitTypeList();

	//SetSafeComboBoxSelection( &typeList, defaultType.c_str(), -1 );
	nameEdit = defaultName;
	fileEdit = defaultFile;
}

/*
BEGIN_MESSAGE_MAP(DialogDeclNew, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_SETFOCUS()
	ON_BN_CLICKED(IDC_DECLNEW_BUTTON_NEW_FILE, OnBnClickedFile)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()
*/

// DeclNew message handlers

/*
================
DeclNew::OnBnClickedFile
================
*/
void DeclNew::OnBnClickedFile() {
	idStr typeName, folder, ext;
	idStr path;
	const char *errorTitle = "Error selecting file.";

	if ( 1/*GetSafeComboBoxSelection(&typeList, typeName, -1) == -1*/) {
		//MessageBox( "Select a declaration type first.", errorTitle, MB_OK );
		return;
	}

	declType_t type = declManager->GetDeclTypeFromName( typeName );
	if ( type >= declManager->GetNumDeclTypes() ) {
		//MessageBox( "Unknown declaration type.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	switch( type ) {
		// FIXME: SteelStorm2 has a _v1 suffix for materials, def and fx - why?
		case DECL_TABLE:		folder = "materials";	ext = ".mtr";					break;
		case DECL_MATERIAL:		folder = "materials";	ext = ".mtr";					break;
		case DECL_SKIN:			folder = "skins";		ext = ".skin";					break;
		case DECL_SOUND:		folder = "sound";		ext = ".sndshd";				break;
		case DECL_ENTITYDEF:	folder = "def";			ext = ".def|.decl";				break;
		case DECL_MODELDEF:		folder = "def";			ext = ".def";					break;
		case DECL_FX:			folder = "fx";			ext = ".fx";					break;
		case DECL_PARTICLE:		folder = "particles";	ext = ".prt";					break;
		case DECL_AF:			folder = "af";			ext = ".af";					break;
		default:				folder = "def";			ext = ".decl";					break;
	}

	path = fileSystem->RelativePathToOSPath( folder );
	path += "\\*";

	/*
	CFileDialog dlgFile( TRUE, "decl", path, 0, ext, this );
	if ( dlgFile.DoModal() == IDOK ) {
		path = fileSystem->OSPathToRelativePath( dlgFile.m_ofn.lpstrFile );
		fileEdit = path;
	}*/
}

/*
================
DeclNew::OnBnClickedOk
================
*/
void DeclNew::OnBnClickedOk() {
	idStr typeName, declName, fileName;
	const char *errorTitle = "Error creating declaration.";

	if ( !declTree ) {
		//MessageBox( "No declaration tree available.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	if ( 1/*GetSafeComboBoxSelection(&typeList, typeName, -1) == -1*/) {
		//MessageBox( "No declaration type selected.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	declName = nameEdit;
	if ( declName.Length() == 0 ) {
		//MessageBox( "No declaration name specified.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	fileName = fileEdit;
	if ( fileName.Length() == 0 ) {
		//MessageBox( "No file name specified.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	if ( declTree->FindItem( idStr( typeName + "/" + declName ) ) ) {
		//MessageBox( "Declaration already exists.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	declType_t type = declManager->GetDeclTypeFromName( typeName );
	if ( type >= declManager->GetNumDeclTypes() ) {
		//MessageBox( "Unknown declaration type.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	newDecl = declManager->CreateNewDecl( type, declName, fileName );
}

/*
================
DeclNew::OnBnClickedCancel
================
*/
void DeclNew::OnBnClickedCancel() {

}

}
