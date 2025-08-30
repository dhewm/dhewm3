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
#include "../util/ImGui_IdWidgets.h"
#include "../util/RegistryOptions.h"

#include "StageView.h"

namespace ImGuiTools {

/**
* Constructor for StageView.
*/
StageView::StageView()
	: m_propView(NULL)
	, currentMaterial(NULL)
	, messageBox(MSG_BOX_CLOSED)
	, labelEdit()
	, labelIndex(-1)
	, replaceStageName()
	, items()
	, itemCurSel(-1)
	, lastKeyDownTime(0)
	, bDragging(false)
	, internalChange(false)
{
}

/**
* Destructor for StageView.
*/
StageView::~StageView() {
	items.Clear();
	itemCurSel = -1;
}

/**
* Called when the selected material has changed.
* @param pMaterial The newly selected material.
*/
void StageView::MV_OnMaterialSelectionChange( MaterialDoc *pMaterial ) {

	currentMaterial = pMaterial;

	RefreshStageList();
}

/**
* Called when the material changes have been saved.
* @param pMaterial The saved material.
*/
void StageView::MV_OnMaterialSaved( MaterialDoc *pMaterial ) {
	// Saving a material reenables all of the stages
	if( pMaterial == currentMaterial ) {
		for ( int i = 1; i < GetItemCount(); i++ ) {
			SetToggleState( i, TOGGLE_STATE_ON );
		}
	}
}

/**
* Called when a stage is added
* @param pMaterial The material that was affected.
* @param stageNum The index of the stage that was added
*/
void StageView::MV_OnMaterialStageAdd( MaterialDoc *pMaterial, int stageNum ) {
	idStr name = pMaterial->GetAttribute( stageNum, "name" );

	int index = InsertItem( stageNum + 1, name.c_str() );
	SetToggleState( index, TOGGLE_STATE_ON );
}

/**
* Called when a stage is deleted
* @param pMaterial The material that was affected.
* @param stageNum The index of the stage that was deleted
*/
void StageView::MV_OnMaterialStageDelete(MaterialDoc* pMaterial, int stageNum) {
	DeleteItem(stageNum+1);
}

/**
* Called when a stage is moved
* @param pMaterial The material that was deleted.
* @param from The from index
* @param to The to index
*/
void StageView::MV_OnMaterialStageMove(MaterialDoc* pMaterial, int from, int to) {

	if ( !internalChange ) {
		from++;
		to++;

		idStr szLabel;
		GetItem( from, szLabel );

		//Delete the original item
		DeleteItem( from );

		//Insert the item
		InsertItem( to, szLabel.c_str() );

		int type = -1;

		int stageType = currentMaterial->GetAttributeInt( to - 1, "stagetype" );
		switch ( stageType ) {
				case MaterialDoc::STAGE_TYPE_NORMAL:
					type = MaterialDefManager::MATERIAL_DEF_STAGE;
					break;
				case MaterialDoc::STAGE_TYPE_SPECIALMAP:
					type = MaterialDefManager::MATERIAL_DEF_SPECIAL_STAGE;
					break;
		}

		m_propView->SetPropertyListType(type, to-1);
	}
}

/**
* Called when an attribute is changed
* @param pMaterial The material that was deleted.
* @param stage The stage that contains the change.
* @param attribName The attribute that has changed.
*/
void StageView::MV_OnMaterialAttributeChanged(MaterialDoc* pMaterial, int stage, const char* attribName) {

	//Refresh this stage list if a material name has changed
	if( !internalChange && currentMaterial == pMaterial && stage >= 0 && attribName && !strcmp( attribName, "name" ) ) {
		SetItemText(stage+1, currentMaterial->GetAttribute(stage, attribName));
	}
}

/**
* Returns true if the current state of the stage view will allow a copy operation
*/
bool StageView::CanCopy() {
	int nItem = GetSelectedItemIndex();

	if(nItem > 0) {
		return true;
	} else {
		return false;
	}
}

/**
* Returns true if the current state of the stage view will allow a paste operation
*/
bool StageView::CanPaste() {
	return materialDocManager->IsCopyStage();
}

/**
* Cut is not supported for stages.
*/
bool StageView::CanCut() {
	//No cut for stages
	return false;
}

/**
* Returns true if the current state of the stage view will allow a delete operation
*/
bool StageView::CanDelete() {
	int nItem = GetSelectedItemIndex();

	if(nItem > 0)
		return true;

	return false;
}

/**
* Returns true if the current state of the stage view will allow a rename operation
*/
bool StageView::CanRename() {
	int nItem = GetSelectedItemIndex();

	if ( nItem > 0 ) {
		MaterialDoc *material = materialDocManager->GetCurrentMaterialDoc();
		if( nItem > 0 && material->GetAttributeInt(nItem-1, "stagetype") == MaterialDoc::STAGE_TYPE_NORMAL)  {
			return true;
		}
	}

	return false;
}

/**
* Rebuilds the list of stages based on the currently selected material
*/
void StageView::RefreshStageList() {

	int selectedItem = GetSelectedItemIndex();

	DeleteAllItems();

	if ( currentMaterial ) {

		// Always add the material item for the main material properties
		InsertItem( 0, "Material" );
		SetToggleState( 0, TOGGLE_STATE_DISABLED );

		// Get the stage info
		int stageCount = currentMaterial->GetStageCount();
		for ( int i = 0; i < stageCount; i++ ) {
			const char *name = currentMaterial->GetAttribute( i, "name" );

			int itemNum = InsertItem( GetItemCount(), name );

			if ( currentMaterial->IsStageEnabled( i ) ) {
				SetToggleState( itemNum, TOGGLE_STATE_ON );
			} else {
				SetToggleState( itemNum, TOGGLE_STATE_OFF );
			}
		}

		if ( selectedItem < 0 ) {
			//Select the material
			SetItemState( 0 );
		} else {
			SetItemState( selectedItem );
		}
	}
}

/**
* Called by the MFC framework when the view is being created.
*/
int StageView::OnCreate() {
	/*if (ToggleListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetToggleIcons(MAKEINTRESOURCE(IDI_ME_DISABLED_ICON), MAKEINTRESOURCE(IDI_ME_ON_ICON), MAKEINTRESOURCE(IDI_ME_OFF_ICON));
	*/
	return 0;
}

bool StageView::Draw( const ImVec2 &size ) {
	if ( ImGui::BeginChild( "###StageView", size, ImGuiChildFlags_Borders ) ) {

		if ( messageBox != MSG_BOX_CLOSED ) {
			ImGui::OpenPopup( "###StageViewPopup" );
		}

		if ( ImGui::BeginPopupModal( "###StageViewPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize ) ) {
			bool accepted = false;

			switch ( messageBox ) {
			case MSG_BOX_RENAME_STAGE:
				ImGui::Text( "%s", "Please enter the new name of the stage" );
				if ( ImGui::InputTextStr( "Edit Label", &labelEdit ) ) {
				}
				break;
			case MSG_BOX_DELETE_STAGE:
				ImGui::Text( "%s", "Are you sure you want to delete this stage?" );
				break;
			case MSG_BOX_DELETE_ALL_STAGES:
				ImGui::Text( "%s", "Are you sure you want to delete all stages?" );
				break;
			case MSG_BOX_REPLACE_STAGE:
				ImGui::Text( "Do you want to replace '%s' stage?", replaceStageName.c_str() );
				break;
			}

			if ( ImGui::Button( "Yes" ) ) {
				ImGui::CloseCurrentPopup();
				accepted = true;
			}
			if ( ImGui::Button( "No" ) ) {
				ImGui::CloseCurrentPopup();
				messageBox = MSG_BOX_CLOSED;
			}
		

			if ( accepted ) {
				switch ( messageBox ) {
				case MSG_BOX_RENAME_STAGE:
					OnLvnEndlabeledit( labelIndex, labelEdit.c_str() );
					messageBox = MSG_BOX_CLOSED;
					break;
				case MSG_BOX_DELETE_STAGE:
					OnDeleteStageAccepted();
					messageBox = MSG_BOX_CLOSED;
					break;
				case MSG_BOX_DELETE_ALL_STAGES:
					OnDeleteAllStagesAccepted();
					messageBox = MSG_BOX_CLOSED;
					break;
				case MSG_BOX_REPLACE_STAGE:
					OnPasteAccepted();
					messageBox = MSG_BOX_CLOSED;
					break;
				default:
					messageBox = MSG_BOX_CLOSED;
					break;
				}
			}

			ImGui::EndPopup();
		}


		if ( ImGui::BeginListBox( "##StageViewItems", size ) ) {
			int num = items.Num();
			for ( int i = 0; i < num; i++ ) {
				ImGui::PushID( i );

				bool selected = ( i == itemCurSel );
				bool enabled = items[i].GetData() == TOGGLE_STATE_ON;
				bool disabled = items[i].GetData() == TOGGLE_STATE_DISABLED;

				ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowDoubleClick;
				if ( disabled ) {
					flags |= ImGuiSelectableFlags_Disabled;
				}

				ImGui::SetNextItemAllowOverlap();

				OnChar( true, i );

				if ( selected ) {
					ImGui::SetItemDefaultFocus();
				}

				bool interacted = ImGui::Selectable( items[i].GetLabel().c_str(), selected );
				if ( interacted ) {
					SetItemState( i );
					MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
				}
				OnChar( false, i );
				PopupMenu();
				if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_None ) )
				{
					ImGui::SetDragDropPayload( "StageViewItemsCell", &i, sizeof( int ) );

					// Display preview
					ImGui::Text( "%s", items[i].GetLabel().c_str() );
					ImGui::EndDragDropSource();
					OnLvnBegindrag( i );
					MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
				}
				if ( ImGui::BeginDragDropTarget() )
				{
					if ( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "StageViewItemsCell" ) )
					{
						IM_ASSERT( payload->DataSize == sizeof( int ) );
						int dropSource = *( const int * )payload->Data;
						OnLvnEnddrag( i );
						MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				ImGui::BeginDisabled( disabled );
				if ( ImGui::Checkbox("###cb", &enabled ) ) {
					OnNMClick( i );
					MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
				}
				ImGui::EndDisabled();

				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();

	return false;
}

void StageView::DeleteAllItems() {
	items.Clear();
	itemCurSel = -1;
	OnLvnItemchanged();
	OnLvnDeleteallitems();
}

void StageView::SetItemState( int index ) {
	if ( index >= items.Num() ) {
		return;
	}
	itemCurSel = index;
	OnLvnItemchanged();
}

int	StageView::GetItemCount() const {
	return items.Num();
}

void StageView::DeleteItem( int index ) {
	if ( index >= 0 && index < items.Num() ) {
		items.RemoveIndex( index );
	}
}

int StageView::InsertItem( int index, const char* label ) {
	if ( index < 0 || index > items.Num() + 1 ) {
		return -1;
	}

	StageViewItem item;

	item.SetLabel( label );

	if ( index < items.Num() ) {
		return items.Insert( item, index );
	}

	return items.Append( item );
}

void StageView::GetItem(int index, idStr &label) {
	if ( index >= 0 && index < items.Num() ) {
		label = items[index].GetLabel();
	}
}

int	StageView::GetSelectedItemIndex() {
	return itemCurSel;
}

void StageView::SetItemText( int index, const char* label ) {
	if ( index >= 0 && index < items.Num() ) {
		items[index].SetLabel( label );
	}
}

void StageView::SetItemData( int index, int data ) {
	if ( index >= 0 && index < items.Num() ) {
		items[index].SetData( data );
	}
}

int StageView::GetItemData( int index ) {
	if ( index >= 0 && index < items.Num() ) {
		return items[index].GetData();
	}
	return 0;
}

/**
* Sets the state of an item in the list.
* @param index Index of the item whose state should be changed.
* @param toggleState The state to set
* @param notify Determines if the notification method OnStateChanged should
* be called. OnStateChanged will also not be called if the state has not changed.
*/
void StageView::SetToggleState( int index, int toggleState, bool notify ) {
	assert(index >= 0 && index < GetItemCount());

	int oldState = GetToggleState(index);
	SetItemData( index, toggleState );

	if ( notify && oldState != toggleState ) {
		OnStateChanged( index, toggleState );
	}
}

/**
* Gets the state of an item in the list
* @param index Index of the item of which to retreive the state.
*/
int StageView::GetToggleState(int index) {
	assert(index >= 0 && index < GetItemCount());

	int data = GetItemData( index );
	return data;
}

/**
* Called when the user changes the selection in the list box. This method will notify the
* property view of the change so that it can display the appropriate properties.
*/
void StageView::OnLvnItemchanged() {

	if (!bDragging) {

		//The state has changed and changed to selected
		if(itemCurSel >= 0) {

			int type = -1;

			if(itemCurSel >= 0) {
				if(itemCurSel == 0)
					type = MaterialDefManager::MATERIAL_DEF_MATERIAL;
				else {
					int stageType = currentMaterial->GetAttributeInt(itemCurSel-1, "stagetype");
					switch(stageType) {
						case MaterialDoc::STAGE_TYPE_NORMAL:
							type = MaterialDefManager::MATERIAL_DEF_STAGE;
							break;
						case MaterialDoc::STAGE_TYPE_SPECIALMAP:
							type = MaterialDefManager::MATERIAL_DEF_SPECIAL_STAGE;
							break;
					}
				}
			}

			m_propView->SetPropertyListType(type, itemCurSel-1);
		}

		if (itemCurSel == -1) {
			//This item was deselected.
			//If there is no item selected then clear the prop list
			m_propView->SetPropertyListType(-1);
		}
	}
}

/**
* Notifies the property view that all stages have been removed.
*/
void StageView::OnLvnDeleteallitems() {
	//The list has been cleared so clear the prop view
	m_propView->SetPropertyListType(-1);
}

/**
* Starts the stage drag operation.
*/
void StageView::OnLvnBegindrag( int index ) {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );

	//Start a drag if the item isn't the material
	if(index > 0) {

		dragIndex = index;

		//Drag is in progress
		bDragging = true;
		dropIndex = -1;
	}
}

/**
* Finishes a stage drag operation of the user was dragging a stage.
*/
void StageView::OnLvnEnddrag( int index ) {
	if( bDragging ) {
		MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
		DropItemOnList( index );

		bDragging = false;
	}
}

/**
* Displays the popup menu when the user performs a right mouse click.
*/
void StageView::OnNMRclick() {
	if(materialDocManager->GetCurrentMaterialDoc()) {
		PopupMenu();
	}
}

/**
* Toggles the state of an item when the user clicks in the window.
*/
void StageView::OnNMClick( int index ) {
	if ( index != -1 ) {
		MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
		int toggleState = GetToggleState( index );
		if ( toggleState != TOGGLE_STATE_DISABLED ) {

			if (toggleState == TOGGLE_STATE_ON) {
				SetToggleState( index, TOGGLE_STATE_OFF, true );
			} else {
				SetToggleState( index, TOGGLE_STATE_ON, true );
			}
		}
	}
}

/**
* Begins a label edit when the user selects the rename menu option.
*/
void StageView::OnRenameStage() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );

	int nItem = GetSelectedItemIndex();
	if ( nItem ) {
		if ( OnLvnBeginlabeledit( nItem ) ) {
			labelEdit = items[nItem].GetLabel();
			labelIndex = nItem;
			messageBox = MSG_BOX_RENAME_STAGE;
		}
	}
}

/**
* Deletes the selected stage when the user selects the delete menu option.
*/
void StageView::OnDeleteStage() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );

	int nItem = GetSelectedItemIndex();
	if(nItem > 0) {
		messageBox = MSG_BOX_DELETE_STAGE;
	}
}

