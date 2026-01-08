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
#include "../util/RegistryOptions.h"
#include "tools/imgui/materialeditor/MaterialEditor.h"

#include "MaterialPropTreeView.h"

#define PROP_TREE_VIEW "PropTreeView"

namespace ImGuiTools {

/**
* Constructor for MaterialPropTreeView.
*/
MaterialPropTreeView::MaterialPropTreeView()
	: currentMaterial(NULL)
	, currentListType(0)
	, currentStage(-1)
	, currentPropDefs(NULL)
{
	registry.Init("MaterialEditor_PropertySettings");
	internalChange = false;
}

/**
* Destructor for MaterialPropTreeView.
*/
MaterialPropTreeView::~MaterialPropTreeView() {
}

bool MaterialPropTreeView::Draw( const ImVec2 &size ) {
	if ( ImGui::BeginChild( "###MaterialPropTreeView", size, ImGuiChildFlags_Borders ) ) {

		MaterialDoc *materialDoc = materialDocManager->GetCurrentMaterialDoc();

		if ( currentPropDefs )
		{
			for ( int i = 0; i < currentPropDefs->Num(); ) {
				MaterialDef *propItem = (*currentPropDefs)[i];
				ImGuiTreeNodeFlags	flags = 0;
				int j;

				switch ( (*currentPropDefs)[i]->type ) {
				case MaterialDef::MATERIAL_DEF_TYPE_GROUP:
					{
						ImGui::SetNextItemOpen( registry.GetBool(va("Expand%d%s", currentListType, propItem->displayName.c_str())) );

						if ( ImGui::TreeNodeEx( static_cast<const void*>(propItem), flags, "%s", propItem->displayName.c_str() ) ) {
							if ( ImGui::IsItemClicked() ) {
								registry.SetBool(va("Expand%d%s", currentListType, propItem->displayName.c_str()), ImGui::IsItemToggledOpen());
								registry.Save();
							}

							j = DrawGroup( i + 1 );

							ImGui::TreePop();
						} else {
							// skip until next group
							j = i + 1;
							while ( j < currentPropDefs->Num() && (*currentPropDefs)[j]->type != MaterialDef::MATERIAL_DEF_TYPE_GROUP ) {
								j++;
							}
						}

						if ( ImGui::IsItemHovered() && !propItem->displayInfo.IsEmpty() ) {
							ImGui::BeginTooltip();
							ImGui::TextUnformatted( propItem->displayInfo.c_str() );
							ImGui::EndTooltip();
						}
					}
					break;
				default:
					j = DrawGroup( i );
					break;
				}

				i = j;
			}
		}
		
		if ( ImGui::IsWindowFocused() ) {
			MaterialEditorSetActiveWindow( ME_WINDOW_PROP );
		}
	}

	ImGui::EndChild();

	return false;
}

int MaterialPropTreeView::DrawGroup( int startGroupNum ) {
	if ( !currentPropDefs ) {
		return startGroupNum + 1;
	}

	MaterialDoc* materialDoc = materialDocManager->GetCurrentMaterialDoc();

	int i = startGroupNum;

	for ( ; i < currentPropDefs->Num(); i++ ) {
		MaterialDef* propItem = (*currentPropDefs)[i];

		switch ( propItem->type ) {
		case MaterialDef::MATERIAL_DEF_TYPE_GROUP:
			return i;
		case MaterialDef::MATERIAL_DEF_TYPE_BOOL:
			{
				bool val = materialDoc->GetAttributeBool( currentStage, propItem->dictName );

				if ( ImGui::Checkbox( propItem->displayName.c_str(), &val ) ) {
					internalChange = true;
					materialDoc->SetAttributeBool(currentStage, propItem->dictName, val ? true : false);
					internalChange = false;
				}
			}
			break;
		case MaterialDef::MATERIAL_DEF_TYPE_STRING:
			{
				idStr val = materialDoc->GetAttribute( currentStage, propItem->dictName );

				if ( ImGui::InputTextStr( propItem->displayName, &val ) ) {
					internalChange = true;
					materialDoc->SetAttribute( currentStage, propItem->dictName, val );
					internalChange = false;
				}
			}
			break;
		case MaterialDef::MATERIAL_DEF_TYPE_FLOAT:
			{
				float val = materialDoc->GetAttributeFloat( currentStage, propItem->dictName );

				if ( ImGui::InputFloat( propItem->displayName, &val ) ) {
					internalChange = true;
					materialDoc->SetAttributeFloat( currentStage, propItem->dictName, val );
					internalChange = false;
				}
			}
		case MaterialDef::MATERIAL_DEF_TYPE_INT:
			{
				int val = materialDoc->GetAttributeInt( currentStage, propItem->dictName );

				if ( ImGui::InputInt( propItem->displayName, &val ) ) {
					internalChange = true;
					materialDoc->SetAttributeInt( currentStage, propItem->dictName, val );
					internalChange = false;
				}
			}
			break;
		}

		if ( ImGui::IsItemHovered() && !propItem->displayInfo.IsEmpty() ) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted( propItem->displayInfo.c_str() );
			ImGui::EndTooltip();
		}
	}

	return i;
}

/**
* Initializes the list of properties based on the type (material, stage, special stage).
* @param listType The type of list (material, stage, special stage)
* @param stageNum The stage from which to get the attributes.
*/
void MaterialPropTreeView::SetPropertyListType(int listType, int stageNum) {

	currentListType = listType;
	currentStage = stageNum;

	MaterialDefList* propList = MaterialDefManager::GetMaterialDefs(currentListType);
	currentPropDefs = propList;
}

/**
* Loads the property view settings from the registry.
*/
void MaterialPropTreeView::LoadSettings() {
	registry.Load();
}

/**
* Saves the property view settings to the registry.
*/
void MaterialPropTreeView::SaveSettings() {
	registry.Save();
}

/**
* Called when the material has changed but not applied.
* @param pMaterial The selected material.
*/
void MaterialPropTreeView::MV_OnMaterialChange(MaterialDoc* pMaterial) {

}

}
