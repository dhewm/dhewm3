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
#ifndef MATERIALPREVIEWVIEW_H_
#define MATERIALPREVIEWVIEW_H_

#include "../util/ImGui_IdWidgets.h"
#include "renderer/Material.h"
#include "sys/sys_imgui.h"

#include "MaterialDoc.h"
#include "MaterialView.h"
//#include "../radiant/GLWidget.h"

namespace ImGuiTools {

class idGLDrawable {
public:
	idGLDrawable();
	~idGLDrawable() {};
	virtual void input() {}
	virtual void draw(int x, int y, int w, int h);
	virtual void setMedia(const char* name) {}
	virtual int getRealTime() { return realTime; };
	virtual bool ScreenCoords() {
		return true;
	}
	void SetRealTime(int i) {
		realTime = i;
	}
	virtual void Update() {};
	float getScale() {
		return scale;
	}
	void setScale(float f) {
		scale = f;
	}
protected:
	float scale;
	float xOffset;
	float yOffset;
	float zOffset;
	int realTime;
};

class idGLDrawableView : public idGLDrawable {

public:
	idGLDrawableView();
	~idGLDrawableView();

	virtual void setMedia(const char *name);
	virtual void input();
	virtual void draw(int x, int y, int w, int h);
	virtual void Update() {};

	void UpdateCamera( renderView_t *refdef );
	void UpdateModel( void );
	void UpdateLights( void );

	void addLight( void );
	void deleteLight( const int lightId );
	void drawLights( renderView_t *refdef );

	void InitWorld();
	void ResetView( void );

	void setLightShader( const int lightId, const idStr shaderName );
	void setLightColor( const int lightId, const idVec3 &value );
	void setLightRadius( const int lightId, const float radius );
	void setLightAllowMove( const int lightId, const bool move );
	void setObject( int Id );
	void setCustomModel( const idStr modelName );
	void setShowLights( bool _showLights );
	void setLocalParm( int parmNum, float value );
	void setGlobalParm( int parmNum, float value );

protected:
	idRenderWorld		*world;
	idRenderModel		*worldModel;
	const idImage		*currentRender;
	const idMaterial	*white;
	const idMaterial	*material;

	bool				showLights;

	idVec3				viewOrigin;
	idAngles			viewRotation;
	float				viewDistance;

	renderEntity_t		worldEntity;
	qhandle_t			modelDefHandle;

	int					objectId;
	idStr				customModelName;

	float				globalParms[MAX_GLOBAL_SHADER_PARMS];

	typedef struct {
		renderLight_t		renderLight;
		qhandle_t			lightDefHandle;
		idVec3				origin;
		const idMaterial	*shader;
		float				radius;
		idVec3				color;
		bool				allowMove;
	} lightInfo_t;

	idList<lightInfo_t>	viewLights;
};

// ==================================================================
// ==================================================================

class MaterialPreviewView : public MaterialView
{
	//DECLARE_DYNCREATE(MaterialPreviewView)

public:
	MaterialPreviewView();
	virtual ~MaterialPreviewView();

	bool Draw( const ImVec2 &size );

public:
	//virtual void OnDraw(CDC* pDC);      // overridden to draw this view

	void	MV_OnMaterialSelectionChange(MaterialDoc* pMaterial);

	void	OnModelChange( int modelId );
	void	OnCustomModelChange( idStr modelName );
	void	OnShowLightsChange( bool showLights );

	void	OnLocalParmChange( int parmNum, float value );
	void	OnGlobalParmChange( int parmNum, float value );

	void	OnLightShaderChange( int lightId, idStr shaderName );
	void	OnLightRadiusChange( int lightId, float radius );
	void	OnLightColorChange( int lightId, idVec3 &color );
	void	OnLightAllowMoveChange( int lightId, bool move );

	void	OnAddLight( void );
	void	OnDeleteLight( int lightId );

protected:
	//idGLWidget			renderWindow;
	idGLDrawableView	renderedView;

	idStr				currentMaterial;

public:
	//afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//afx_msg void OnSize(UINT nType, int cx, int cy);
};

}

#endif /* !MATERIALPREVIEWVIEW_H_ */