void StageView::OnDeleteStageAccepted() {
	int nItem = GetSelectedItemIndex();
	if(nItem > 0) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		material->RemoveStage(nItem-1);
		itemCurSel = 0;
	}
}

/**
* Conforms the user wants to delete all stages and then performs the operation.
*/
void StageView::OnDeleteAllStages() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
	messageBox = MSG_BOX_DELETE_ALL_STAGES;
}

void StageView::OnDeleteAllStagesAccepted() {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	material->ClearStages();
	itemCurSel = -1;
}

/**
* Adds a new stage when the user selects the menu option.
*/
void StageView::OnAddStage() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );

	MaterialDoc *material = materialDocManager->GetCurrentMaterialDoc();

	idStr name = va( "Stage %d", material->GetStageCount() + 1 );
	material->AddStage( MaterialDoc::STAGE_TYPE_NORMAL, name.c_str() );
}

/**
* Adds a new bumpmap stage when the user selects the menu option.
*/
void StageView::OnAddBumpmapStage() {
	MaterialDoc *material = materialDocManager->GetCurrentMaterialDoc();
	material->AddStage( MaterialDoc::STAGE_TYPE_SPECIALMAP, "bumpmap" );
}

/**
* Adds a new diffusemap stage when the user selects the menu option.
*/
void StageView::OnAddDiffuseStage() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	material->AddStage( MaterialDoc::STAGE_TYPE_SPECIALMAP, "diffusemap" );
}

