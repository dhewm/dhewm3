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
#include "tools/edit_gui_common.h"


#include "MaterialEditView.h"

#define EDIT_HEIGHT 25

#define EDIT_TAB_CONTROL	0x2006
#define NAME_CONTROL		0x2007

/*
BEGIN_MESSAGE_MAP(MaterialEditView, CFormView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, EDIT_TAB_CONTROL, OnTcnSelChange)
	ON_NOTIFY(EN_CHANGE, IDC_MATERIALEDITOR_EDIT_TEXT, OnEnChangeEdit)
END_MESSAGE_MAP()
*/

namespace ImGuiTools {

/**
* Constructor for MaterialEditView.
*/
MaterialEditView::MaterialEditView() {
	sourceInit = false;
	sourceChanged = false;
	propsTabSelected = true;
	textTabSelected = false;
}

/**
* Destructor for MaterialEditView.
*/
MaterialEditView::~MaterialEditView() {
}

/**
* Called when the selected material has changed.
* @param pMaterial The newly selected material.
*/
void MaterialEditView::MV_OnMaterialSelectionChange(MaterialDoc* pMaterial) {

	//Apply any text changes that have been made
	ApplyMaterialSource();

	if ( pMaterial ) {
		m_nameEdit = pMaterial->name;
		m_textView.SetReadOnly( false );

		//If the edit tab is selected then get the source
		if ( textTabSelected ) {
			GetMaterialSource();
		}

		currentMaterialName = pMaterial->name;
	} else {
		m_nameEdit.Clear();

		GetMaterialSource();
		m_textView.SetReadOnly( true );

		currentMaterialName.Clear();
	}
}

void MaterialEditView::MV_OnMaterialNameChanged(MaterialDoc* pMaterial, const char* oldName) {
	if ( !currentMaterialName.Icmp( oldName ) ) {
		currentMaterialName = pMaterial->name;
	}
}

/**
* Returns the current source text in the source edit control.
*/
idStr MaterialEditView::GetSourceText() {
	idStr text;
	m_textView.GetText( text );

	return text;
}

/**
* Gets the source of the current document and populates the
* source edit control.
*/
void MaterialEditView::GetMaterialSource() {

	//Clear it
	sourceInit = true;
	m_textView.SetText( "" );
	sourceInit = false;

	if ( materialDocManager ) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		if ( material ) {
			idStr text = material->GetEditSourceText();

			sourceInit = true;
			m_textView.SetText(text);
			sourceInit = false;
		}
	}
}

/**
* Takes the source out of the edit control and applies it
* to the material.
*/
void MaterialEditView::ApplyMaterialSource() {

	if ( !sourceChanged ) {
		return;
	}

	MaterialDoc* material = materialDocManager->CreateMaterialDoc(currentMaterialName);

	if(material) {
		idStr text = GetSourceText();
		material->ApplySourceModify(text);
	}

	sourceChanged = false;
}

/**
* Called by the MFC framework when the view is being created.
*/
void MaterialEditView::OnCreate()
{
	m_textView.Init();
	m_textView.LoadKeyWordsFromFile( "editors/material.def" );

	m_textView.SetText("");
	m_textView.SetReadOnly( true );

	/*
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_nameEdit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_READONLY, CRect(0,0,0,0), this, NAME_CONTROL);

	m_editSplitter.CreateStatic(this, 1, 2);

	if(!m_editSplitter.CreateView(0, 0, RUNTIME_CLASS(StageView), CSize(200, 200), NULL)) {
		TRACE0("Failed to create stage property pane\n");
		return -1;
	}

	if(!m_editSplitter.CreateView(0, 1, RUNTIME_CLASS(MaterialPropTreeView), CSize(500, 200), NULL)) {
		TRACE0("Failed to create property pane\n");
		return -1;
	}

	m_nameEdit.SetFont(materialEditorFont);
	*/
	m_editSplitterPos = 200;
	m_editSplitterWidth = 700;
	m_editSplitterHeight = 200;

	m_stageView = new StageView(); //(StageView*)m_editSplitter.GetPane(0, 0);
	m_stageView->OnCreate();
	m_materialPropertyView = new MaterialPropTreeView(); // (MaterialPropTreeView*)m_editSplitter.GetPane(0, 1);
	/*
	m_tabs.Create(TCS_BOTTOM | TCS_FLATBUTTONS | WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, EDIT_TAB_CONTROL);
	m_tabs.InsertItem(0, "Properties");
	m_tabs.InsertItem(1, "Text");

	m_tabs.SetFont(materialEditorFont);
	*/
}

bool MaterialEditView::Draw( const ImVec2 &size ) {
	ImGui::BeginChild( "MaterialEditView", size, ImGuiChildFlags_Borders );

	// two rows

	if ( ImGui::BeginTabBar( "MaterialEditViewTabBar" ) ) {

		if ( ImGui::BeginTabItem( "Properties" ) ) {
			OnTcnSelChange( 0 );

			if ( ImGui::BeginChild( "materialEditSplitter", size ) ) {
				float splitterButtonWidthOrHeight = 8.0f;

				m_editSplitterHeight = size.y;

				ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

				if ( m_stageView->Draw( ImVec2( m_editSplitterPos, m_editSplitterHeight ) ) ) {

				}

				ImGui::SameLine();
				ImGui::InvisibleButton( "editSplitter", ImVec2( splitterButtonWidthOrHeight, m_editSplitterHeight) );
				if ( ImGui::IsItemActive() ) {
					m_editSplitterPos += ImGui::GetIO().MouseDelta.x;
				}
				ImGui::SameLine();

				if ( m_materialPropertyView->Draw( ImVec2( 0, m_editSplitterHeight) ) ) {
			
				}

				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImGui::EndTabItem();
		}
		if ( ImGui::BeginTabItem( "Text" ) ) {
			OnTcnSelChange( 1 );

			m_textView.Draw();
			OnEnChangeEdit();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::EndChild();
	return false;
}

/**
* Called when the user changes the properties/text tab selection. This methods shows and hides
* the appropriate windows.
*/
void MaterialEditView::OnTcnSelChange(int sel) {

	switch(sel) {
		case 0:
			propsTabSelected = true;
			textTabSelected = false;

			ApplyMaterialSource();

			m_stageView->RefreshStageList();

			break;
		case 1:
			propsTabSelected = false;
			textTabSelected = true;

			GetMaterialSource();
			m_textView.SetReadOnly( false );
	}
}

/**
* Called when the user changes text in the edit control
*/
void MaterialEditView::OnEnChangeEdit() {
	if ( !m_textView.IsEdited() ) {
		return;
	}

	if ( materialDocManager && !sourceInit ) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		if ( material && !material->IsSourceModified() ) {
			sourceChanged = true;
			material->SourceModify(this);
		}
	}
}

}
