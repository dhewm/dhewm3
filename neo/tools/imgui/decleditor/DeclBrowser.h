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

#ifndef __DECLBROWSER_H__
#define __DECBROWSER_H__

#include "../../edit_public.h"

#include "PathTreeCtrl.h"
#include "DeclNew.h"
#include "DeclEditor.h"

namespace ImGuiTools {

// DeclBrowser dialog

class DeclBrowser {
public:
						DeclBrowser();   // standard constructor

	static DeclBrowser&	Instance();

	void				ReloadDeclarations( void );
	bool				CompareDecl( TreeNode *item, const char *name ) const;
	bool				OnToolTipNotify( TreeNode *item, idStr &tooltipText ) const;
	void				OnTreeSelChanged( bool doubleClicked );

	void				Reset();
	void				Draw();

	void				ShowIt(bool show) {
		isShown = show;
	}
	bool				IsShown() {
		return isShown;
	}

private:
	void				OnTreeDblclk();
	void				OnBnClickedFind();
	void				OnBnClickedEdit();
	void				OnBnClickedNew();
	void				OnBnClickedNewAccepted();
	void				OnBnClickedReload();

private:
	bool				isShown;
	idStr				statusBarText;
	PathTreeCtrl		declTree;
	idStr				findNameStatic;
	idStr				findTextStatic;
	idStr				findNameEdit;
	idStr				findTextEdit;
	bool				findButtonEnabled;
	bool				editButtonEnabled;
	bool				newButtonEnabled;
	bool				reloadButtonEnabled;
	bool				cancelButtonEnabled;


	PathTreeCtrl		baseDeclTree;
	int					numListedDecls;
	idStr				findNameString;
	idStr				findTextString;

	DeclNew				declNewDlg;
	DeclEditor			declEditorDlg;

private:
	void				AddDeclTypeToTree( declType_t type, const char *root, PathTreeCtrl &tree );
	void				AddScriptsToTree( PathTreeCtrl &tree );
	void				AddGUIsToTree( PathTreeCtrl &tree );
	void				InitBaseDeclTree( void );

	void				GetDeclName( TreeNode *item, idStr &typeName, idStr &declName ) const;
	const idDecl *		GetDeclFromTreeItem( TreeNode *item ) const;
	const idDecl *		GetSelectedDecl( void ) const;
	void				EditSelected( void );
};

}

#endif /* !__DECLBROWSER_H__ */
