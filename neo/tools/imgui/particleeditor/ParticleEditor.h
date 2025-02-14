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

#ifndef __PARTICLEEDITOR_H__
#define __PARTICLEEDITOR_H__

#include "../../edit_public.h"

class idDeclParticle;
class idParticleStage;

namespace ImGuiTools
{

class RangeSlider {
public:
	bool Draw( const char *label, float sliderWidth );

	void SetRange( int _min, int _max ) {
		min = _min;
		max = _max;
	}
	int GetRangeMin() {
		return min;
	}
	int GetRangeMax() {
		return max;
	}

	void SetValueRange(float _low, float _high) {
		low = _low;
		high = _high;
	}

	float GetValueLow() {
		return low;
	}

	float GetValueHigh() {
		return high;
	}

	void SetValuePos( float val ) {
		SetPos( GetRangeMin() + ( GetRangeMax() - GetRangeMin() ) * ( val - low ) / ( high - low ) );
	}

	float GetValue() {
		return low + ( high - low ) * ( float )( GetPos() - GetRangeMin() ) / ( GetRangeMax() - GetRangeMin() );
	}

	void SetPos( int _pos ) {
		pos = _pos;
	}

	int GetPos() {
		return pos;
	}

private:
	float low, high;
	int pos;
	int min, max;
};

class ParticleDrop {
public:
						ParticleDrop();

	void				Start( idDeclParticle *curParticle );
	void				Draw();

private:
	idStr				classname;
	idVec3				org;
	idDict				args;
	idAngles			viewAngles;

	enum state_t { DONE = 0, NAME };

	idStr				errorText;
	idStr				name;
	state_t				state;
};

class ParticleEditor {

public:
						ParticleEditor();   // standard constructor

	static ParticleEditor& Instance();

	void				SelectParticle( const char *name );
	void				SetParticleVisualization( int i );
	void				SetVectorControlUpdate( idQuat rotation );

	enum { TESTMODEL, IMPACT, MUZZLE, FLIGHT, SELECTED };

	void					Reset();
	void					Draw();

	void			ShowIt(bool show) {
		isShown = show;
	}
	bool			IsShown() {
		return isShown;
	}

private:
	void		OnCbnSelchangeComboPath();
	void		OnLbnSelchangeListStages();
	void		ButtonColor();
	void		ButtonFadeColor();
	void		ButtonEntityColor();
	void		OnBnClickedButtonSave();
	void		OnBnClickedButtonSaveParticles();
	void		OnBnClickedTestModel();
	void		OnBnClickedImpact();
	void		OnBnClickedMuzzle();
	void		OnBnClickedFlight();
	void		OnBnClickedSelected();
	void		OnBnClickedDoom();
	void		OnBnClickedButtonUpdate();
	void		OnBnClickedParticleMode();
	void		OnBtnYup();
	void		OnBtnYdn();
	void		OnBtnXdn();
	void		OnBtnXup();
	void		OnBtnZup();
	void		OnBtnZdn();

private:
	bool				isShown;

	DeclNewSelect		particleNewDlg;
	DeclSelect			particleSelectDlg;
	DeclSelect			materialSelectDlg;

	idStr				inFileText;

	bool				buttonSaveParticleEntitiesEnabled;

	ColorPicker			colorDlg;
	ColorPicker			fadeColorDlg;
	ColorPicker			entityColorDlg;
	ParticleDrop		particleDropDlg;

	idDeclParticle *	curParticle;

	// edit controls
	bool				editControlsEnabled;

	// stage controls
	bool				stageControlsEnabled;
	bool				editRingOffsetEnabled;
	bool				editOrientationParm1Enabled;
	bool				editOrientationParm2Enabled;

	idList<idStr>		listStages;
	idHashIndex			listStagesItemData;
	int					listStagesSel;
	RangeSlider			sliderBunching;
	RangeSlider			sliderFadeIn;
	RangeSlider			sliderFadeOut;
	RangeSlider			sliderFadeFraction;
	RangeSlider			sliderCount;
	RangeSlider			sliderTime;
	RangeSlider			sliderGravity;
	RangeSlider			sliderSpeedFrom;
	RangeSlider			sliderSpeedTo;
	RangeSlider			sliderRotationFrom;
	RangeSlider			sliderRotationTo;
	RangeSlider			sliderSizeFrom;
	RangeSlider			sliderSizeTo;
	RangeSlider			sliderAspectFrom;
	RangeSlider			sliderAspectTo;
	//VectorCtl			vectorControl;
	
	idStr				depthHack;
	idStr				matName;
	int					animFrames;
	float				animRate;
	idVec4				color;
	idVec4				fadeColor;
	float				timeOffset;
	float				deadTime;
	float				offset[3];
	float				xSize;
	float				ySize;
	float				zSize;
	float				ringOffset;
	idStr				dirParmText;
	float				directionParm;
	int					direction;
	int					orientation;
	int					distribution;
	idStr				viewOrigin;
	idStr				customPath;
	idStr				customParms;
	float				trails;
	float				trailTime;
	float				cycles;
	idStr				editRingOffset;
	bool				worldGravity;
	bool				entityColor;
	bool				randomDistribution;
	float				initialAngle;
	float				boundsExpansion;
	idStr				customDesc;

	bool				particleMode;

	int					visualization;

	bool				mapModified;

private:
	void				AddStage( bool clone );
	void				RemoveStage();
	void				RemoveStageThink();
	void				ShowStage();
	void				HideStage();
	void				SetCurParticle( idDeclParticle *dp );
	idDeclParticle *	GetCurParticle();
	idParticleStage *	GetCurStage();
	void				ClearDlgVars();
	void				CurStageToDlgVars();
	void				DlgVarsToCurStage();
	void				ShowCurrentStage();
	void				UpdateControlInfo();
	void				SetParticleView();
	void				UpdateParticleData();
	void				SetSelectedModel( const char *val );
	void				EnableStageControls();
	void				EnableEditControls();
	void				UpdateSelectedOrigin( float x, float y, float z );
};

}

#endif /* !__PARTICLEEDITOR_H__ */