/**
* Adds a new specularmap stage when the user selects the menu option.
*/
void StageView::OnAddSpecualarStage() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	material->AddStage( MaterialDoc::STAGE_TYPE_SPECIALMAP, "specularmap" );
}

/**
* Performs a copy operation when the user selects the menu option.
*/
void StageView::OnCopy() {

	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

	int nItem = GetSelectedItemIndex();

	if ( nItem > 0 ) {
		materialDocManager->CopyStage( material, nItem - 1 );
	}
}

/**
* Performs a paste operation when the user selects the menu option.
*/
void StageView::OnPaste() {
	MaterialEditorSetActiveWindow( ME_WINDOW_STAGE );
	if ( materialDocManager->IsCopyStage() ) {

		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

		int type;
		idStr name;

		materialDocManager->GetCopyStageInfo(type, name);

		int existingIndex = material->FindStage(type, name);

		if ( type != MaterialDoc::STAGE_TYPE_SPECIALMAP || existingIndex == -1 ) {
			materialDocManager->PasteStage( material );
		} else {
			replaceStageName = name;
			messageBox = MSG_BOX_REPLACE_STAGE;
		}
	}
}

void StageView::OnPasteAccepted() {
	if ( materialDocManager->IsCopyStage() ) {

		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

		int type;
		idStr name;

		materialDocManager->GetCopyStageInfo(type, name);

		int existingIndex = material->FindStage(type, name);

		if ( type != MaterialDoc::STAGE_TYPE_SPECIALMAP || existingIndex == -1 ) {
			
		} else {
			material->RemoveStage( existingIndex );
			materialDocManager->PasteStage( material );
		}
	}
}

