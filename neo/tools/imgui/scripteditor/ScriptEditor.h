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

#ifndef __SCRIPTEDITOR_H__
#define __SCRIPTEDITOR_H__

#include "../../edit_public.h"

class TextEditor;

namespace ImGuiTools
{

class ScriptEditor {

	//DECLARE_DYNAMIC(DialogScriptEditor)

public:
						ScriptEditor();   // standard constructor
	//virtual				~DialogScriptEditor();

	static ScriptEditor& Instance();

	void				OpenFile( const char *fileName );

	void					Reset();
	void					Draw();

	void					ShowIt(bool show) {
		isShown = show;
	}
	bool					IsShown() {
		return isShown;
	}

	//{{AFX_VIRTUAL(DialogScriptEditor)
	bool				OnInitDialog();
	//virtual void		DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	//virtual BOOL		PreTranslateMessage( MSG* pMsg );
	//}}AFX_VIRTUAL

protected:
	void		OnEditGoToLine();
	void		OnEditFind();
	void		OnEditFindNext();
	void		OnEditReplace();
	//LRESULT		OnFindDialogMessage( WPARAM wParam, LPARAM lParam );
	//void		OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult );
	//void		OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult );
	void		OnBnClickedOk();
	void		OnBnClickedCancel();

private:
	bool				isShown;
	idStr				windowText;
	idStr				statusBarText;
	TextEditor *		scriptEdit;
	bool				okButtonEnabled;
	bool				cancelButtonEnabled;
	/*
	CButton				okButton;
	CButton				cancelButton;
	
	static toolTip_t	toolTips[];
	
	HACCEL				m_hAccel;
	CRect				initialRect;
	CFindReplaceDialog *findDlg;
	*/
	idStr				findStr;
	idStr				replaceStr;
	bool				matchCase;
	bool				matchWholeWords;
	bool				searchForward;
	idStr				fileName;
	int					firstLine;

private:
	void				InitScriptEvents( void );
	void				UpdateStatusBar( void );
};

}
#endif /* !_SCRIPTEDITOR_H__ */
