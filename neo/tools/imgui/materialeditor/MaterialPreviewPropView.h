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
#ifndef MATERIALPREVIEWPROPVIEW_H_
#define MATERIALPREVIEWPROPVIEW_H_

#include "MaterialPreviewView.h"

namespace ImGuiTools {

// MaterialPreviewPropView view

class MaterialPreviewPropView
{
public:
	MaterialPreviewPropView();

	virtual ~MaterialPreviewPropView();

	bool Draw( const ImVec2 &size );

public:
	void AddLight( void );
	void InitializePropTree( void );

	void RegisterPreviewView( MaterialPreviewView *view );

protected:
	class LightData
	{
	public:
		int			id;
		int			materialNum;
		idVec3		color;
		float		radius;
		bool		moveable;

		LightData()
			: id(0)
			, materialNum(-1)
			, color(vec3_zero)
			, radius(1.0f)
			, moveable(false)
		{
		}
	};

	idStr				customModel;
	idList<LightData>	lights;
	idStrList			lightMaterials;
	int					defaultPointLightIndex;
	int					modelType;
	bool				showLights;
	float				localParms[MAX_ENTITY_SHADER_PARMS];
	float				globalParms[MAX_GLOBAL_SHADER_PARMS];
	DeclSelect			customModelSelectDlg; // TODO: replace by file dialog?

	MaterialPreviewView	*materialPreview;
};

}

#endif /* !MATERIALPREVIEWPROPVIEW_H_ */