/**
* Determines is a label edit can be performed on the selected stage.
*/
bool StageView::OnLvnBeginlabeledit( int index ) {
	//if this is a special stage then don't allow edit

	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	if ( index <= 0 || material->GetAttributeInt( index-1, "stagetype" ) != MaterialDoc::STAGE_TYPE_NORMAL )
	{
		return true;
	}

	return false;
}

/**
* Performs the stage name change after the label edit is done.
*/
void StageView::OnLvnEndlabeledit( int index, const char *newLabel ) {
	if ( newLabel ) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		internalChange = true;
		material->SetAttribute( index - 1, "name", newLabel );
		internalChange = false;
	}
}

/**
* Handles keyboard shortcuts for copy and paste operations.
*/
void StageView::OnChar( bool prepare, int index ) {
	if ( prepare ) {
		ImGui::SetItemKeyOwner( ImGuiKey_C );
		ImGui::SetItemKeyOwner( ImGuiKey_V );
		ImGui::SetItemKeyOwner( ImGuiKey_F2 );
		ImGui::SetItemKeyOwner( ImGuiKey_Delete );
	}
	
	int timeEnd = Sys_Milliseconds();
	int elapsed = timeEnd - lastKeyDownTime;
	int keydownTime = 200;

	if ( ImGui::IsKeyChordPressed( ImGuiMod_Ctrl | ImGuiKey_C ) ) {
		if ( elapsed > keydownTime ) {
			OnCopy();
			lastKeyDownTime = timeEnd;
		}
	}
	if ( ImGui::IsKeyChordPressed( ImGuiMod_Ctrl | ImGuiKey_V ) ) {
		if ( elapsed > keydownTime ) {
			OnPaste();
			lastKeyDownTime = timeEnd;
		}
	}
	if ( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) ) {
		if ( OnLvnBeginlabeledit( index ) ) {
			labelEdit = items[index].GetLabel();
			labelIndex = index;
		}
	}
	if ( ImGui::IsKeyPressed( ImGuiKey_F2 ) ) {
		if ( elapsed > keydownTime ) {
			if ( OnLvnBeginlabeledit( itemCurSel ) ) {
				labelEdit = items[itemCurSel].GetLabel();
				labelIndex = itemCurSel;
			}
			lastKeyDownTime = timeEnd;
		}
	}
	if ( ImGui::IsKeyPressed( ImGuiKey_Delete ) ) {
		if ( elapsed > keydownTime ) {
			OnDeleteStage();
			lastKeyDownTime = timeEnd;
		}
	}

	if ( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) ) {
		OnNMRclick();
	}
}

