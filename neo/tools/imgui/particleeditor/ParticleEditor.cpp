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

#include "ParticleEditor.h"

#include "sys/sys_imgui.h"

#include "idlib/math/Vector.h"
#include "idlib/math/Quat.h"

#include "framework/FileSystem.h"
#include "framework/Game.h"

// ParticleEditor dialog

namespace ImGuiTools
{

static const char *customPathValues[] = {
	"standard",
	"helix",
	"flies",
	"spherical",
	"drip",
	NULL
};

bool RangeSlider::Draw( const char *label, float sliderWidth ) {
	float v = GetValue();
	bool changed = false;
	idStr realLabel = label;
	realLabel += "##Slider";
	ImGui::SetNextItemWidth( sliderWidth );
	if ( ImGui::SliderFloat( realLabel.c_str(), &v, GetValueLow(), GetValueHigh() ) ) {
		SetValuePos( v );
		changed = true;
	}

	return changed;
}

ParticleEditor& ParticleEditor::Instance()
{
	static ParticleEditor instance;
	return instance;
}

ParticleEditor::ParticleEditor()
	: particleNewDlg( DECL_PARTICLE, "particles/", ".prt", "New Particle System" )
	, particleSelectDlg( DECL_PARTICLE, "Select Particle System" )
	, materialSelectDlg( DECL_MATERIAL, "Select Material" )
	, colorDlg( "Color" )
	, fadeColorDlg( "Fade Color" )
	, entityColorDlg( "Entity Color" )
	, particleDropDlg()
{
	isShown = false;
}

void ParticleEditor::Draw()
{
	int i, num;
	bool showTool;
	bool clickedNew = false, clickedSelect = false;

	showTool = isShown;

	if ( ImGui::Begin( "Particle Editor", &showTool, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar ) ) {
		impl::SetReleaseToolMouse( true );

		if( ImGui::BeginMenuBar() )
		{
			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "New", "Ctrl+N" ) )
				{
					clickedNew = true;
				}

				if( ImGui::MenuItem( "Open..", "Ctrl+O" ) )
				{
					clickedSelect = true;
				}

				if( ImGui::MenuItem( "Save", "Ctrl+S" ) )
				{
					OnBnClickedButtonSave();
				}

				if( ImGui::MenuItem( "Close", "Ctrl+W" ) )
				{
					showTool = false;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if ( clickedNew ) {
			particleNewDlg.Start();
		}

		if ( particleNewDlg.Draw() ) {
			idDeclParticle *dp = static_cast<idDeclParticle*>( particleNewDlg.GetDecl() );

			SetCurParticle( dp );
		}

		if ( clickedSelect ) {
			particleSelectDlg.Start( NULL );
		}

		if ( particleSelectDlg.Draw() ) {
			idDeclParticle *dp = static_cast<idDeclParticle*>( particleSelectDlg.GetDecl() );

			SetCurParticle( dp );
		}

		ImGui::TextUnformatted( inFileText.IsEmpty() ? "(no file opened)" : inFileText.c_str() );
		ImGui::SameLine();
		
		ImGui::SetNextItemWidth(70);
		if ( ImGui::InputTextStr( "Depth hack", &depthHack ) ) {
			CurStageToDlgVars();
			DlgVarsToCurStage();
		}

		ImGui::Text( "Stages" );
		if ( ImGui::BeginTable( "stagesTable", 2, ImGuiTableFlags_SizingFixedFit ) ) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			bool addStagePressed = ImGui::Button( "+" );
			ImGui::SetItemTooltip( "Add a new generic stage" );
			if ( addStagePressed ) {
				AddStage( false );
			}
			ImGui::SameLine();
			bool cloneStagePressed = ImGui::Button( "C" );
			ImGui::SetItemTooltip( "Clone the selected stage" );
			if ( cloneStagePressed ) {
				AddStage( true );
			}
			ImGui::SameLine();
			bool removeStagePressed = ImGui::Button( "-" );
			ImGui::SetItemTooltip( "Remove the selected stage" );
			if ( removeStagePressed ) {
				RemoveStage();
			}
			RemoveStageThink();
			ImGui::SameLine();
			bool hideStagePressed = ImGui::Button( "H" );
			ImGui::SetItemTooltip( "Hide the selected stage" );
			if ( hideStagePressed ) {
				HideStage();
			}
			ImGui::SameLine();
			bool showStagePressed = ImGui::Button( "S" );
			ImGui::SetItemTooltip( "Show the selected stage" );
			if ( showStagePressed ) {
				ShowStage();
			}
			// some additional space after [S], before the right column
			ImGui::SameLine( 0, 30 );
			ImGui::NewLine();

			if ( ImGui::BeginListBox( "##ListStages", ImVec2( 100, 150 ) ) ) {
				num = listStages.Num();
				for ( i = 0; i < num; i++ ) {
					ImGui::PushID(i);

					bool selected = ( i == listStagesSel );

					if ( ImGui::Selectable( listStages[i].c_str(), selected ) ) {
						listStagesSel = i;
						OnLbnSelchangeListStages();
					}

					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}

					ImGui::PopID();
				}
				ImGui::EndListBox();
			}

			ImGui::BeginDisabled( !stageControlsEnabled );

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(200);
			if ( ImGui::InputTextStr( "Material", &matName ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "...###BrowseMaterial" ) ) {
				materialSelectDlg.Start( matName.c_str() );
			}
			if ( materialSelectDlg.Draw() ) {
				idMaterial *material = static_cast<idMaterial *>( materialSelectDlg.GetDecl() );
				if ( material ) {
					matName = material->GetName();
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
			}
			ImGui::SetNextItemWidth(70);
			if ( ImGui::InputInt( "Anim Frames", &animFrames, 0 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			if ( ImGui::InputFloat( "Anim Rate", &animRate ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ButtonColor();

			ButtonFadeColor();

			if ( ImGui::Checkbox( "Entity Color", &entityColor ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			
			if ( sliderFadeIn.Draw( "Fade In %", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderFadeOut.Draw( "Fade Out %", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderFadeFraction.Draw( "Fade Frac", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderBunching.Draw( "Bunching", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::EndTable();
		}

		bool countPressed = sliderCount.Draw( "Count", 300 );
		ImGui::SetItemTooltip( "How many particles there are (at any moment)" );
		if ( countPressed ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		bool timePressed = sliderTime.Draw( "Time", 300 );
		ImGui::SetItemTooltip( "How long one cycle of the effect will last in seconds" );
		if ( timePressed ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		ImGui::SetNextItemWidth(50);
		bool cyclesPressed = ImGui::InputFloat( "Cycles", &cycles );
		ImGui::SetItemTooltip( "Determines how many times the particle effect will repeat (0 = infinite, 1 = single time)" );
		if ( cyclesPressed ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		if ( ImGui::InputFloat( "Time offset", &timeOffset ) ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50);
		if ( ImGui::InputFloat( "Dead time", &deadTime ) ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}

		if ( ImGui::Checkbox( "World gravity", &worldGravity ) ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		ImGui::SameLine();
		if ( sliderGravity.Draw( "Gravity", 300 ) ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}

		if ( ImGui::BeginTable( "distributionAndOrientationTable", 2, ImGuiTableFlags_SizingStretchSame ) ) {
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex( 0 );
			ImGui::SeparatorText( "Distribution" );
			if ( ImGui::BeginTable("distributionTable", 2, ImGuiTableFlags_SizingStretchSame ) ) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex( 0 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Rect", distribution == 0 ) ) {
					distribution = 0;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Cylinder", distribution == 1 ) ) {
					distribution = 1;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Sphere", distribution == 2 ) ) {
					distribution = 2;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::TextUnformatted( "Offset" );
				ImGui::SetNextItemWidth( 100 );
				if ( ImGui::InputFloat3( "###Offset", offset ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				
				ImGui::TableSetColumnIndex( 1 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::InputFloat( "XSize", &xSize ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::InputFloat( "YSize", &ySize ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::InputFloat( "ZSize", &zSize ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				ImGui::SetNextItemWidth( 70 );
				ImGui::BeginDisabled( !editRingOffsetEnabled );
				if ( ImGui::InputFloat( "Ring", &ringOffset ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				ImGui::EndDisabled();

				ImGui::EndTable();
			}
			if ( ImGui::Checkbox( "Random Distribution", &randomDistribution ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex( 1 );
			ImGui::SeparatorText( "Direction / Orientation" );
			if ( ImGui::BeginTable( "orientationTable", 3, ImGuiTableFlags_SizingFixedFit ) ) { // ImGuiTableFlags_SizingStretchSame
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex( 0 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Cone", direction == 0 ) ) {
					direction = 0;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Outward", direction == 1 ) ) {
					direction = 1;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}

				ImGui::TableSetColumnIndex( 1 );
				ImGui::SetNextItemWidth( 70 );
				bool radioViewPressed = ImGui::RadioButton( "View", orientation == 0 );
				ImGui::SetItemTooltip( "Render particle aligned to the viewer" );
				if ( radioViewPressed ) {
					orientation = 0;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::SetNextItemWidth( 70 );
				bool radioAimedPressed = ImGui::RadioButton( "Aimed", orientation == 1 );
				ImGui::SetItemTooltip( "Stretch every particle along its movement direction, drawing a trail behind particle" );
				if ( radioAimedPressed ) {
					orientation = 1;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}

				ImGui::TableSetColumnIndex( 2 );
				// empty cell

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex( 0 );
				ImGui::TextUnformatted( "Static" );
				if ( ImGui::InputFloat( "###Static", &directionParm ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}

				ImGui::TableSetColumnIndex( 1 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "X", orientation == 2 ) ) {
					orientation = 2;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Y", orientation == 3 ) ) {
					orientation = 3;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Z", orientation == 4 ) ) {
					orientation = 4;
					DlgVarsToCurStage();
					CurStageToDlgVars();
					UpdateControlInfo();
				}

				ImGui::TableSetColumnIndex( 2 );
				ImGui::BeginDisabled( !editOrientationParm1Enabled );
				bool trailsPressed = ImGui::InputFloat( "Trails", &trails );
				ImGui::SetItemTooltip( "Set to 0 for straight trail. Set to > 0 for curved trails." );
				if ( trailsPressed ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				ImGui::EndDisabled();
				ImGui::BeginDisabled( !editOrientationParm2Enabled );
				bool trailTimePressed = ImGui::InputFloat( "Time", &trailTime );
				ImGui::SetItemTooltip( "The duration in seconds of the trail" );
				if ( trailTimePressed ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				ImGui::EndDisabled();
				ImGui::EndTable();
			}

			ImGui::SetNextItemWidth( 70 );
			if ( ImGui::InputFloat( "Initial Angle", &initialAngle ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::EndTable();
		}

		if ( ImGui::BeginTable( "speedRotationSizeAspectTable", 2, ImGuiTableFlags_SizingStretchSame ) ) {
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex( 0 );
			if ( sliderSpeedFrom.Draw( "Speed From", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderSpeedTo.Draw( "Speed To", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex( 1 );
			if ( sliderRotationFrom.Draw( "Rotation From", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderRotationTo.Draw( "Rotation To", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex( 0 );
			if ( sliderSizeFrom.Draw( "Size From", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderSizeTo.Draw( "Size To", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex(1);
			if ( sliderAspectFrom.Draw( "Aspect From", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderAspectTo.Draw( "Aspect To", 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex( 0 );
			ImGui::SetNextItemWidth( 75 );
			if ( ImGui::InputFloat( "Bounds Expansion", &boundsExpansion ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex( 1 );
			ImGui::Text( "Desc:" );
			ImGui::SameLine();
			ImGui::TextUnformatted( customDesc.c_str() );

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex( 0 );
			if ( ImGui::BeginCombo( "Custom Path", customPath.c_str() ) ) {
				for ( i = 0; customPathValues[i]; i++ ) {
					ImGui::PushID(0);
					bool selected = ( customPath == customPathValues[i]);
					if ( ImGui::Selectable( customPathValues[i], selected)) {
						customPath = customPathValues[i];
						OnCbnSelchangeComboPath();
					}
					if ( selected ) {
						ImGui::SetItemDefaultFocus();
					}
					ImGui::PopID();
				}
				ImGui::EndCombo();
			}

			ImGui::TableSetColumnIndex( 1 );
			if ( ImGui::InputTextStr( "Parms", &customParms ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Apply" ) ) {
				OnBnClickedButtonUpdate();
			}

			ImGui::EndTable();
		}
		ImGui::EndDisabled();

		if ( ImGui::BeginTable( "doomStuffAndEntityEditing", 2, ImGuiTableFlags_SizingStretchSame ) ) {
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex( 0 );
			ImGui::SeparatorText( "Doom Stuff" );

			bool testModel = ImGui::Button( "T" );
			ImGui::SetItemTooltip( "Show the selected particle as a testmodel" );
			if ( testModel ) {
				visualization = TESTMODEL;
				SetParticleView();
			}
			ImGui::SameLine();
			bool impact = ImGui::Button( "I" );
			ImGui::SetItemTooltip( "Show the selected particle on projectile impact" );
			if ( impact ) {
				visualization = IMPACT;
				SetParticleView();
			}
			ImGui::SameLine();
			bool muzzle = ImGui::Button( "M" );
			ImGui::SetItemTooltip( "Show the selected particle as muzzle smoke" );
			if ( muzzle ) {
				visualization = MUZZLE;
				SetParticleView();
			}
			ImGui::SameLine();
			bool flight = ImGui::Button( "F");
			ImGui::SetItemTooltip( "Show the selected particle as projectile flight smoke" );
			if ( flight ) {
				visualization = FLIGHT;
				SetParticleView();
			}
			ImGui::SameLine();
			bool selectedEntity = ImGui::Button( "S" );
			ImGui::SetItemTooltip( "Show the selected particle on the selected entity" );
			if ( selectedEntity ) {
				visualization = SELECTED;
				SetParticleView();
			}
			ImGui::Spacing();
			bool switchToDoom = ImGui::Button( "Switch to DOOM" );
			ImGui::SetItemTooltip( "Force focus to DOOM" );
			if ( switchToDoom ) {
				OnBnClickedDoom();
			}
			
			ImGui::TableSetColumnIndex( 1 );
			ImGui::SeparatorText( "Entity Editing" );
			if ( ImGui::BeginTable( "entityEditingTable", 3, ImGuiTableFlags_SizingFixedSame ) ) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex( 0 );
				if ( ImGui::Checkbox( "Edit Mode", &particleMode ) ) {
					OnBnClickedParticleMode();
				}
				ImGui::BeginDisabled( !editControlsEnabled );
				if ( ImGui::Button( "Drop" ) ) {
					idDeclParticle *idp = GetCurParticle();
					particleDropDlg.Start( idp );
				}
				particleDropDlg.Draw();

				ImGui::TableSetColumnIndex( 1 );
				if ( ImGui::BeginTable( "entityPosTableXY", 3, ImGuiTableFlags_SizingFixedSame ) ) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					ImGui::TableSetColumnIndex( 1 );
					if ( ImGui::Button( "Y+" ) ) {
						OnBtnYup();
					}
					ImGui::TableSetColumnIndex( 2 );
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					if ( ImGui::Button( "X-" ) ) {
						OnBtnXdn();
					}
					ImGui::TableSetColumnIndex( 1 );
					ImGui::TableSetColumnIndex( 2 );
					if ( ImGui::Button( "X+" ) ) {
						OnBtnXup();
					}
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					ImGui::TableSetColumnIndex( 1 );
					if ( ImGui::Button( "Y-" ) ) {
						OnBtnYdn();
					}
					ImGui::TableSetColumnIndex( 2 );

					ImGui::EndTable();
				}
				ImGui::TableSetColumnIndex( 2 );
				if ( ImGui::BeginTable( "entityPosTableZ", 1 ) ) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					if ( ImGui::Button( "Z+" ) ) {
						OnBtnZup();
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					if ( ImGui::Button( "Z-" ) ) {
						OnBtnZdn();
					}

					ImGui::EndTable();
				}

				ImGui::EndDisabled();

				ImGui::EndTable();
			}

			ImGui::Spacing();
			if ( ImGui::Button( "Vector" ) ) { // TODO: what is this for?
			}

			ImGui::SameLine();

			ButtonEntityColor();

			ImGui::SameLine(0, 60);

			bool saveMap = ImGui::Button( "Save .MAP" );
			ImGui::SetItemTooltip( "Save the current map with any updated particle emitter entities" );
			if ( saveMap ) {
				OnBnClickedButtonSaveParticles();
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();

	if ( isShown && !showTool )
	{
		isShown = showTool;
		impl::SetReleaseToolMouse( false );
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_ParticleEditor );
	}
}

// ParticleEditor message handlers

void ParticleEditor::OnBnClickedParticleMode() {
	cvarSystem->SetCVarInteger( "g_editEntityMode", ( particleMode ) ? 4 : 0 );
	EnableEditControls();
}

void ParticleEditor::OnBnClickedButtonSaveParticles() {
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "saveParticles" );
	buttonSaveParticleEntitiesEnabled = false;
}

void ParticleEditor::ButtonColor() {
	colorDlg.Button( color );
	ImGui::SetItemTooltip( "Color that the material will be multiplied by" );
	ImGui::SameLine();
	ImGui::TextUnformatted( "Color" );

	if ( colorDlg.Draw() ) {
		idParticleStage *ps = GetCurStage();
		if ( ps ) {
			color = colorDlg.GetColor();
			color.w = 1.0f;
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
	}
}

void ParticleEditor::ButtonEntityColor() {
	idList<idEntity*> list;
	idDict dict2;
	idStr str;
	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );
	idVec3 selectedEntityColor = vec3_origin;

	if ( count ) {
		const idDict *dict = gameEdit->EntityGetSpawnArgs( list[0] );
		if ( dict ) {
			selectedEntityColor = dict->GetVector( "_color", "1 1 1" );
		}
	}

	entityColorDlg.Button( idVec4( selectedEntityColor.x, selectedEntityColor.y, selectedEntityColor.z, 1.0f ) );
	
	if ( entityColorDlg.Draw() ) {
		if ( count ) {
			idVec3 clr = entityColorDlg.GetColor().ToVec3();

			for ( int i = 0; i < count; i++ ) {
				const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
				const char *name = dict->GetString( "name" );
				idEntity *ent = gameEdit->FindEntity( name );
				if ( ent ) {
					gameEdit->EntitySetColor( ent, clr );
					str = va( "%f %f %f", clr.x, clr.y, clr.z );
					dict2.Clear();
					dict2.Set( "_color", str );
					gameEdit->EntityChangeSpawnArgs( ent, &dict2 );
					gameEdit->MapSetEntityKeyVal( name, "_color",  str );
				}
			}
			buttonSaveParticleEntitiesEnabled = true;
		}
	}
}

void ParticleEditor::ButtonFadeColor() {
	fadeColorDlg.Button( fadeColor );
	ImGui::SetItemTooltip( "Material will be multiplied by the color of a fully faded out particle" );
	ImGui::SameLine();
	ImGui::TextUnformatted( "Fade Color" );

	if ( fadeColorDlg.Draw() ) {
		idParticleStage *ps = GetCurStage();
		if ( ps ) {
			fadeColor = fadeColorDlg.GetColor();
			fadeColor.w = 1.0f;
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
	}
}

void ParticleEditor::OnBnClickedButtonUpdate() {
	DlgVarsToCurStage();
	CurStageToDlgVars();
}

void ParticleEditor::SelectParticle( const char *name ) {
	idDeclParticle *decl = static_cast<idDeclParticle*>( const_cast<idDecl*>( declManager->FindType( DECL_PARTICLE, name, false ) ) );
	
	SetCurParticle( decl );
}

void ParticleEditor::SetCurParticle( idDeclParticle *dp ) {
	curParticle = dp;
	UpdateParticleData();
}

idDeclParticle *ParticleEditor::GetCurParticle() {
	return curParticle;
}

void ParticleEditor::UpdateParticleData() {

	listStages.Clear();
	listStagesItemData.Clear();
	listStagesSel = -1;
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	listStages.Resize( idp->stages.Num() );
	for ( int i = 0; i < idp->stages.Num(); i++ ) {
		int index = listStages.Append( idStr::Format( "stage %i", i ) );
		if ( index >= 0 ) {
			listStagesItemData.Add( index, i );
		}
	}
	listStagesSel = 0;
	OnLbnSelchangeListStages();
	inFileText = idStr::Format( "%s (%s)", idp->GetName(), idp->GetFileName() );

	SetParticleView();
}

void ParticleEditor::OnCbnSelchangeComboPath() {
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::UpdateControlInfo() {
	editRingOffsetEnabled = ( distribution == 2 );
	dirParmText = (direction == 0 ) ? "Angle" : "Upward Bias";
	editOrientationParm1Enabled = ( orientation == 1 );
	editOrientationParm2Enabled = ( orientation == 1 );

	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	sliderBunching.SetValuePos( ps->spawnBunching );
	sliderFadeIn.SetValuePos( ps->fadeInFraction );
	sliderFadeOut.SetValuePos( ps->fadeOutFraction );
	sliderFadeFraction.SetValuePos( ps->fadeIndexFraction );
	sliderCount.SetValuePos( ps->totalParticles );
	sliderTime.SetValuePos( ps->particleLife );
	sliderGravity.SetValuePos( ps->gravity );
	sliderSpeedFrom.SetValuePos( ps->speed.from );
	sliderSpeedTo.SetValuePos( ps->speed.to );
	sliderRotationFrom.SetValuePos( ps->rotationSpeed.from );
	sliderRotationTo.SetValuePos( ps->rotationSpeed.to );
	sliderSizeFrom.SetValuePos( ps->size.from );
	sliderSizeTo.SetValuePos( ps->size.to );
	sliderAspectFrom.SetValuePos( ps->aspect.from );
	sliderAspectTo.SetValuePos( ps->aspect.to );
}

void ParticleEditor::OnBnClickedDoom() {
	//::SetFocus(win32.hWnd);
}

void ParticleEditor::SetParticleVisualization( int i ) {
	visualization = i;
	SetParticleView();
}

void ParticleEditor::SetParticleView() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "testmodel" );
	idStr str;
	switch ( visualization ) {
		case TESTMODEL :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, va("testmodel %s\n", str.c_str() ) );
		break;
		case IMPACT :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_IMPACT );
			cvarSystem->SetCVarString( "g_testParticleName", str );
		break;
		case MUZZLE :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_MUZZLE );
			cvarSystem->SetCVarString( "g_testParticleName", str );
		break;
		case FLIGHT :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_FLIGHT );
			cvarSystem->SetCVarString( "g_testParticleName", str );
		break;
		case SELECTED :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_FLIGHT );
			SetSelectedModel( str );
		break;
	}
}

void ParticleEditor::SetSelectedModel( const char *val ) {
	idList<idEntity*> list;
	idMat3 axis;

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		for ( int i = 0; i < count; i++ ) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			gameEdit->EntitySetModel( list[i], val );
			gameEdit->EntityUpdateVisuals( list[i] );
			gameEdit->EntityGetAxis( list[i], axis );
			//vectorControl.SetidAxis( axis );
			gameEdit->MapSetEntityKeyVal( name, "model", val );
		}
		buttonSaveParticleEntitiesEnabled = true;
	}
}

void ParticleEditor::AddStage( bool clone ) {

	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	idParticleStage *stage = new idParticleStage;

	if ( clone ) {
		idParticleStage *source = GetCurStage();
		if ( source == NULL ) {
			delete stage;
			return;
		}
		*stage = *source;
	} else {
		stage->Default();
	}
	int newIndex = idp->stages.Append( stage );
	int index = listStages.Append( idStr::Format( "stage %i", newIndex ) );
	listStagesSel = index;
	listStagesItemData.Add( index, newIndex );
	ShowCurrentStage();
	EnableStageControls();
}

void ParticleEditor::RemoveStage() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	ImGui::OpenPopup( "Remove stage?" );
}

void ParticleEditor::RemoveStageThink()
{
	bool accepted = false;
	bool canceled = false;

	if ( ImGui::BeginPopupModal( "Remove stage?", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "Are you sure you want to remove this stage?" );

		if ( ImGui::Button( "OK" ) ) {
			accepted = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			canceled = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if ( canceled ) {
		return;
	}
	if ( !accepted ) {
		return;
	}

	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	int index = listStagesSel;
	if ( index >= 0 ) {
		int newIndex = listStagesItemData.First( index );
		if ( newIndex >= 0 && newIndex < idp->stages.Num() ) {
			listStages.RemoveIndex( index );
			listStagesItemData.Remove( index, newIndex );
			listStagesSel = index;
			idp->stages.RemoveIndex( newIndex );
			UpdateParticleData();
			ShowCurrentStage();
		}
	}
	EnableStageControls();
}

void ParticleEditor::ShowStage() {
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	ps->hidden = false;
	int index = listStagesSel;
	int newIndex = listStagesItemData.First( index );
	listStages.RemoveIndex( index );
	listStagesItemData.Remove( index, newIndex );
	listStages.Insert( idStr::Format("stage %i", index ), index );
	listStagesItemData.Add( index, newIndex );
	listStagesSel = index;
	EnableStageControls();
}

void ParticleEditor::HideStage() {
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	ps->hidden = true;
	int index = listStagesSel;
	int newIndex = listStagesItemData.First( index );
	listStages.RemoveIndex( index );
	listStagesItemData.Remove( index, newIndex );
	listStages.Insert( idStr::Format("stage %i (H) ", index ), index );
	listStagesItemData.Add( index, newIndex );
	listStagesSel = index;
	EnableStageControls();
}

idParticleStage *ParticleEditor::GetCurStage() {
	idDeclParticle *idp = GetCurParticle();
	int sel = listStagesSel;
	int index = listStagesItemData.First( sel );
	if ( idp == NULL || index < 0 || index >= idp->stages.Num() ) {
		return NULL;
	}
	return idp->stages[index];
}

void ParticleEditor::ClearDlgVars() {
	matName.Clear();
	color = idVec4(0, 0, 0, 1);
	fadeColor = idVec4(0, 0, 0, 1);
	offset[0] = 0;
	offset[1] = 0;
	offset[2] = 0;
	direction = 1;
	orientation = 1;
	distribution = 1;
	customPath.Clear();
	customParms.Clear();
	worldGravity = false;
	entityColor = false;
	randomDistribution = true;
	customDesc.Clear();
}

void ParticleEditor::CurStageToDlgVars() {

	// go ahead and get the two system vars too
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	depthHack = va( "%.3f", idp->depthHack );

	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	matName = ps->material->GetName();
	animFrames = ps->animationFrames;
	animRate = ps->animationRate;
	color.x = ps->color.x;
	color.y = ps->color.y;
	color.z = ps->color.z;
	color.w = ps->color.w;
	fadeColor.x = ps->fadeColor.x;
	fadeColor.y = ps->fadeColor.y;
	fadeColor.z = ps->fadeColor.z;
	fadeColor.w = ps->fadeColor.w;
	sliderFadeIn.SetValuePos( ps->fadeInFraction );
	sliderFadeOut.SetValuePos( ps->fadeOutFraction );
	sliderFadeFraction.SetValuePos( ps->fadeIndexFraction );
	sliderCount.SetValuePos( ps->totalParticles );
	sliderTime.SetValuePos( ps->particleLife );
	timeOffset = ps->timeOffset;
	deadTime = ps->deadTime;
	sliderGravity.SetValuePos( ps->gravity );
	sliderBunching.SetValuePos( ps->spawnBunching );
	offset[0] = ps->offset.x;
	offset[1] = ps->offset.y;
	offset[2] = ps->offset.z;
	xSize = ps->distributionParms[0];
	ySize = ps->distributionParms[1];
	zSize = ps->distributionParms[2];
	ringOffset = ps->distributionParms[3];
	directionParm = ps->directionParms[0];
	direction = ps->directionType;
	orientation = ps->orientation;
	distribution = ps->distributionType;
	sliderSpeedFrom.SetValuePos( ps->speed.from );
	sliderSpeedTo.SetValuePos( ps->speed.to );
	sliderRotationFrom.SetValuePos( ps->rotationSpeed.from );
	sliderRotationTo.SetValuePos( ps->rotationSpeed.to );
	sliderSizeFrom.SetValuePos( ps->size.from );
	sliderSizeTo.SetValuePos( ps->size.to );
	sliderAspectFrom.SetValuePos( ps->aspect.from );
	sliderAspectTo.SetValuePos( ps->aspect.to );
	trails = ps->orientationParms[0];
	trailTime = ps->orientationParms[1];
	cycles = ps->cycles;
	customPath = ps->GetCustomPathName();
	customDesc = ps->GetCustomPathDesc();
	customParms.Clear();
	if ( ps->customPathType != PPATH_STANDARD ) {
		for ( int i = 0; i < ps->NumCustomPathParms(); i++ ) {
			customParms += va( "%.1f ", ps->customPathParms[i] );
		}
	}
	worldGravity = ps->worldGravity;
	initialAngle = ps->initialAngle;
	boundsExpansion = ps->boundsExpansion;
	randomDistribution = ps->randomDistribution;
	entityColor = ps->entityColor;
}

void ParticleEditor::DlgVarsToCurStage() {

	// go ahead and set the two system vars too
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	idp->depthHack = atof( depthHack );

	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	ps->material = declManager->FindMaterial( matName );
	ps->animationFrames = animFrames;
	ps->animationRate = animRate;
	ps->color.x = color.x;
	ps->color.y = color.y;
	ps->color.z = color.z;
	ps->color.w = color.w;
	ps->fadeColor.x = fadeColor.x;
	ps->fadeColor.y = fadeColor.y;
	ps->fadeColor.z = fadeColor.z;
	ps->fadeColor.w = fadeColor.w;
	ps->fadeInFraction = sliderFadeIn.GetValue();
	ps->fadeOutFraction = sliderFadeOut.GetValue();
	ps->fadeIndexFraction = sliderFadeFraction.GetValue();
	ps->totalParticles = sliderCount.GetValue();
	ps->particleLife = sliderTime.GetValue();
	ps->timeOffset = timeOffset;
	ps->deadTime = deadTime;
	ps->gravity = sliderGravity.GetValue();
	ps->spawnBunching = sliderBunching.GetValue();
	ps->offset.x = offset[0];
	ps->offset.y = offset[1];
	ps->offset.z = offset[2];
	ps->distributionParms[0] = xSize;
	ps->distributionParms[1] = ySize;
	ps->distributionParms[2] = zSize;
	ps->distributionParms[3] = ringOffset;
	ps->directionParms[0] = directionParm;
	ps->directionType = static_cast<prtDirection_t>( direction );
	ps->orientation = static_cast<prtOrientation_t>( orientation );
	ps->distributionType = static_cast<prtDistribution_t>( distribution );
	ps->speed.from = sliderSpeedFrom.GetValue();
	ps->speed.to = sliderSpeedTo.GetValue();
	ps->rotationSpeed.from = sliderRotationFrom.GetValue();
	ps->rotationSpeed.to = sliderRotationTo.GetValue();
	ps->size.from = sliderSizeFrom.GetValue();
	ps->size.to = sliderSizeTo.GetValue();
	ps->aspect.from = sliderAspectFrom.GetValue();
	ps->aspect.to = sliderAspectTo.GetValue();
	ps->orientationParms[0] = trails;
	ps->orientationParms[1] = trailTime;
	ps->worldGravity = worldGravity;
	ps->cycles = cycles;
	ps->cycleMsec = ( ps->particleLife + ps->deadTime ) * 1000;

	sscanf( customParms, "%f %f %f %f %f %f %f %f", &ps->customPathParms[0], &ps->customPathParms[1], &ps->customPathParms[2],
		   &ps->customPathParms[3], &ps->customPathParms[4], &ps->customPathParms[5],
		   &ps->customPathParms[6], &ps->customPathParms[7] );

	ps->SetCustomPathType( customPath );

	ps->initialAngle = initialAngle;
	ps->boundsExpansion = boundsExpansion;
	ps->randomDistribution = randomDistribution;
	ps->entityColor = entityColor;

}

void ParticleEditor::ShowCurrentStage() {
	ClearDlgVars();
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnLbnSelchangeListStages() {
	ShowCurrentStage();
	EnableStageControls();
}

void ParticleEditor::OnBnClickedButtonSave() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	idp->Save();
}

/*void VectorCallBack(idQuat rotation) {
	ParticleEditor::Instance().SetVectorControlUpdate( rotation );
}*/

void ParticleEditor::SetVectorControlUpdate( idQuat rotation ) {
	if ( particleMode ) {
		idList<idEntity*> list;

		list.SetNum( 128 );
		int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
		list.SetNum( count );

		if ( count ) {
			for ( int i = 0; i < count; i++ ) {
				const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
				if ( dict == NULL ) {
					continue;
				}
				const char *name = dict->GetString( "name" );
				gameEdit->EntitySetAxis( list[i], rotation.ToMat3() );
				gameEdit->EntityUpdateVisuals( list[i] );
				gameEdit->MapSetEntityKeyVal( name, "rotation", rotation.ToMat3().ToString() );
			}
			buttonSaveParticleEntitiesEnabled = true;
		}
	}
}

void ParticleEditor::Reset()
{
	particleMode = ( cvarSystem->GetCVarInteger( "g_editEntityMode" ) == 4 );
	mapModified = false;

	sliderBunching.SetRange( 0, 20 );
	sliderBunching.SetValueRange( 0.0f, 1.0f );
	sliderFadeIn.SetRange( 0, 20 );
	sliderFadeIn.SetValueRange( 0.0f, 1.0f );
	sliderFadeOut.SetRange( 0, 20 );
	sliderFadeOut.SetValueRange( 0.0f, 1.0f );
	sliderCount.SetRange( 0, 1024 );
	sliderCount.SetValueRange( 0, 4096 );
	sliderTime.SetRange( 0, 200 );
	sliderTime.SetValueRange( 0.0f, 10.0f );
	sliderGravity.SetRange( 0, 600 );
	sliderGravity.SetValueRange( -300.0f, 300.0f );
	sliderSpeedFrom.SetRange( 0, 600 );
	sliderSpeedFrom.SetValueRange( -300.0f, 300.0f );
	sliderSpeedTo.SetRange( 0, 600 );
	sliderSpeedTo.SetValueRange( -300.0f, 300.0f );
	sliderRotationFrom.SetRange( 0, 100 );
	sliderRotationFrom.SetValueRange( 0.0f, 100.0f );
	sliderRotationTo.SetRange( 0, 100 );
	sliderRotationTo.SetValueRange( 0.0f, 100.0f );
	sliderSizeFrom.SetRange( 0, 256 );
	sliderSizeFrom.SetValueRange( 0.0f, 128.0f );
	sliderSizeTo.SetRange( 0, 256 );
	sliderSizeTo.SetValueRange( 0.0f, 128.0f );
	sliderAspectFrom.SetRange( 0, 256 );
	sliderAspectFrom.SetValueRange( 0.0f, 128.0f );
	sliderAspectTo.SetRange( 0, 256 );
	sliderAspectTo.SetValueRange( 0.0f, 128.0f );
	sliderFadeFraction.SetRange( 0, 20 );
	sliderFadeFraction.SetValueRange( 0.0f, 1.0f );

	UpdateParticleData();
	SetParticleView();

	buttonSaveParticleEntitiesEnabled = false;
	EnableEditControls();

	//vectorControl.SetVectorChangingCallback( VectorCallBack );
}

void ParticleEditor::EnableStageControls() {
	idParticleStage *stage = GetCurStage();
	bool b = ( stage && stage->hidden ) ? false : true;

	stageControlsEnabled = b;
}

void ParticleEditor::EnableEditControls() {
	editControlsEnabled = particleMode;
}

void ParticleEditor::UpdateSelectedOrigin( float x, float y, float z ) {
	idList<idEntity*> list;
	idVec3 origin;
	idVec3 vec(x, y, z);

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		for ( int i = 0; i < count; i++ ) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			gameEdit->EntityTranslate( list[i], vec );
			gameEdit->EntityUpdateVisuals( list[i] );
			gameEdit->MapEntityTranslate( name, vec );
		}
		buttonSaveParticleEntitiesEnabled = true;
	}
}

void ParticleEditor::OnBtnYup()
{
	UpdateSelectedOrigin(0, 8, 0);
}

void ParticleEditor::OnBtnYdn()
{
	UpdateSelectedOrigin(0, -8, 0);
}

void ParticleEditor::OnBtnXdn()
{
	UpdateSelectedOrigin(-8, 0, 0);
}

void ParticleEditor::OnBtnXup()
{
	UpdateSelectedOrigin(8, 0, 0);
}

void ParticleEditor::OnBtnZup()
{
	UpdateSelectedOrigin(0, 0, 8);
}

void ParticleEditor::OnBtnZdn()
{
	UpdateSelectedOrigin(0, 0, -8);
}

ParticleDrop::ParticleDrop()
	: classname( "" )
	, org( 0.0f, 0.0f, 0.0f )
	, args()
	, viewAngles( 0.0f, 0.0f, 0.0f )
	, errorText( "" )
	, name( "" )
	, state( DONE )
{

}

void ParticleDrop::Start( idDeclParticle *idp )
{
	if ( !idp ) {
		return;
	}

	if ( !gameEdit->PlayerIsValid() ) {
		return;
	}

	gameEdit->PlayerGetViewAngles( viewAngles );
	gameEdit->PlayerGetEyePosition( org );

	org += idAngles( 0, viewAngles.yaw, 0 ).ToForward() * 80 + idVec3( 0, 0, 1 );
	args.Clear();
	args.Set( "origin", org.ToString() );
	args.Set( "classname", "func_emitter");
	args.Set( "angle", va( "%f", viewAngles.yaw + 180 ) );

	if ( idp == NULL ) {
		return;
	}

	idStr str = idp->GetName();
	str.SetFileExtension( ".prt" );

	args.Set( "model", str );

	name = gameEdit->GetUniqueEntityName( "func_emitter" );
	state = NAME;
	errorText.Clear();

	ImGui::OpenPopup( "Name particle" );
}

void ParticleDrop::Draw()
{
	if ( state == DONE ) {
		return;
	}

	bool accepted = false;
	bool canceled = false;

	if ( ImGui::BeginPopupModal( "Name particle", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "%s", errorText.c_str() );

		if ( ImGui::InputTextStr( "Name", &name ) ) {
			// nop
		}

		if ( ImGui::Button( "OK" ) ) {
			accepted = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			canceled = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if ( canceled ) {
		state = DONE;
		return;
	}
	if ( !accepted ) {
		return;
	}

	idEntity *gameEnt = gameEdit->FindEntity( name );
	if ( gameEnt ) {
		errorText = "Please choose another name";
		return;
	}

	args.Set( "name", name.c_str() );

	idEntity *ent = NULL;
	gameEdit->SpawnEntityDef( args, &ent );
	if ( ent ) {
		gameEdit->EntityUpdateChangeableSpawnArgs( ent, NULL );
		gameEdit->ClearEntitySelection();
		gameEdit->AddSelectedEntity( ent );
	}

	gameEdit->MapAddEntity( &args );
	state = DONE;
}

}
