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
#ifndef MEMAINFRAME_H_
#define MEMAINFRAME_H_

#include "MaterialEditor.h"
#include "MaterialTreeView.h"
#include "MaterialPropTreeView.h"
#include "MaterialPreviewView.h"
#include "StageView.h"
#include "MaterialPreviewPropView.h"
#include "MEOptions.h"
#include "ConsoleView.h"
#include "FindDialog.h"
//#include "../common/PropTree/PropTreeView.h"
#include "MaterialDocManager.h"
#include "MaterialEditView.h"

namespace ImGuiTools {

/**
* The main window for the material editor.
*/
class MEMainFrame : public MaterialView
{

public:
	MEMainFrame();
	virtual ~MEMainFrame();

	//Public Operations
	void						PrintConsoleMessage(const char *msg);

	bool						PreCreateWindow();
	void						OnCreateClient();

	//Message Handlers
	int							OnCreate();
	void						OnDestroy();

	void						SetActiveWindow(MaterialEditorWindow_t window);

	void						Draw();

	//Menu Message Handlers
	void						OnFileExit();
	void						OnFileSaveMaterial();
	void						OnFileSaveFile();
	void						OnFileSaveAll();
	bool						IsFileSaveMaterialEnabled();
	bool						IsFileSaveEnabled();
	bool						IsFileSaveAllEnabled();

	void						OnApplyMaterial();
	void						OnApplyFile();
	void						OnApplyAll();
	bool						IsApplyMaterialEnabled();
	bool						IsApplyFileEnabled();
	bool						IsApplyAllEnabled();

	void						OnEditCut();
	void						OnEditCopy();
	void						OnEditPaste();
	void						OnEditDelete();
	void						OnEditRename();
	bool						IsEditCutEnabled();
	bool						IsEditCopyEnabled();
	bool						IsEditPasteEnabled();
	bool						IsEditDeleteEnabled();
	bool						IsEditRenameEnabled();

	void				OnEditFind();
	void				OnEditFindNext();

	void				OnEditUndo();
	void				OnEditRedo();
	bool				IsEditUndoEnabled();
	bool				IsEditRedoEnabled();

	void				OnViewIncludeFile();
	void				OnReloadArbPrograms();
	void				OnReloadImages();

private:
	//Methods for Find interactions
	friend						FindDialog;
	void						CloseFind();
	void						FindNext(MaterialSearchData_t* search);

	//MaterialView Interface
	virtual void				MV_OnMaterialSelectionChange(MaterialDoc* pMaterial);

private:
	//Status and Toolbars
	//CStatusBar					m_wndStatusBar;
	//CToolBar					m_wndToolBar;

	//Splitter windows
	/*
	CTabCtrl					m_tabs;
	CSplitterWnd				m_splitterWnd;
	CSplitterWnd				m_editSplitter;
	CSplitterWnd				m_previewSplitter;
	CSplitterWnd*				m_materialEditSplitter;
	*/
	float						editSplitterWidth;
	float						editSplitterPos;
	float						editSplitterHeight;
	float						previewSplitterWidth;
	float						previewSplitterPos;
	float						previewSplitterHeight;

	//Child Views
	MaterialTreeView*			m_materialTreeView;
	StageView*					m_stageView;
	MaterialPropTreeView*		m_materialPropertyView;
	MaterialPreviewView*		m_materialPreviewView;
	MaterialPreviewPropView*	m_previewPropertyView;
	ConsoleView*				m_consoleView;

	MaterialEditView*			m_materialEditView;

	//Find Data
	FindDialog*		 			m_find;
	MaterialSearchData_t		searchData;

	//Document Management
	MaterialDocManager			materialDocManager;
	MaterialDoc*				currentDoc;

	//Options
	MEOptions					options;

	bool						includeFileInMaterialList;

	MaterialEditorWindow_t		activeWindow;
};

}

#endif /* !MEMAINFRAME_H_ */
