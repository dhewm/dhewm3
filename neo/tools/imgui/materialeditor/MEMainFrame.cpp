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
#include "sys/sys_imgui.h"

#include "MaterialEditor.h"
#include "MEMainFrame.h"
#include "MaterialDef.h"

#include "framework/CmdSystem.h"

#define TAB_CONTROL 0x1006

/*
BEGIN_MESSAGE_MAP(MEMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_SIZE()

	ON_NOTIFY(TCN_SELCHANGE, TAB_CONTROL, OnTcnSelChange)

	ON_COMMAND(ID_ME_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_ME_FILE_SAVEMATERIAL, OnFileSaveMaterial)
	ON_COMMAND(ID_ME_FILE_SAVEFILE, OnFileSaveFile)
	ON_COMMAND(ID_ME_FILE_SAVE, OnFileSaveAll)
	ON_UPDATE_COMMAND_UI(ID_ME_FILE_SAVEMATERIAL, OnFileSaveMaterialUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_FILE_SAVEFILE, OnFileSaveFileUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_FILE_SAVE, OnFileSaveAllUpdate )

	ON_COMMAND(ID_ME_PREVIEW_APPLYCHANGES, OnApplyMaterial)
	ON_COMMAND(ID_ME_PREVIEW_APPLYFILE, OnApplyFile)
	ON_COMMAND(ID_ME_PREVIEW_APPLYALL, OnApplyAll)
	ON_UPDATE_COMMAND_UI(ID_ME_PREVIEW_APPLYCHANGES, OnApplyMaterialUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_PREVIEW_APPLYFILE, OnApplyFileUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_PREVIEW_APPLYALL, OnApplyAllUpdate )

	ON_COMMAND(ID_ME_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_ME_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_ME_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_ME_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_ME_EDIT_RENAME, OnEditRename)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_CUT, OnEditCutUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_COPY, OnEditCopyUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_PASTE, OnEditPasteUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_DELETE, OnEditDeleteUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_RENAME, OnEditRenameUpdate)

	ON_COMMAND(ID_ME_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_ME_EDIT_FIND_NEXT, OnEditFindNext)

	ON_COMMAND(ID_ME_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_ME_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_UNDO, OnEditUndoUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_REDO, OnEditRedoUpdate )

	ON_COMMAND(ID_VIEW_INCLUDEFILENAME, OnViewIncludeFile)
	ON_COMMAND(ID_PREVIEW_RELOADARBPROGRAMS, OnReloadArbPrograms)
	ON_COMMAND(ID_PREVIEW_RELOADIMAGES, OnReloadImages )
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};*/

