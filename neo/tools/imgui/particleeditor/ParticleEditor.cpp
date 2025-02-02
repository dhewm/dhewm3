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

#include "ParticleEditor.h"

#include "sys/sys_imgui.h"
#include "../util/ImGui_IdWidgets.h"

#include "idlib/math/Vector.h"
#include "idlib/math/Quat.h"

#include "framework/FileSystem.h"
#include "framework/Game.h"

// ParticleEditor dialog

namespace ImGuiTools
{

bool ColorPicker::Draw( const char *label, float *r, float *g, float *b, float *a ) {
	idStr realLabel;
	float rgba_ref[4] = { *r, *g, *b, *a };
	float rgba[4] = { *r, *g, *b, *a };
	bool isAccepted = false;

	realLabel = label;
	realLabel += "Popup";

	ImGui::OpenPopup( realLabel.c_str() );

	if ( ImGui::BeginPopupModal( realLabel.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		realLabel = label;
		realLabel += "Picker";

		bool changed = ImGui::ColorPicker4( realLabel.c_str(), rgba, ImGuiColorEditFlags_AlphaBar, rgba_ref );

		if ( ImGui::Button( "OK" ) ) {
			isAccepted = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			isAccepted = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return isAccepted;
}

bool RangeSlider::Draw( const char *label, float itemWidth, float sliderWidth ) {
	float v = GetValue();
	idStr realLabel;
	bool changed = false;

	realLabel += "##Input";
	realLabel += label;

	ImGui::SetNextItemWidth( itemWidth );
	if ( ImGui::InputFloat( realLabel.c_str(), &v ) ) {
		SetValuePos( v );
		changed = true;
	}
	ImGui::SameLine();

	realLabel.Clear();
	realLabel += label;
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
{
	isShown = false;
}

void ParticleEditor::Draw()
{
	int i, num;
	const char* comboPreviewValue = comboParticleSel >= 0 ? comboParticle[comboParticleSel].c_str() : nullptr;
	bool showTool;

	showTool = isShown;

	if ( ImGui::Begin( "Particle Editor", &showTool, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		if ( ImGui::BeginCombo( "Particle", comboPreviewValue ) ) {
			num = comboParticle.Num();
			for ( i = 0; i < num; i++ ) {
				ImGui::PushID(i);
				bool selected = ( comboParticleSel == i );

				if ( ImGui::Selectable( comboParticle[i].c_str(), selected ) ) {
					comboParticleSel = i;
					OnCbnSelchangeComboParticles();
				}

				if ( selected ) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		ButtonNew();

		ImGui::SameLine();
		if ( ImGui::Button( "Save" ) ) {
			OnBnClickedButtonSave();
		}

		ImGui::TextUnformatted( inFileText.c_str() );
		ImGui::SameLine();
		
		if ( ImGui::InputTextStr( "Depth hack", &depthHack ) ) {
			CurStageToDlgVars();
			DlgVarsToCurStage();
		}
		ImGui::SameLine();
		ButtonSaveAs();

		ImGui::Text( "Stages" );
		if ( ImGui::BeginTable( "stagesTable", 2 ) ) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			if ( ImGui::Button( "+" ) ) {
				AddStage();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "-" ) ) {
				RemoveStage();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "H" ) ) {
				HideStage();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "S" ) ) {
				ShowStage();
			}

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
			ImGui::SetNextItemWidth(300);
			if ( ImGui::InputTextStr( "Material", &matName ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "...###BrowseMaterial" ) ) {
				OnBnClickedButtonBrowsematerial();
			}
			ImGui::SetNextItemWidth(50);
			if ( ImGui::InputInt( "Anim Frames", &animFrames, ImGuiInputTextFlags_CharsDecimal ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			if ( ImGui::InputInt( "Anim Rate", &animRate, ImGuiInputTextFlags_CharsDecimal) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			/*
			* color
			* fadecolor
			* entity color checkbox
			*/
			//OnBnClickedButtonBrowsecolor
			//OnBnClickedButtonBrowsefadecolor
			//OnBnClickedEntityColor

			
			if ( sliderFadeIn.Draw( "Fade In %", 50, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderFadeOut.Draw( "Fade Out %", 50, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderFadeFraction.Draw( "Fade Frac", 50, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderBunching.Draw( "Bunching", 50, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::EndTable();
		}

		if ( sliderCount.Draw( "Count", 50, 300 ) ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		if ( sliderTime.Draw( "Time", 50, 300 ) ) {
			DlgVarsToCurStage();
			CurStageToDlgVars();
		}
		ImGui::SetNextItemWidth(50);
		if ( ImGui::InputFloat( "Cycles", &cycles ) ) {
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
			OnBnClickedWorldGravity();
		}
		ImGui::SameLine();
		if ( sliderGravity.Draw( "Gravity", 50, 300 ) ) {
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
					OnBnClickedRadioRect();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Cylinder", distribution == 1 ) ) {
					OnBnClickedRadioCylinder();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Sphere", distribution == 2 ) ) {
					OnBnClickedRadioSphere();
				}
				ImGui::TextUnformatted( "Offset" );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::InputTextStr( "###Offset", &offset ) ) {
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
				if ( ImGui::InputFloat( "Ring", &ringOffset ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}

				ImGui::EndTable();
			}
			if ( ImGui::Checkbox( "Random Distribution", &randomDistribution ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex( 1 );
			ImGui::SeparatorText( "Direction / Orientation" );
			if ( ImGui::BeginTable( "orientationTable", 3, ImGuiTableFlags_SizingStretchSame) ) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex( 0 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Cone", direction == 0 ) ) {
					OnBnClickedRadioCone();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Outward", direction == 1 ) ) {
					OnBnClickedRadioOutward();
				}

				ImGui::TableSetColumnIndex( 1 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "View", orientation == 0 ) ) {
					OnBnClickedRadioView();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Aimed", orientation == 1 ) ) {
					OnBnClickedRadioAimed();
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
					OnBnClickedRadioX();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Y", orientation == 3 ) ) {
					OnBnClickedRadioY();
				}
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::RadioButton( "Z", orientation == 4 ) ) {
					OnBnClickedRadioZ();
				}

				ImGui::TableSetColumnIndex( 2 );
				if ( ImGui::InputFloat( "Trails", &trails ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}
				if ( ImGui::InputFloat( "Time", &trailTime ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex( 0 );
				ImGui::SetNextItemWidth( 70 );
				if ( ImGui::InputFloat( "Initial Angle", &initialAngle ) ) {
					DlgVarsToCurStage();
					CurStageToDlgVars();
				}

				ImGui::EndTable();
			}

			ImGui::EndTable();
		}

		if ( ImGui::BeginTable( "speedRotationSizeAspectTable", 2, ImGuiTableFlags_SizingStretchSame ) ) {
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex( 0 );
			if ( sliderSpeedFrom.Draw( "Speed From", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderSpeedTo.Draw( "Speed To", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex( 1 );
			if ( sliderRotationFrom.Draw( "Rotation From", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderRotationTo.Draw( "Rotation To", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex( 0 );
			if ( sliderSizeFrom.Draw( "Size From", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderSizeTo.Draw( "Size To", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}

			ImGui::TableSetColumnIndex(1);
			if ( sliderAspectFrom.Draw( "Aspect From", 75, 150 ) ) {
				DlgVarsToCurStage();
				CurStageToDlgVars();
			}
			if ( sliderAspectTo.Draw( "Aspect To", 75, 150 ) ) {
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
			if ( ImGui::InputTextStr( "Custom Path", &customPath ) ) {
				OnCbnSelchangeComboPath();
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

			if ( ImGui::Button( "T" ) ) {
				OnBnClickedTestModel();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "I" ) ) {
				OnBnClickedImpact();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "M" ) ) {
				OnBnClickedMuzzle();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "F" ) ) {
				OnBnClickedFlight();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "S" ) ) {
				OnBnClickedSelected();
			}
			if ( ImGui::Button( "Switch to DOOM" ) ) {
				OnBnClickedDoom();
			}
			
			ImGui::TableSetColumnIndex( 1 );
			ImGui::SeparatorText( "Entity Editing" );
			if ( ImGui::BeginTable( "entityEditingTable", 5, ImGuiTableFlags_SizingFixedSame ) ) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex( 0 );
				if ( ImGui::Checkbox( "Edit Mode", &particleMode ) ) {
					OnBnClickedParticleMode();
				}
				ImGui::BeginDisabled( !editControlsEnabled );
				if (ImGui::Button("Drop")) {
					OnBtnDrop();
				}

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

				ImGui::TableSetColumnIndex( 3 );
				if ( ImGui::Button( "Vector" ) ) {
				}
				
				ImGui::TableSetColumnIndex( 4 );
				if ( ImGui::Button( "Color" ) ) {
					OnBnClickedButtonBrowseEntitycolor();
				}

				if ( ImGui::Button( "Save .MAP" ) ) {
					OnBnClickedButtonSaveParticles();
				}
				ImGui::EndDisabled();

				ImGui::EndTable();
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();

	if ( isShown && !showTool )
	{
		isShown = showTool;
		impl::SetReleaseToolMouse(false);
		D3::ImGuiHooks::CloseWindow(D3::ImGuiHooks::D3_ImGuiWin_ParticleEditor);
	}
}

// ParticleEditor message handlers

void ParticleEditor::OnBnClickedParticleMode() {
	cvarSystem->SetCVarInteger( "g_editEntityMode", ( particleMode ) ? 4 : 0 );
	EnableEditControls();
}


void ParticleEditor::ButtonSaveAs() {
	idDeclParticle *idp = GetCurParticle();

	if ( ImGui::Button( "Save As" ) ) {
		ImGui::OpenPopup( "New Particle" );
	}

	bool accepted = false;
	idStr name;

	if ( ImGui::BeginPopupModal( "New Particle", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::InputTextStr( "Name", &name );
		if ( ImGui::Button( "OK" ) ) {
			accepted = true;
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			accepted = false;
		}
		ImGui::EndPopup();
	}

	if ( idp && accepted ) {
		if ( declManager->FindType( DECL_PARTICLE, name, false ) ) {
			ImGui::OpenPopup( "Particle exists" );
			if ( ImGui::BeginPopupModal( "Particle exists" ) ) {
				ImGui::TextUnformatted("Particle already exists!");
				ImGui::EndPopup();
			}
			return;
		}

		ImGui::OpenPopup( "Name###SaveAs" );

		idStr fileName;
		if ( ImGui::InputDialogName( "New particle", "Name", &fileName ) ) {
			idStr particleName = fileName;
			particleName.StripPath();
			particleName.StripFileExtension();

			idDeclParticle *decl = dynamic_cast<idDeclParticle*>( declManager->CreateNewDecl( DECL_PARTICLE, particleName, fileName ) );
			if ( decl ) {
				decl->stages.DeleteContents( true );
				decl->depthHack = idp->depthHack;
				for ( int i = 0; i < idp->stages.Num(); i++ ) {
					idParticleStage *stage = new idParticleStage();
					*stage = *idp->stages[i];
					decl->stages.Append( stage );
				}
				int index = comboParticle.Append( particleName );
				comboParticleSel = index;

				OnBnClickedButtonSave();
				OnCbnSelchangeComboParticles();
				OnBnClickedButtonUpdate();
			}
		}
	}
}


void ParticleEditor::OnBnClickedButtonSaveParticles() {
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "saveParticles" );
	buttonSaveParticleEntitiesEnabled = false;
}

void ParticleEditor::OnBnClickedButtonBrowsematerial() {
	// FIXME: port this
	/*CPreviewDlg matDlg(this);
	matDlg.SetMode(CPreviewDlg::MATERIALS, "particles" );
	matDlg.SetDisablePreview( true );
	if ( matDlg.DoModal() == IDOK ) {
		matName = matDlg.mediaName;
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}*/
}

void ParticleEditor::OnBnClickedButtonBrowsecolor() {
	float r, g, b;
	float ob;
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	r = ps->color.x;
	g = ps->color.y;
	b = ps->color.z;
	ob = 1.0f;

	if ( colorPickerDlg.Draw( "##Browsecolor", &r, &g, &b, &ob ) ) {
		color.Format( "%f %f %f %f", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}
}

void ParticleEditor::OnBnClickedButtonBrowseEntitycolor() {
	float r, g, b;
	float ob;
	idList<idEntity*> list;
	idDict dict2;
	idStr str;
	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		const idDict *dict = gameEdit->EntityGetSpawnArgs( list[0] );
		if ( dict ) {
			idVec3 clr = dict->GetVector( "_color", "1 1 1" );
			r = clr.x;
			g = clr.y;
			b = clr.z;
			ob = 1.0f;
			if ( colorPickerDlg.Draw( "###browseEntitycolor", &r, &g, &b, &ob ) ) {
				for ( int i = 0; i < count; i++ ) {
					dict = gameEdit->EntityGetSpawnArgs( list[i] );
					const char *name = dict->GetString( "name" );
					idEntity *ent = gameEdit->FindEntity( name );
					if ( ent ) {
						gameEdit->EntitySetColor( ent, idVec3( r, g, b ) );
						str = va( "%f %f %f", r, g, b );
						dict2.Clear();
						dict2.Set( "_color", str );
						gameEdit->EntityChangeSpawnArgs( ent, &dict2 );
						gameEdit->MapSetEntityKeyVal( name, "_color",  str );
					}
				}
			}
		}
		buttonSaveParticleEntitiesEnabled = true;
	}
}

void ParticleEditor::OnBnClickedButtonBrowsefadecolor() {
	float r, g, b;
	float ob;
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	r = ps->fadeColor.x;
	g = ps->fadeColor.y;
	b = ps->fadeColor.z;
	ob = 1.0f;
	if ( colorPickerDlg.Draw( "###Browsefadecolor", &r, &g, &b, &ob ) ) {
		fadeColor.Format( "%f %f %f %f", r, g, b, 1.0f );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}
}

void ParticleEditor::OnBnClickedButtonUpdate() {
	DlgVarsToCurStage();
	CurStageToDlgVars();
}

void ParticleEditor::SelectParticle( const char *name ) {
	int index = comboParticle.FindIndex( name );
	if ( index >= 0 ) {
		comboParticleSel = index;
		UpdateParticleData();
	}
}

idDeclParticle *ParticleEditor::GetCurParticle() {
	int index = comboParticleSel;
	if ( index < 0 ) {
		return NULL;
	}
	return static_cast<idDeclParticle *>( const_cast<idDecl *>( declManager->DeclByIndex( DECL_PARTICLE, index ) ) );
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
	inFileText = idStr::Format( "Particle file: %s", idp->GetFileName() );

	SetParticleView();
}

void ParticleEditor::OnCbnSelchangeComboParticles() {
	UpdateParticleData();
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

void ParticleEditor::OnBnClickedRadioRect() {
	distribution = 0;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioSphere() {
	distribution = 2;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioCylinder() {
	distribution = 1;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioCone() {
	direction = 0;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioOutward() {
	direction = 1;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioView() {
	orientation = 0;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioAimed() {
	orientation = 1;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioX() {
	orientation = 2;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioY() {
	orientation = 3;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedRadioZ() {
	orientation = 4;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void ParticleEditor::OnBnClickedDoom() {
	//::SetFocus(win32.hWnd);
}


void ParticleEditor::OnBnClickedTestModel() {
	visualization = TESTMODEL;
	SetParticleView();
}

void ParticleEditor::OnBnClickedImpact() {
	visualization = IMPACT;
	SetParticleView();
}

void ParticleEditor::OnBnClickedMuzzle(){
	visualization = MUZZLE;
	SetParticleView();
}

void ParticleEditor::OnBnClickedFlight() {
	visualization = FLIGHT;
	SetParticleView();
}

void ParticleEditor::OnBnClickedSelected() {
	visualization = SELECTED;
	SetParticleView();
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


void ParticleEditor::OnBnClickedWorldGravity() {
	DlgVarsToCurStage();
	CurStageToDlgVars();
}

void ParticleEditor::OnBnClickedEntityColor() {
	entityColor = !entityColor;
	DlgVarsToCurStage();
	CurStageToDlgVars();
}


void ParticleEditor::AddStage() {

	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	idParticleStage *stage = new idParticleStage;

	if ( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) ) {
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

	if ( !ImGui::InputMessageBox( "Are you sure you want to remove this stage?", "Remove Stage", true ) ) {
		return;
	}

	int index = listStagesSel;
	if ( index >= 0 ) {
		int newIndex = listStagesItemData.First( index );
		if ( newIndex >= 0 && newIndex < idp->stages.Num() ) {
			idp->stages.RemoveIndex( newIndex );
			index += ( index >= 1 ) ? -1 : 1;
			newIndex = comboParticle.FindIndex( idp->GetName() );
			EnumParticles();
			if ( newIndex >= 0 ) {
				comboParticleSel = newIndex;
			}
			OnCbnSelchangeComboParticles();
			listStagesSel = index;
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
	if ( idp == NULL || sel == LB_ERR || index >= idp->stages.Num() ) {
		return NULL;
	}
	return idp->stages[index];
}

void ParticleEditor::ClearDlgVars() {
	matName = "";
	color = "";
	fadeColor = "";
	offset = "";
	direction = 1;
	orientation = 1;
	distribution = 1;
	customPath = "";
	customParms = "";
	worldGravity = FALSE;
	entityColor = FALSE;
	randomDistribution = TRUE;
	customDesc = "";
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
	color = ps->color.ToString();
	fadeColor = ps->fadeColor.ToString();
	sliderFadeIn.SetValuePos( ps->fadeInFraction );
	sliderFadeOut.SetValuePos( ps->fadeOutFraction );
	sliderFadeFraction.SetValuePos( ps->fadeIndexFraction );
	sliderCount.SetPos( ps->totalParticles );
	sliderTime.SetValuePos( ps->particleLife );
	timeOffset = ps->timeOffset;
	deadTime = ps->deadTime;
	sliderGravity.SetValuePos( ps->gravity );
	sliderBunching.SetValuePos( ps->spawnBunching );
	offset = ps->offset.ToString( 0 );
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
	customParms = "";
	customDesc = ps->GetCustomPathDesc();
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
	sscanf( color, "%f %f %f %f", &ps->color.x, &ps->color.y, &ps->color.z, &ps->color.w );
	sscanf( fadeColor, "%f %f %f %f", &ps->fadeColor.x, &ps->fadeColor.y, &ps->fadeColor.z, &ps->fadeColor.w );
	ps->fadeInFraction = sliderFadeIn.GetValue();
	ps->fadeOutFraction = sliderFadeOut.GetValue();
	ps->fadeIndexFraction = sliderFadeFraction.GetValue();
	ps->totalParticles = sliderCount.GetValue();
	ps->particleLife = sliderTime.GetValue();
	ps->timeOffset = timeOffset;
	ps->deadTime = deadTime;
	ps->gravity = sliderGravity.GetValue();
	ps->spawnBunching = sliderBunching.GetValue();
	sscanf( offset, "%f %f %f", &ps->offset.x, &ps->offset.y, &ps->offset.z );
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
	ps->randomDistribution = ( randomDistribution != false );
	ps->entityColor = ( entityColor == true );

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

void ParticleEditor::ButtonNew() {
	idStr fileName;

	if ( !ImGui::Button( "New" ) ) {
		return;
	}

	if ( ImGui::InputDialogName( "New Particle", "Name", &fileName ) ) {

		idStr particleName = fileName;
		particleName.StripPath();
		particleName.StripFileExtension();

		if( !particleName.IsEmpty() )
		{
			idDeclParticle *newDecl = static_cast<idDeclParticle*>( const_cast<idDecl*>( declManager->FindType( DECL_PARTICLE, particleName.c_str(), false ) ) );
			if ( !newDecl ) {
				// create it
				newDecl = static_cast<idDeclParticle*>( declManager->CreateNewDecl( DECL_PARTICLE, particleName.c_str(), fileName.c_str() ) );
			}

			int index = comboParticle.Append( particleName );
			comboParticleSel = index;

			OnCbnSelchangeComboParticles();

			ImGui::CloseCurrentPopup();
		}
	}
}

void ParticleEditor::OnBnClickedButtonSave() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	idp->Save();
}

void ParticleEditor::EnumParticles() {
	comboParticle.Clear();
	for ( int i = 0; i < declManager->GetNumDecls( DECL_PARTICLE ); i++ ) {
		const idDecl *idp = declManager->DeclByIndex( DECL_PARTICLE, i );
		comboParticle.Append( idp->GetName() );
	}
	comboParticleSel = 0;
	OnCbnSelchangeComboParticles();
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

	EnumParticles();
	SetParticleView();

	/*
	toolTipCtrl.Create( this );
	toolTipCtrl.Activate( TRUE );

	CWnd* wnd = GetWindow( GW_CHILD );
	CString str;
	while ( wnd ) {
		if ( str.LoadString( wnd->GetDlgCtrlID() ) ) {
			toolTipCtrl.AddTool( wnd, str );
		}
		wnd = wnd->GetWindow( GW_HWNDNEXT );
	}
	*/

	buttonSaveParticleEntitiesEnabled = false;
	EnableEditControls();

	//vectorControl.SetVectorChangingCallback( VectorCallBack );
}

/*
BOOL ParticleEditor::PreTranslateMessage(MSG *pMsg) {
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST ) {
		toolTipCtrl.RelayEvent( pMsg );
	}
	return CDialog::PreTranslateMessage(pMsg);
}
*/

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

void ParticleEditor::OnBtnDrop()
{
	idStr		classname;
	idStr		key;
	idStr		value;
	idVec3		org;
	idDict		args;
	idAngles	viewAngles;

	if ( !gameEdit->PlayerIsValid() ) {
		return;
	}

	gameEdit->PlayerGetViewAngles( viewAngles );
	gameEdit->PlayerGetEyePosition( org );

	org += idAngles( 0, viewAngles.yaw, 0 ).ToForward() * 80 + idVec3( 0, 0, 1 );
	args.Set("origin", org.ToString());
	args.Set("classname", "func_emitter");
	args.Set("angle", va( "%f", viewAngles.yaw + 180 ));

	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	idStr str = idp->GetName();
	str.SetFileExtension( ".prt" );

	args.Set("model", str);

	idStr name = gameEdit->GetUniqueEntityName( "func_emitter" );
	bool nameValid = false;
	
	while (!nameValid) {
		if ( ImGui::InputDialogName( "Name particle", "Name", &name ) ) {
			idEntity *gameEnt = gameEdit->FindEntity( name );
			if ( gameEnt ) {
				if ( !ImGui::InputMessageBox( "Please choose another name", "Duplicate Entity Name!", true ) ) {
					return;
				}
			} else {
				nameValid = true;
			}
		}
	}

	args.Set( "name", name.c_str() );

	idEntity *ent = NULL;
	gameEdit->SpawnEntityDef( args, &ent );
	if (ent) {
		gameEdit->EntityUpdateChangeableSpawnArgs( ent, NULL );
		gameEdit->ClearEntitySelection();
		gameEdit->AddSelectedEntity( ent );
	}

	gameEdit->MapAddEntity( &args );
}

void ParticleEditor::OnOK()
{
	// never return on OK as windows will map this at times when you don't want
	// ENTER closing the dialog
	// CDialog::OnOK();
}

}
