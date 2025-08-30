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



#include "MaterialPreviewPropView.h"

namespace ImGuiTools {

static const char *MaterialPreviewPropViewModelTypes[] = {
	"Cube",
	"Box - 2:1",
	"Box - 4:1",
	"Box - 1:2",
	"Box - 1:4",
	"Cylinder - V",
	"Cylinder - H",
	"Sphere"
};

// MaterialPropTreeView

MaterialPreviewPropView::MaterialPreviewPropView()
	: customModel("")
	, lights()
	, lightMaterials()
	, defaultPointLightIndex(-1)
	, customModelSelectDlg( DECL_MODELDEF, "Select Custom Model" )
{
	materialPreview = NULL;
	modelType = 0;
	showLights = true;

	for ( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++) {
		if ( i < 4 ) {
			localParms[i] = 1;
		} else {
			localParms[i] = 0;
		}
	}

	for (int i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++) {
		if ( i < 4 ) {
			globalParms[i] = 1;
		}
		else {
			globalParms[i] = 0;
		}
	}
}

MaterialPreviewPropView::~MaterialPreviewPropView() {
	customModel.Clear();
	lights.Clear();
	lightMaterials.Clear();
}

bool MaterialPreviewPropView::Draw( const ImVec2 &size ) {
	ImGui::BeginChild( "MaterialPreviewPropView", size, ImGuiChildFlags_Borders );

	if ( ImGui::TreeNodeEx( static_cast<const void*>( "#PP" ), ImGuiTreeNodeFlags_DefaultOpen, "Preview Properties")) {
		const char* modelTypePreview = MaterialPreviewPropViewModelTypes[modelType];

		if ( ImGui::BeginCombo( "Model Type", modelTypePreview, ImGuiComboFlags_None ) ) {
			for ( int n = 0; n < IM_ARRAYSIZE( MaterialPreviewPropViewModelTypes ); n++ ) {
				const bool selected = (modelType == n);
				if ( ImGui::Selectable( MaterialPreviewPropViewModelTypes[n], selected ) ) {
					materialPreview->OnModelChange( n );
					modelType = n;
				}

				if ( selected ) {
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
		ImGui::SetItemTooltip( "Select the type of model on which to preview the material." );

		// Custom model entry
		ImGui::Text( "Custom Model" );
		ImGui::SameLine();
		if ( ImGui::InputTextStr( "Custom Model", &customModel ) ) {
			materialPreview->OnCustomModelChange( customModel.c_str() );
		}
		if ( ImGui::Button( "...###CustomModel" ) ) {
			customModelSelectDlg.Start( customModel.c_str() );
		} else {
			ImGui::SetItemTooltip( "Specify any model to display the current material." );
		}
		if ( customModelSelectDlg.Draw() ) {
			idDecl *decl = customModelSelectDlg.GetDecl();
			if ( decl ) {
				customModel = decl->GetFileName();
				materialPreview->OnCustomModelChange( customModel.c_str() );
			}
		}

		// Checkbox for showing debug light spheres
		if ( ImGui::Checkbox( "Show lights", &showLights ) ) {
			materialPreview->OnShowLightsChange( showLights );
		} else {
			ImGui::SetItemTooltip( "Show the light origin sphere and number in the preview." );
		}

		ImGui::TreePop();
	}
	
	if ( ImGui::IsItemHovered() ) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted( "Properties for the preview window." );
		ImGui::EndTooltip();
	}

	// Local and Global shader parms
	if ( ImGui::TreeNodeEx( static_cast<const void*>( "#LP" ), 0, "Local Parms")) {
		for ( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
			if ( ImGui::InputFloat( va( "parm%d", i ), &localParms[i] ) ) {
				materialPreview->OnLocalParmChange( i, localParms[i] );
			}
			ImGui::SetItemTooltip( "Set the local shaderparm for the model" );
		}

		ImGui::TreePop();
	}
	if ( ImGui::IsItemHovered() ) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted( "Local shaderparms for the model being displayed." );
		ImGui::EndTooltip();
	}

	if ( ImGui::TreeNodeEx( static_cast<const void*>( "#GP" ), 0, "Global Parms")) {
		for ( int i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
			if ( ImGui::InputFloat( va( "global%d", i ), &globalParms[i] ) ) {
				materialPreview->OnGlobalParmChange( i, globalParms[i] );
			}
			ImGui::SetItemTooltip( "Set the global shaderparm for the renderworld" );
		}

		ImGui::TreePop();
	}
	if ( ImGui::IsItemHovered() ) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted( "Global shaderparms for the renderworld being displayed." );
		ImGui::EndTooltip();
	}

	// Lights

	if ( ImGui::TreeNodeEx( static_cast<const void*>( "#L" ), 0, "Lights" ) ) {
		ImGui::TextUnformatted( "Preview Lights" );
		ImGui::SameLine();
		if ( ImGui::Button( "Add Light" ) ) {
			AddLight();
		}
		ImGui::SetItemTooltip( "Test the button" );

		int lightToDelete = -1;

		for ( int i = 0; i < lights.Num(); i++ ) {
			bool lightDeletion = false;
			LightData *light = &lights[i];

			ImGui::PushID( i );

			if ( ImGui::Button( "Remove" ) ) {
				lightToDelete = i;
			}

			const char* shader = light->materialNum == -1 ? NULL : lightMaterials[light->materialNum].c_str();

			if ( ImGui::BeginCombo( "Shader", shader, ImGuiComboFlags_None ) ) {
				for ( int n = 0; n < lightMaterials.Num(); n++ ) {
					const bool selected = ( light->materialNum == n );
					if ( ImGui::Selectable( lightMaterials[n], selected ) ) {
						light->materialNum = n;
						materialPreview->OnLightShaderChange( i, lightMaterials[n] );
					}

					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			ImGui::SetItemTooltip( "Set the light shader." );

			if ( ImGui::ColorEdit3( "Color", light->color.ToFloatPtr() ) ) {
				materialPreview->OnLightColorChange( i, light->color );
			}
			ImGui::SetItemTooltip( "Color of the light." );

			float r = light->radius;
			if ( ImGui::DragFloat( "Radius", &r, 1.0f, 0.0f, 10000.0f, "%.1f" ) ) {
				light->radius = r;
				materialPreview->OnLightRadiusChange( i, r );
			}
			ImGui::SetItemTooltip( "Radius of the light." );

			bool moveable = light->moveable;
			if ( ImGui::Checkbox( "Move light", &moveable ) ) {
				light->moveable = moveable;
				materialPreview->OnLightAllowMoveChange( i, moveable );
			}
			ImGui::SetItemTooltip( "When checked, allow light to move." );

			ImGui::PopID();
		}

		if ( lightToDelete != -1 ) {
			LightData	*light		= &lights[lightToDelete];
			int			lightNum	= light->id;

			materialPreview->OnDeleteLight( lightNum );

			lights.RemoveIndex( lightToDelete );

			for ( int j = lightToDelete; j < lights.Num(); j++) {
				lights[j].id = lightNum++;
			}
		}

		ImGui::TreePop();

		if ( ImGui::IsWindowFocused() ) {
			MaterialEditorSetActiveWindow( ME_WINDOW_PREVIEW_PROP );
		}
	}

	ImGui::EndChild();
	return false;
}

void MaterialPreviewPropView::AddLight( void ) {
	if ( !lightMaterials.Num() ) {
		const idMaterial* mat;

		// Add all light shaders to the combo box
		int count = declManager->GetNumDecls( DECL_MATERIAL );
		for ( int i = 0; i < count; i++ ) {
			mat = declManager->MaterialByIndex( i, false );

			idStr materialName = mat->GetName();
			materialName.ToLower();

			if ( materialName.Left(7) == "lights/" || materialName.Left(5) == "fogs/" ) {
				int index = lightMaterials.Append( materialName );

				if ( materialName == "lights/defaultpointlight" ) {
					defaultPointLightIndex = index;
				}
			}
		}
	}

	LightData	light;

	light.id = lights.Num();
	light.materialNum = defaultPointLightIndex;
	light.radius = 300.0f;
	light.color.Set( 1.0f, 1.0f, 1.0f );
	light.moveable = true;

	lights.Append( light );

	if ( materialPreview ) {
		materialPreview->OnAddLight();
	}
}

//Create sample data for the preview properties
void MaterialPreviewPropView::InitializePropTree( void ) {
	AddLight();
}

void MaterialPreviewPropView::RegisterPreviewView( MaterialPreviewView *view ) {
	materialPreview = view;
}

}