namespace ImGuiTools {

/**
* Constructor for MEMainFrame. Initialize some member data and load the options.
*/
MEMainFrame::MEMainFrame() {

	currentDoc = NULL;
	m_find = NULL;

	searchData.searched = false;

	activeWindow = ME_WINDOW_NONE;

	options.Load();
}

/**
* Destructor for MEMainFrame.
*/
MEMainFrame::~MEMainFrame() {
}

/**
* Called to add console text to the console view.
* @param msg The text that is to be added to the console.
*/
void MEMainFrame::PrintConsoleMessage(const char *msg) {
	m_consoleView->AddText(msg);
}

/**
* Sets a few window styles for the main window during the creation process.
*/
bool MEMainFrame::PreCreateWindow() {
	return true;
}

/**
* Called by the MFC framework to allow the window to create any client windows. This method
* creates all of the spliter windows and registers all of the views with the document manager.
*/
void MEMainFrame::OnCreateClient() {
	includeFileInMaterialList = true;

	/*
	CCreateContext consoleContext;
	consoleContext.m_pNewViewClass = RUNTIME_CLASS(ConsoleView);
	*/
	m_consoleView = new ConsoleView();
	m_consoleView->Init();
	//m_consoleView->ShowWindow(SW_HIDE);
	/*
	m_tabs.Create(TCS_BOTTOM | TCS_FLATBUTTONS | WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, TAB_CONTROL);
	m_tabs.InsertItem(0, "Editor");
	m_tabs.InsertItem(1, "Console");
	m_tabs.SetFont(materialEditorFont);

	m_splitterWnd.CreateStatic(this, 2, 1);


	m_editSplitter.CreateStatic(&m_splitterWnd, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitterWnd.IdFromRowCol(0, 0));

	if(!m_editSplitter.CreateView(0, 0, RUNTIME_CLASS(MaterialTreeView), CSize(300, 200), pContext)) {
		TRACE0("Failed to create material list pane\n");
		return FALSE;
	}

	if(!m_editSplitter.CreateView(0, 1, RUNTIME_CLASS(MaterialEditView), CSize(200, 200), pContext)) {
		TRACE0("Failed to create stage property pane\n");
		return FALSE;
	}


	m_previewSplitter.CreateStatic(&m_splitterWnd, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitterWnd.IdFromRowCol(1, 0));

	if(!m_previewSplitter.CreateView(0, 0, RUNTIME_CLASS(MaterialPreviewPropView), CSize(300, 200), pContext)) {
		TRACE0("Failed to create preview property pane\n");
		return FALSE;
	}

	if(!m_previewSplitter.CreateView(0, 1, RUNTIME_CLASS(MaterialPreviewView), CSize(100, 200), pContext)) {
		TRACE0("Failed to create preview pane\n");
		return FALSE;
	}
	*/
	//Get references to all of the views
	m_materialTreeView = new MaterialTreeView(); //(MaterialTreeView*)m_editSplitter.GetPane(0, 0);
	m_materialTreeView->OnCreate();
	m_previewPropertyView = new MaterialPreviewPropView();// (MaterialPreviewPropView*)m_previewSplitter.GetPane(0, 0);
	m_materialPreviewView = new MaterialPreviewView(); //(MaterialPreviewView*)m_previewSplitter.GetPane(0, 1);

	m_materialEditView = new MaterialEditView(); // (MaterialEditView*)m_editSplitter.GetPane(0, 1);
	m_materialEditView->OnCreate();
	m_stageView = m_materialEditView->m_stageView;
	m_materialPropertyView = m_materialEditView->m_materialPropertyView;
	//m_materialEditSplitter = &m_materialEditView->m_editSplitter;

	//Load the splitter positions from the registry

	editSplitterPos = 300;
	editSplitterWidth = 500 + m_materialEditView->m_editSplitterWidth;
	editSplitterHeight = 200;
	previewSplitterPos = 300;
	previewSplitterPos = 500;
	previewSplitterWidth = 500;
	previewSplitterHeight = 500;

	int val = options.GetMaterialEditHeight();
	if(val <= 0)
		val = 300;
	//m_splitterWnd.SetRowInfo(0, val, 0);
	editSplitterHeight = val;

	val = options.GetMaterialTreeWidth();
	if(val <= 0)
		val = 300;
	editSplitterPos = val;
	
	val = options.GetStageWidth();
	if(val <= 0)
		val = 200;
	m_materialEditView->m_editSplitterPos = val;

	val = options.GetPreviewPropertiesWidth();
	if(val <= 0)
		val = 300;
	previewSplitterPos = val;

	//Register the views with the document manager
	materialDocManager.RegisterMaterialView(this);
	materialDocManager.RegisterMaterialView(m_materialTreeView);
	materialDocManager.RegisterMaterialView(m_stageView);
	materialDocManager.RegisterMaterialView(m_materialPropertyView);
	materialDocManager.RegisterMaterialView(m_materialPreviewView);
	materialDocManager.RegisterMaterialView(m_materialEditView);

	//Let the stage window know about the prop window
	m_stageView->SetMaterialPropertyView(m_materialPropertyView);

	//Let the preview props now about the preview window
	m_previewPropertyView->RegisterPreviewView(m_materialPreviewView);
	m_previewPropertyView->InitializePropTree();
	//m_previewPropertyView->GetPropertyTreeCtrl().SetColumn(120);

	MaterialDefManager::InitializeMaterialDefLists();
	
	//Some prop tree initialization

	val = options.GetMaterialPropHeadingWidth();
	if(val <= 0)
		val = 200;
	//m_materialPropertyView->GetPropertyTreeCtrl().SetColumn(val);
	m_materialPropertyView->LoadSettings();

	val = options.GetPreviewPropHeadingWidth();
	if(val <= 0)
		val = 120;
	//m_previewPropertyView->GetPropertyTreeCtrl().SetColumn(val);

	// Build the material list
	m_materialTreeView->InitializeMaterialList( includeFileInMaterialList );
	
	//SetActiveView(m_materialTreeView);
}

/**
* Called by the framework while the window is being created. This methods
* creates the tool bars and status bars
* /todo Bmatt Nerve: Need to get the toolbars to work correctly.
*/
int MEMainFrame::OnCreate() {
	m_stageView = new StageView();

	return 0;
}

/**
* Called by the MFC framework while the window is being destroyed. This method
* saves the splitter and window positions.
*/
void MEMainFrame::OnDestroy() {
	int cur;
	int min;

	cur = editSplitterHeight;
	options.SetMaterialEditHeight(cur);

	cur = editSplitterPos;
	options.SetMaterialTreeWidth(cur);

	cur = m_materialEditView->m_editSplitterPos;
	options.SetStageWidth(cur);

	cur = previewSplitterPos;
	options.SetPreviewPropertiesWidth(cur);


	//cur = m_materialPropertyView->GetPropertyTreeCtrl().GetColumn();
	//options.SetMaterialPropHeadingWidth(cur);

	//cur = m_previewPropertyView->GetPropertyTreeCtrl().GetColumn();
	//options.SetPreviewPropHeadingWidth(cur);

	//options.SetWindowPlacement ( "mainframe", m_hWnd );
	options.Save();

	m_materialPropertyView->SaveSettings();

	MaterialDefManager::DestroyMaterialDefLists();
}

void MEMainFrame::Draw() {
	if( ImGui::BeginMenuBar() )
	{
		if( ImGui::BeginMenu( "File" ) )
		{
			ImGui::BeginDisabled( !IsFileSaveMaterialEnabled() );
			if( ImGui::MenuItem( "Save Material" ) ) {
				OnFileSaveMaterial();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled( !IsFileSaveEnabled() );
			if ( ImGui::MenuItem( "Save File", "Ctrl+S" ) ) {
				OnFileSaveFile();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled( !IsFileSaveAllEnabled() );
			if ( ImGui::MenuItem( "Save All" ) ) {
				OnFileSaveAll();
			}
			ImGui::EndDisabled();

			ImGui::Separator();

			if( ImGui::MenuItem( "Exit", "Ctrl+W" ) || ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_W ) ) {
				OnFileExit();
			}

			ImGui::EndMenu();
		}
		if ( ImGui::BeginMenu( "Edit" ) ) {
			ImGui::BeginDisabled( !IsEditUndoEnabled() );
			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_Z );
			if ( ImGui::MenuItem( "Undo", "Ctrl+Z" ) || ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_Z ) ) {
				OnEditUndo();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled( !IsEditRedoEnabled() );
			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_Y );
			ImGui::MenuItem( "Redo", "Ctrl-Y" );
			if ( ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_Y ) || ImGui::Shortcut( ImGuiMod_Shift | ImGuiMod_Ctrl | ImGuiKey_Z ) ) {
				OnEditRedo();
			}
			ImGui::EndDisabled();

			ImGui::Separator();

			ImGui::BeginDisabled( !IsEditCutEnabled() );
			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_X );
			if ( ImGui::MenuItem( "Cut", "Ctrl+X" ) || ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_X ) ) {
				OnEditCut();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled( !IsEditCopyEnabled() );
			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_C );
			if ( ImGui::MenuItem( "Copy", "Ctrl+C" ) || ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_C ) ) {
				OnEditCopy();
			}
			ImGui::EndDisabled();
			ImGui::BeginDisabled( !IsEditPasteEnabled() );
			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_V );
			if ( ImGui::MenuItem( "Paste", "Ctrl+V" ) || ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_V ) ) {
				OnEditPaste();
			}
			ImGui::EndDisabled();
			ImGui::BeginDisabled( !IsEditDeleteEnabled() );
			ImGui::SetNextItemShortcut( ImGuiKey_Delete );
			if ( ImGui::MenuItem( "Delete", "Del" ) || ImGui::Shortcut( ImGuiKey_Delete ) ) {
				OnEditDelete();
			}
			ImGui::EndDisabled();
			ImGui::BeginDisabled( !IsEditRenameEnabled() );
			ImGui::SetNextItemShortcut( ImGuiKey_F2 );
			if ( ImGui::MenuItem( "Rename", "F2" ) || ImGui::Shortcut( ImGuiKey_F2 ) ) {
				OnEditRename();
			}
			ImGui::EndDisabled();

			ImGui::Separator();

			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_F );
			if ( ImGui::MenuItem( "Find", "Ctrl+F" ) || ImGui::Shortcut( ImGuiMod_Ctrl | ImGuiKey_F ) ) {
				OnEditFind();
			}

			ImGui::EndMenu();
		}
		if ( ImGui::BeginMenu( "Material View" ) ) {
			if ( ImGui::Checkbox( "Include Filename",  &includeFileInMaterialList ) ) {
				OnViewIncludeFile();
			}

			ImGui::EndMenu();
		}
		if ( ImGui::BeginMenu( "Preview" ) ) {
			ImGui::BeginDisabled( !IsApplyAllEnabled() );
			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_A );
			if ( ImGui::MenuItem( "Apply Material", "Ctrl+A" ) ) {
				OnApplyMaterial();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled( !IsApplyFileEnabled() );
			if ( ImGui::MenuItem( "Apply File" ) ) {
				OnApplyFile();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled( !IsApplyAllEnabled() );
			if ( ImGui::MenuItem( "Apply All" ) ) {
				OnApplyAll();
			}
			ImGui::EndDisabled();

			ImGui::Separator();

			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_R );
			if ( ImGui::MenuItem( "Reload ARB Programs", "Ctrl+R" ) ) {
				OnReloadArbPrograms();
			}

			ImGui::SetNextItemShortcut( ImGuiMod_Ctrl | ImGuiKey_I );
			if ( ImGui::MenuItem( "Reload Images", "Ctrl+I" ) ) {
				OnReloadImages();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// debug
	/*ImGui::Text( "Focus: %s",
		activeWindow == ME_WINDOW_NONE ? "None"
		: activeWindow == ME_WINDOW_TREE ? "Tree"
		: activeWindow == ME_WINDOW_TEXT_EDIT ? "Text Edit"
		: activeWindow == ME_WINDOW_PROP ? "Prop"
		: activeWindow == ME_WINDOW_PREVIEW ? "Preview"
		: activeWindow == ME_WINDOW_PREVIEW_PROP ? "Preview Prop"
		: activeWindow == ME_WINDOW_STAGE ? "Stage"
		: "Console" );*/

	if ( ImGui::BeginTabBar( "MainTabBar" ) ) {

		if ( ImGui::BeginTabItem( "Editor" ) ) {
			float splitterButtonWidthOrHeight = 8.0f;
			ImVec2 windowSize = ImVec2( Max( editSplitterWidth, previewSplitterWidth ) + splitterButtonWidthOrHeight, editSplitterHeight + previewSplitterHeight + splitterButtonWidthOrHeight * 2.0f );

			if ( ImGui::BeginChild( "editorSplitters", windowSize ) ) {
				ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

				// m_editSplitter: splitter with 1 row, 2 columns (at cell 0,0 of m_splitterWnd)

				// m_materialTreeView : MaterialTreeView at 0, 0 of m_editSplitter
					// Size 300, 200
				if ( m_materialTreeView->Draw( ImVec2( editSplitterPos, editSplitterHeight ) ) ) {

				}

				ImGui::SameLine();
				ImGui::InvisibleButton( "editSplitter", ImVec2( splitterButtonWidthOrHeight, editSplitterHeight ) );
				if ( ImGui::IsItemActive() ) {
					editSplitterPos += ImGui::GetIO().MouseDelta.x;
				}
				ImGui::SameLine();

				// m_materialEditView : MaterialEditView at 0, 1 of m_editSplitter
					// Size 200, 200
				if ( m_materialEditView->Draw( ImVec2( 0, editSplitterHeight ) ) ) {
			
				}

				if ( m_find ) {
					m_find->Draw( ImVec2( previewSplitterPos, 0 ), ImVec2( editSplitterWidth, editSplitterHeight ) );
				}

				ImGui::InvisibleButton( "editRowHSplitter", ImVec2( -1, splitterButtonWidthOrHeight ) );
				if ( ImGui::IsItemActive() ) {
					editSplitterHeight += ImGui::GetIO().MouseDelta.y;
				}
				// m_previewSplitter : splitter with 1 row, 2 columns(at cell 1, 0 of m_splitterWnd)
					// m_previewPropertyView : MaterialPreviewPropView at 0, 0 of m_previewSplitter
					// Size 300, 200
			
				if ( m_previewPropertyView->Draw( ImVec2( previewSplitterPos, previewSplitterHeight ) ) ) {

				}

				ImGui::SameLine();
				ImGui::InvisibleButton( "previewSplitter", ImVec2( splitterButtonWidthOrHeight, previewSplitterHeight ) );
				if ( ImGui::IsItemActive() ) {
					previewSplitterPos += ImGui::GetIO().MouseDelta.x;
				}
				ImGui::SameLine();

				// m_materialPreviewView : MaterialPreviewView at 0, 1 of m_previewSplitter
					// Size 200, 200
				if ( m_materialPreviewView->Draw( ImVec2( windowSize.x - previewSplitterPos, previewSplitterHeight ) ) ) {

				}

				ImGui::InvisibleButton( "previewRowHSplitter", ImVec2( -1, splitterButtonWidthOrHeight ) );
				if ( ImGui::IsItemActive() ) {
					previewSplitterHeight += ImGui::GetIO().MouseDelta.y;
				}

				ImGui::PopStyleVar();
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
        }
		if (ImGui::BeginTabItem("Console"))
		{
			m_consoleView->Draw(ImVec2(0, 0), ImVec2(800, 600));
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void MEMainFrame::SetActiveWindow( MaterialEditorWindow_t window ) {
	activeWindow = window;
}

/**
* Shuts down the material editor.
* /todo BMatt Nerve: Need to warn the user if a file is modified.
*/
void MEMainFrame::OnFileExit() {
	OnDestroy();
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "quit\n" );
}

/**
* Saves the selected material.
*/
void MEMainFrame::OnFileSaveMaterial() {
	MaterialDoc* material = materialDocManager.GetCurrentMaterialDoc();
	if(material) {
		materialDocManager.SaveMaterial(material);
	}
}

/**
* Saves the selected file.
*/
void MEMainFrame::OnFileSaveFile() {
	idStr filename = m_materialTreeView->GetSaveFilename();
	if(filename.Length() > 0) {
		materialDocManager.SaveFile(filename);
	}
}

/**
* Saves all modified materials.
*/
void MEMainFrame::OnFileSaveAll() {
	materialDocManager.SaveAllMaterials();
}

/**
* Enables the save material menu item if a material is selected and has been modified.
*/
bool MEMainFrame::IsFileSaveMaterialEnabled() {
	MaterialDoc* pDoc = materialDocManager.GetCurrentMaterialDoc();

	return pDoc && pDoc->modified;
}

/**
* Enables the Save File menu item if the current file contains a modified material.
*/
bool MEMainFrame::IsFileSaveEnabled() {
	return (m_materialTreeView->CanSaveFile());
}

/**
* Enables the Save All menu item if there are any materials that have been modified.
*/
bool MEMainFrame::IsFileSaveAllEnabled() {
	return (materialDocManager.IsAnyModified());
}

/**
* Apply the selected material.
*/
void MEMainFrame::OnApplyMaterial() {
	MaterialDoc* material = materialDocManager.GetCurrentMaterialDoc();
	if(material) {
		materialDocManager.ApplyMaterial(material);
	}
}

/**
* Applies all modified materials in the selected file.
*/
void MEMainFrame::OnApplyFile() {
	idStr filename = m_materialTreeView->GetSaveFilename();
	if(filename.Length() > 0) {
		materialDocManager.ApplyFile(filename);
	}
}

/**
* Applies all modified materials.
*/
void MEMainFrame::OnApplyAll() {
	materialDocManager.ApplyAll();
}

/**
* Enables the Apply Material menu item if the current material has an apply waiting.
*/
bool MEMainFrame::IsApplyMaterialEnabled() {
	MaterialDoc* pDoc = materialDocManager.GetCurrentMaterialDoc();

	return (pDoc && pDoc->applyWaiting);
}

/**
* Enables the apply file menu item if the current file contains any materials
* that need to be applied.
*/
bool MEMainFrame::IsApplyFileEnabled() {
	MaterialDoc* pDoc = materialDocManager.GetCurrentMaterialDoc();

	return (pDoc && materialDocManager.DoesFileNeedApply(pDoc->renderMaterial->GetFileName()));
}

/**
* Enables the apply all menu item if there are any materials that need
* to be applied.
*/
bool MEMainFrame::IsApplyAllEnabled() {
	return (materialDocManager.DoesAnyNeedApply());
}

/**
* Performs a cut operation on the selected material.
*/
void MEMainFrame::OnEditCut() {
	switch ( activeWindow ) {
	case ME_WINDOW_TREE:
		m_materialTreeView->OnCut();
		break;
	case ME_WINDOW_TEXT_EDIT:
		m_materialEditView->OnCut();
		break;
	default:
		break;
	}
}

/**
* Performs a copy operation on the selected material or stage.
*/
void MEMainFrame::OnEditCopy() {
	switch ( activeWindow ) {
	case ME_WINDOW_STAGE:
		m_stageView->OnCopy();
		break;
	case ME_WINDOW_TREE:
		m_materialTreeView->OnCopy();
		break;
	case ME_WINDOW_TEXT_EDIT:
		m_materialEditView->OnCopy();
		break;
	default:
		break;
	}
}

/**
* Performs a paste operation on the selected material or stage.
*/
void MEMainFrame::OnEditPaste() {
	switch ( activeWindow ) {
	case ME_WINDOW_STAGE:
		m_stageView->OnPaste();
		break;
	case ME_WINDOW_TREE:
		m_materialTreeView->OnPaste();
		break;
	case ME_WINDOW_TEXT_EDIT:
		m_materialEditView->OnPaste();
		break;
	default:
		break;
	}
}

/**
* Performs a delete operation on the selected material or stage.
*/
void MEMainFrame::OnEditDelete() {
	switch ( activeWindow ) {
	case ME_WINDOW_STAGE:
		m_stageView->OnDeleteStage();
		return;
	case ME_WINDOW_TREE:
		m_materialTreeView->OnDeleteMaterial();
		break;
	case ME_WINDOW_TEXT_EDIT:
		m_materialEditView->OnDelete();
		break;
	default:
		break;
	}
}

/**
* Performs a rename operation on the selected material or stage.
*/
void MEMainFrame::OnEditRename() {
	switch ( activeWindow ) {
	case ME_WINDOW_STAGE:
		m_stageView->OnRenameStage();
		break;
	case ME_WINDOW_TREE:
		m_materialTreeView->OnRenameMaterial();
		break;
	default:
		break;
	}
}


/**
* Enable the cut menu item if a material is selected.
*/
bool MEMainFrame::IsEditCutEnabled() {
	return m_stageView->CanCut() || m_materialTreeView->CanCut() || m_materialEditView->CanCut();
}

/**
* Enables the copy menu item if a material or stage is selected.
*/
bool MEMainFrame::IsEditCopyEnabled() {
	return m_stageView->CanCopy() || m_materialTreeView->CanCopy() || m_materialEditView->CanCopy();
}

/**
* Enables a paste operation when a material or stage has been copied.
*/
bool MEMainFrame::IsEditPasteEnabled() {
	return m_stageView->CanPaste() || m_materialTreeView->CanPaste() || m_materialEditView->CanPaste();
}

/**
* Enables a delete operation when a material or stage is selected.
*/
bool MEMainFrame::IsEditDeleteEnabled() {
	return m_stageView->CanDelete() || m_materialTreeView->CanDelete() || m_materialEditView->CanDelete();
}

/**
* Enables a rename operation when a material, folder or stage is selected.
*/
bool MEMainFrame::IsEditRenameEnabled() {
	return m_stageView->CanRename() || m_materialTreeView->CanRename();
}

/**
* Opens the find dialog.
*/
void MEMainFrame::OnEditFind() {

	if (m_find== NULL)
	{
		m_find = new FindDialog( this );
	}

	m_find->Start();
}

/**
* Performs a search with the previously selected search parameters.
*/
void MEMainFrame::OnEditFindNext() {
	FindNext(NULL);
}

/**
* Performs an undo operation.
*/
void MEMainFrame::OnEditUndo() {
	//Check for undo operation on special windows
	switch ( activeWindow ) {
	case ME_WINDOW_TEXT_EDIT:
		m_materialEditView->m_textView.Undo();
		break;
	case ME_WINDOW_TREE:
	case ME_WINDOW_PROP:
	case ME_WINDOW_PREVIEW:
	case ME_WINDOW_PREVIEW_PROP:
	case ME_WINDOW_STAGE:
	case ME_WINDOW_CONSOLE:
		materialDocManager.Undo();
		break;
	default:
		break;
	}
}

/**
* Performs a redo operation.
*/
void MEMainFrame::OnEditRedo() {
	//Check for redo operation on special windows
	switch ( activeWindow ) {
	case ME_WINDOW_TEXT_EDIT:
		m_materialEditView->m_textView.Redo();
		break;
	case ME_WINDOW_TREE:
	case ME_WINDOW_PROP:
	case ME_WINDOW_PREVIEW:
	case ME_WINDOW_PREVIEW_PROP:
	case ME_WINDOW_STAGE:
	case ME_WINDOW_CONSOLE:
		materialDocManager.Redo();
		break;
	default:
		break;
	}
}

/**
* Enables the undo menu item if an undo is available.
*/
bool MEMainFrame::IsEditUndoEnabled() {
	return m_materialEditView->m_textView.CanUndo() || materialDocManager.IsUndoAvailable();
}

/**
* Enables the redo menu item if a redo is available.
*/
bool MEMainFrame::IsEditRedoEnabled() {
	return m_materialEditView->m_textView.CanRedo() || materialDocManager.IsRedoAvailable();
}

/**
* Toggles between including the file into the material list and not.
*/
void MEMainFrame::OnViewIncludeFile() {
	m_materialTreeView->InitializeMaterialList( includeFileInMaterialList );
}

/**
* Executes the reloadARBPrograms console command for convinience.
*/
void MEMainFrame::OnReloadArbPrograms() {
	cmdSystem->BufferCommandText(CMD_EXEC_NOW, "reloadARBprograms");
}

/**
* Executes the reloadImages command to reload images that have been changed outside
* of the editor.
*/
void MEMainFrame::OnReloadImages() {
	cmdSystem->BufferCommandText(CMD_EXEC_NOW, "reloadImages");
}

/**
* Called by the find dialog when it is closing.
*/
void MEMainFrame::CloseFind() {

}

/**
* Begins a search based on the provided parameters or the previously used
* parameters.
*/
void MEMainFrame::FindNext(MaterialSearchData_t* search) {

	if(search) {
		searchData = *search;
	} else {
		if(!searchData.searched) {
			return;
		}
	}

	//The material tree controls the searching
	if(!m_materialTreeView->FindNextMaterial(&searchData)) {
		m_find->UnableToFind();
	}
	searchData.searched = true;
}

/**
* Called when the selected material has changed.
* @param pMaterial The newly selected material.
*/
void MEMainFrame::MV_OnMaterialSelectionChange(MaterialDoc* pMaterial) {

}

}
