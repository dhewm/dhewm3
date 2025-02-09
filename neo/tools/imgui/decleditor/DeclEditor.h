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

#ifndef __DECLEDITOR_H__
#define __DECLEDITOR_H__

namespace ImGuiTools {

// DeclEditor dialog

class DeclEditor {
public:
						DeclEditor();   // standard constructor

	void				LoadDecl( idDecl *decl );
	void				Reset();

	void					Draw();

	void					ShowIt(bool show) {
		isShown = show;
	}
	bool					IsShown() {
		return isShown;
	}

private:
	void				OnEditGoToLine();
	void				OnEditFind();
	void				OnEditFindNext();
	void				OnEditReplace();
	//void				OnFindDialogMessage( WPARAM wParam, LPARAM lParam );
	//void				OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult );
	//void				OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult );
	void				OnBnClickedTest();
	void				OnBnClickedOk();
	void				OnBnClickedCancel();

private:
	bool				isShown;
	idStr				windowText;
	idStr				statusBarText;
	idStr				declEdit;
	bool				testButtonEnabled;
	bool				okButtonEnabled;
	bool				cancelButtonEnabled;

	//static toolTip_t	toolTips[];
	/*
	HACCEL				m_hAccel;
	CRect				initialRect;
	CFindReplaceDialog *findDlg;
	*/
	idStr				findStr;
	idStr				replaceStr;
	bool				matchCase;
	bool				matchWholeWords;
	bool				searchForward;
	idDecl *			decl;
	int					firstLine;

private:
	bool				TestDecl( const idStr &declText );
	void				UpdateStatusBar( void );
};

}

#endif /* !__DECLEDITOR_H__ */
