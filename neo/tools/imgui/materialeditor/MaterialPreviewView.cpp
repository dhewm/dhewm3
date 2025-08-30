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


#include "framework/EventLoop.h"
#include "framework/Session.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/Image.h"
#include "renderer/ModelManager.h"
#include "../../renderer/tr_local.h"


//#include "../radiant/QE3.H"
#include "MaterialDoc.h"
#include "renderer/RenderSystem.h"
#include "MaterialPreviewView.h"

namespace ImGuiTools {

// MaterialPreviewView

MaterialPreviewView::MaterialPreviewView() {
	// Initialize the rendered material
	renderedView.setMedia( "_default" );
}

MaterialPreviewView::~MaterialPreviewView() {
}

// MaterialPreviewView drawing

bool MaterialPreviewView::Draw( const ImVec2 &size ) {
	if ( ImGui::BeginChild( "MaterialPreviewView", size, ImGuiChildFlags_Borders ) ) {
		ImGui::SetNextItemAllowOverlap();
		ImGui::InvisibleButton("###MaterialPreviewRenderedView", size, ImGuiButtonFlags_AllowOverlap | ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle | ImGuiButtonFlags_MouseButtonRight );
		ImVec2 pos = ImGui::GetCursorPos();
		if ( ImGui::IsItemActive() )
		{
			renderedView.input();
		}
		renderedView.draw( pos.x, pos.y, size.x, size.y );

		if ( ImGui::IsWindowFocused() ) {
			MaterialEditorSetActiveWindow( ME_WINDOW_PREVIEW );
		}
	}
	ImGui::EndChild();

	return false;
}

// MaterialPreviewView message handlers

void MaterialPreviewView::MV_OnMaterialSelectionChange( MaterialDoc *pMaterial )
{
	if ( pMaterial && pMaterial->renderMaterial ) {
		currentMaterial = pMaterial->renderMaterial->GetName();
		renderedView.setMedia( currentMaterial );
	}
}

void MaterialPreviewView::OnLocalParmChange( int parmNum, float value ) {

	renderedView.setLocalParm( parmNum, value );
}

void MaterialPreviewView::OnGlobalParmChange( int parmNum, float value ) {

	renderedView.setGlobalParm( parmNum, value );
}

void MaterialPreviewView::OnLightShaderChange( int lightId, idStr shaderName ) {

	renderedView.setLightShader( lightId, shaderName );
}

void MaterialPreviewView::OnLightColorChange( int lightId, idVec3 &color ) {

	renderedView.setLightColor( lightId, color );
}

void MaterialPreviewView::OnLightRadiusChange( int lightId, float radius ) {

	renderedView.setLightRadius( lightId, radius );
}

void MaterialPreviewView::OnLightAllowMoveChange( int lightId, bool move ) {

	renderedView.setLightAllowMove( lightId, move );
}

void MaterialPreviewView::OnAddLight( void ) {

	renderedView.addLight();
}

void MaterialPreviewView::OnDeleteLight( int lightId ) {

	renderedView.deleteLight( lightId );
}

void MaterialPreviewView::OnModelChange( int modelId ) {

	renderedView.setObject( modelId );
}

void MaterialPreviewView::OnCustomModelChange( idStr modelName ) {

	renderedView.setCustomModel( modelName );
}

void MaterialPreviewView::OnShowLightsChange( bool showLights ) {

	renderedView.setShowLights( showLights );
}

/*
 =============================================================================
 =============================================================================
 =============================================================================
 */

idGLDrawable::idGLDrawable() {
	scale = 1.0;
	xOffset = 0.0;
	yOffset = 0.0;
	realTime = 0;
}

void idGLDrawable::draw(int x, int y, int w, int h) {
	GL_State( GLS_DEFAULT );
	qglViewport( x, y, w, h );
	qglScissor( x, y, w, h );
	qglMatrixMode( GL_PROJECTION );
	qglClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
	qglClear( GL_COLOR_BUFFER_BIT );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglLineWidth( 0.5 );
	qglColor3f( 1, 1, 1 );
	globalImages->BindNull();
	qglBegin(GL_LINE_LOOP);
	qglColor3f(1, 0, 0);
	qglVertex2f(x + 3, y + 3);
	qglColor3f(0, 1, 0);
	qglVertex2f(x + 3, h - 3);
	qglColor3f(0, 0, 1);
	qglVertex2f(w - 3, h - 3);
	qglColor3f(1, 1, 1);
	qglVertex2f(w - 3, y + 3);
	qglEnd();
}

idGLDrawableView::idGLDrawableView() {
	material = NULL;
	modelDefHandle = -1;

	objectId = 0;
	showLights = true;
	realTime = 16;

	viewOrigin.Set( 0.f, 0.f, 0.f );
	viewRotation.Set( 0.f, 0.f, 0.f );
	viewDistance = -196.f;

	world = NULL;
	worldModel = NULL;

	ResetView();
}

idGLDrawableView::~idGLDrawableView() {
	delete world;
	delete worldModel;
}

void idGLDrawableView::ResetView( void ) {
	idDict			spawnArgs;

	InitWorld();

	memset( &worldEntity, 0, sizeof( worldEntity ) );
	spawnArgs.Clear();
	spawnArgs.Set("classname", "func_static");
	spawnArgs.Set("name", spawnArgs.GetString("model"));
	spawnArgs.Set("origin", "0 0 0");

	gameEdit->ParseSpawnArgsToRenderEntity(&spawnArgs, &worldEntity);

	// load a model and set the current material as its customshader
	worldModel = renderModelManager->FindModel( "models/materialeditor/cube128.ase" );
	worldEntity.hModel = worldModel;

	// current material
	worldEntity.customShader = material;

	// current rotation
	worldEntity.axis = mat3_identity;

	// set global shader parms
	memset( globalParms, 0, sizeof( globalParms ) );
	globalParms[0] = globalParms[1] = globalParms[2] = globalParms[3] = 1.f;

	worldEntity.shaderParms[0] = 1.f;
	worldEntity.shaderParms[1] = 1.f;
	worldEntity.shaderParms[2] = 1.f;
	worldEntity.shaderParms[3] = 1.f;
	modelDefHandle = world->AddEntityDef( &worldEntity );
}

void idGLDrawableView::InitWorld() {

	if ( world == NULL ) {
		world = renderSystem->AllocRenderWorld();
	}
	if ( worldModel == NULL ) {
		worldModel = renderModelManager->AllocModel();
	}

	world->InitFromMap( NULL );
	worldModel->InitEmpty( "GLWorldModel" );

	viewLights.Clear();
}

void idGLDrawableView::addLight( void ) {
	int				lightId;
	idStr			str;
	lightInfo_t		viewLight;
	idDict			spawnArgs;

	lightId = viewLights.Num();

	spawnArgs.Set( "classname", "light" );
	spawnArgs.Set( "name", va( "light_%d", lightId ) );
	spawnArgs.Set( "origin", va( "-128 0 %d", (lightId * 16) ) );
	spawnArgs.Set( "light", "300" );
	spawnArgs.Set( "texture", "lights/defaultPointLight" );
	sprintf( str, "%f %f %f", 1.f, 1.f, 1.f );
	spawnArgs.Set( "_color", str );
	gameEdit->ParseSpawnArgsToRenderLight( &spawnArgs, &viewLight.renderLight );

	viewLight.lightDefHandle = world->AddLightDef( &viewLight.renderLight );
	viewLight.origin = viewLight.renderLight.origin;
	viewLight.shader = declManager->FindMaterial( "lights/defaultPointLight", false );
	viewLight.color.x = viewLight.renderLight.shaderParms[ SHADERPARM_RED ];
	viewLight.color.y = viewLight.renderLight.shaderParms[ SHADERPARM_GREEN ];
	viewLight.color.z = viewLight.renderLight.shaderParms[ SHADERPARM_BLUE ];
	viewLight.radius = 300.f;
	viewLight.allowMove = true;

	// Add light to the list
	viewLights.Append( viewLight );
}

void idGLDrawableView::deleteLight( const int lightId ) {
	if ( lightId < viewLights.Num() ) {
		world->FreeLightDef( viewLights[lightId].lightDefHandle );

		viewLights.RemoveIndex( lightId );
	}
}

void idGLDrawableView::UpdateCamera( renderView_t *refdef ) {
	idVec3		pos, dir;
	idAngles	angs;

	// Set the camera origin
	pos = viewRotation.ToForward();
	pos *= viewDistance;
	refdef->vieworg = pos;

	// Set the view to point back at the origin
	dir = vec3_origin - pos;
	angs = dir.ToAngles();
	refdef->viewaxis = angs.ToMat3();
}


void idGLDrawableView::UpdateModel( void ) {

	switch( objectId ) {
		case 0:
			worldModel = renderModelManager->FindModel( "models/materialeditor/cube128.ase" );
			break;
		case 1:
			worldModel = renderModelManager->FindModel( "models/materialeditor/box128x64.ase" );
			break;
		case 2:
			worldModel = renderModelManager->FindModel( "models/materialeditor/box128x32.ase" );
			break;
		case 3:
			worldModel = renderModelManager->FindModel( "models/materialeditor/box64x128.ase" );
			break;
		case 4:
			worldModel = renderModelManager->FindModel( "models/materialeditor/box32x128.ase" );
			break;
		case 5:
			worldModel = renderModelManager->FindModel( "models/materialeditor/cylinder_v.ase" );
			break;
		case 6:
			worldModel = renderModelManager->FindModel( "models/materialeditor/cylinder_h.ase" );
			break;
		case 7:
			worldModel = renderModelManager->FindModel( "models/materialeditor/sphere64.ase" );
			break;
		case -1:
			worldModel = renderModelManager->FindModel( customModelName.c_str() );
			break;
		default:
			worldModel = renderModelManager->FindModel( "models/materialeditor/cube128.ase" );
			break;
	};

	worldEntity.hModel = worldModel;

	// current material
	worldEntity.customShader = material;
	// current rotation
	worldEntity.origin = viewOrigin;

	worldEntity.axis = mat3_identity;

	world->UpdateEntityDef( modelDefHandle, &worldEntity );
}

void idGLDrawableView::UpdateLights( void ) {
	int i;

	for ( i = 0; i < viewLights.Num(); i++ ) {
		lightInfo_t	*vLight = &viewLights[i];

		vLight->renderLight.shader = vLight->shader;

		vLight->renderLight.shaderParms[ SHADERPARM_RED ] = vLight->color.x;
		vLight->renderLight.shaderParms[ SHADERPARM_GREEN ] = vLight->color.y;
		vLight->renderLight.shaderParms[ SHADERPARM_BLUE ] = vLight->color.z;

		vLight->renderLight.lightRadius[0] = vLight->renderLight.lightRadius[1] =
		vLight->renderLight.lightRadius[2] = vLight->radius;

		vLight->renderLight.origin = vLight->origin;

		world->UpdateLightDef( vLight->lightDefHandle, &vLight->renderLight );
	}
}

void idGLDrawableView::drawLights( renderView_t *refdef ) {
	int i;

	for ( i=0; i < viewLights.Num(); i++ ) {
		lightInfo_t	*vLight = &viewLights[i];

		idVec4 lColor;
		lColor.x = vLight->color.x;
		lColor.y = vLight->color.y;
		lColor.z = vLight->color.z;
		lColor.w = 1.f;

		idSphere sphere(vLight->renderLight.origin, 4);
		session->rw->DebugSphere( lColor, sphere, 0, true );
		session->rw->DrawText( va( "%d", i + 1 ), vLight->renderLight.origin + idVec3( 0, 0, 5 ), 0.25f, idVec4( 1, 1, 0, 1 ), refdef->viewaxis, 1, 0, true );
	}
}

void idGLDrawableView::input() {
	// mouse move
	bool doZoom;
	float sensitivity = 0.5f;

	if ( ImGui::IsMouseDragging( ImGuiMouseButton_Left ) || ImGui::IsMouseDragging( ImGuiMouseButton_Middle ) || ImGui::IsMouseDragging( ImGuiMouseButton_Right ) ) {
		ImVec2 delta = ImGui::GetIO().MouseDelta;

		// Left mouse button rotates and zooms the view
		if ( ImGui::IsMouseDragging( ImGuiMouseButton_Left ) ) {
			doZoom = ImGui::IsKeyDown( ImGuiKey_LeftAlt ) || ImGui::IsKeyDown( ImGuiKey_RightAlt );
			if (doZoom) {
				viewDistance -= delta.y;
			} else {
				viewRotation.yaw += -(delta.x * sensitivity);
				viewRotation.pitch += (delta.y * sensitivity);

				viewRotation.pitch = idMath::ClampFloat( -89.9f, 89.9f, viewRotation.pitch );
			}

		// Right mouse button moves lights in the view plane
		} else if ( ImGui::IsMouseDragging( ImGuiMouseButton_Right ) ) {
			int		i;
			float	lightMovement = 0;
			idVec3	lightForward, lightRight, lightUp;
			idVec3	lightMove;

			lightMove.Zero();
			viewRotation.ToVectors( &lightForward, &lightRight, &lightUp );

			doZoom = ImGui::IsKeyDown( ImGuiKey_LeftAlt ) || ImGui::IsKeyDown( ImGuiKey_RightAlt );
			if (doZoom) {
				if ( delta.y != 0 ) {
					lightMovement = -delta.y * sensitivity;

					lightMovement = idMath::ClampFloat( -32.f, 32.f, lightMovement );
					lightMove = lightForward * lightMovement;
				}
			} else {
				if (delta.x != 0) {
					lightMovement = delta.x * sensitivity;

					lightMovement = idMath::ClampFloat( -32.f, 32.f, lightMovement );
					lightMove = lightRight * lightMovement;
				}
				if (delta.y != 0) {
					lightMovement = -delta.y * sensitivity;

					lightMovement = idMath::ClampFloat( -32.f, 32.f, lightMovement );
					lightMove += lightUp * lightMovement;
				}
			}

			// Go through the lights and move the ones that are set to allow movement
			for ( i = 0; i < viewLights.Num(); i++ ) {
				lightInfo_t	*vLight = &viewLights[i];

				if ( vLight->allowMove ) {
					vLight->origin += lightMove;
				}
			}

		// Middle mouse button moves object up and down
		} else if ( ImGui::IsMouseDragging( ImGuiMouseButton_Middle ) ) {
			viewOrigin.z -= delta.y;

			UpdateModel();
		}
	}
}

void idGLDrawableView::draw( int x, int y, int w, int h ) {
	int					i;
	renderView_t		refdef;
	const idMaterial	*mat = material;

	if ( mat ) {
		UpdateLights();

		// render it
		memset( &refdef, 0, sizeof( refdef ) );

		UpdateCamera( &refdef );

		// Copy global shaderparms to view
		for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
			refdef.shaderParms[ i ] = globalParms[ i ];
		}

		refdef.width = SCREEN_WIDTH;
		refdef.height = SCREEN_HEIGHT;
		refdef.fov_x = 90;
		refdef.fov_y = 2 * atan((float)h / w) * idMath::M_RAD2DEG;

		refdef.time = eventLoop->Milliseconds();

		renderSystem->SetColor( idVec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
		renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0, white );

		world->RenderScene( &refdef );

		if ( showLights ) {
			drawLights( &refdef );
		}

		renderSystem->CropRenderSize( SCREEN_WIDTH, SCREEN_HEIGHT, true );
		renderSystem->CaptureRenderToImage( "_currentRender" );
		renderSystem->UnCrop();

		const ImVec2 p0 = ImGui::GetItemRectMin();
		const ImVec2 p1 = ImGui::GetItemRectMax();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImGui::PushClipRect(p0, p1, true);
		draw_list->AddImage(currentRender->texnum, p0, p1, ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );
		ImGui::PopClipRect();
	}
}