/**
* Called by the ToggleListView when the toggle state has changed.
*/
void StageView::OnStateChanged(int index, int toggleState) {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	if ( material && index > 0 ) {
		if ( toggleState == TOGGLE_STATE_ON ) {
			material->EnableStage( index - 1, true );
		} else if ( toggleState == TOGGLE_STATE_OFF ) {
			material->EnableStage( index - 1, false );
		}
	}
}

/**
* Displays the popup menu with the appropriate menu items enabled.
*/
void StageView::PopupMenu() {

	//Determine the type of object clicked on
	int nItem = GetSelectedItemIndex();

	if (ImGui::BeginPopupContextItem( NULL ) ) {

		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		int stageType = nItem > 0 ? material->GetAttributeInt(nItem-1, "stagetype") : -1;

		ImGui::BeginDisabled( nItem <= 0 || (nItem > 0 && stageType != MaterialDoc::STAGE_TYPE_NORMAL) );
		if ( ImGui::Button( "Rename" ) ) {
			OnRenameStage();
		}
		ImGui::EndDisabled();

		if ( ImGui::Button( "Add" ) ) {
			OnAddStage();
		}
		ImGui::BeginDisabled( material->FindStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "bumpmap") >= 0 );
		if ( ImGui::Button( "Add Bumpmap Stage" ) ) {
			OnAddBumpmapStage();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled( material->FindStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "diffusemap") >= 0 );
		if ( ImGui::Button( "Add Diffuse Stage" ) ) {
			OnAddDiffuseStage();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled( material->FindStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "specularmap") >= 0 );
		if ( ImGui::Button( "Add Specular Stage" ) ) {
			OnAddSpecualarStage();
		}
		ImGui::EndDisabled();

		ImGui::Separator();

		ImGui::BeginDisabled( nItem <= 0 );
		if ( ImGui::Button( "Copy" ) ) {
			OnCopy();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled( !materialDocManager->IsCopyStage() );
		if ( ImGui::Button( "Paste" ) ) {
			OnPaste();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled( nItem <= 0 );
		if ( ImGui::Button( "Delete" ) ) {
			OnDeleteStage();
		}
		ImGui::EndDisabled();
		if ( ImGui::Button( "Delete All Stages" ) ) {
			OnDeleteAllStages();
		}

		ImGui::EndPopup();
	}
}

/**
* Performs the stage move when the user has dragged and dropped a stage.
*/
void StageView::DropItemOnList( int dropIndex ) {
	int toStage;

	//Get and adjust the drop index based on the direction of the move
	if ( dropIndex < 0 ) {
		dropIndex = GetItemCount() - 1;
	}

	//Ignore the drop if the index is the same or they are droping on the material
	if ( dropIndex == dragIndex || dropIndex == 0 ) {
		return;
	}

	//Move the stage data
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

	internalChange = true;
	toStage = dropIndex - 1;
	material->MoveStage( dragIndex - 1, dropIndex - 1 );
	internalChange = false;

	if ( dragIndex < dropIndex ) {
		dropIndex++;
	}

	//Get the item
	idStr szLabel;

	GetItem( dragIndex, szLabel );

	//Insert the item
	InsertItem( dropIndex, szLabel.c_str() );

	//Adjust the drag index if the move was up in the list
	if ( dragIndex > dropIndex ) {
		dragIndex++;
	}

	//Delete the original item
	DeleteItem( dragIndex );

	int type = -1;
	int stageType = currentMaterial->GetAttributeInt(toStage, "stagetype");
	switch(stageType) {
		case MaterialDoc::STAGE_TYPE_NORMAL:
			type = MaterialDefManager::MATERIAL_DEF_STAGE;
			break;
		case MaterialDoc::STAGE_TYPE_SPECIALMAP:
			type = MaterialDefManager::MATERIAL_DEF_SPECIAL_STAGE;
			break;
	}
	m_propView->SetPropertyListType(type, toStage);
}

}