// ============================
// Interface to PropTree Window
// ============================

void idGLDrawableView::setMedia( const char *name ) {
	float	ratio = 1;

	currentRender = globalImages->GetImage( "_currentRender" );
	white = declManager->FindMaterial( "guis/assets/white.tga" );

	if ( name && *name ) {
		material = declManager->FindMaterial( name );
	} else {
		material = NULL;
	}

	if ( material->GetNumStages() == 0 ) {
		material = declManager->FindMaterial( "_default" );
	}

	if ( material->GetStage(0)->texture.image ) {
		ratio = (float)((float)material->GetImageWidth() / (float)material->GetImageHeight());
	}

	if ( objectId == -1 ) {
		// Don't change a custom model
	} else if ( ratio == 1 ) {
		objectId = 0;
	} else if ( ratio == 2 ) {
		objectId = 1;
	} else if ( ratio == 4 ) {
		objectId = 2;
	} else if ( ratio == 0.5 ) {
		objectId = 3;
	} else if ( ratio == 0.25 ) {
		objectId = 4;
	}

	UpdateModel();
}

void idGLDrawableView::setLocalParm( int parmNum, float value ) {
	if ( parmNum < 0 || parmNum >= MAX_ENTITY_SHADER_PARMS ) {
		return;
	}

	worldEntity.shaderParms[ parmNum ] = value;

	UpdateModel();
}

void idGLDrawableView::setGlobalParm( int parmNum, float value ) {
	if ( parmNum < 0 || parmNum >= MAX_GLOBAL_SHADER_PARMS ) {
		return;
	}

	globalParms[ parmNum ] = value;
}

void idGLDrawableView::setLightShader( const int lightId, const idStr shaderName ) {
	if ( lightId < viewLights.Num() ) {
		viewLights[ lightId ].shader = declManager->FindMaterial( shaderName, false );
	}
}

void idGLDrawableView::setLightColor( const int lightId, const idVec3 &value ) {
	if ( lightId < viewLights.Num() ) {
		// Update this lights color
		viewLights[ lightId ].color = value;
	}
}

void idGLDrawableView::setLightRadius( const int lightId, const float radius ) {
	if ( lightId < viewLights.Num() ) {
		viewLights[ lightId ].radius = radius;
	}
}

void idGLDrawableView::setLightAllowMove( const int lightId, const bool move ) {
	if ( lightId < viewLights.Num() ) {
		viewLights[ lightId ].allowMove = move;
	}
}

void idGLDrawableView::setObject( int Id ) {
	objectId = Id;

	UpdateModel();
}

void idGLDrawableView::setCustomModel( const idStr modelName ) {
	if ( modelName.Length() ) {
		objectId = -1;
	} else {
		objectId = 0;
	}

	customModelName = modelName;

	UpdateModel();
}

void idGLDrawableView::setShowLights( bool _showLights ) {
	showLights = _showLights;
}

}
