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
	bool Draw( const char *label, float itemWidth, float sliderWidth );

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

protected:
	void		OnCbnSelchangeComboParticles();
	void		OnCbnSelchangeComboPath();
	void		OnLbnSelchangeListStages();
	void		OnBnClickedButtonBrowsematerial();
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
	void		OnBtnDrop();
	//void		OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

private:
	bool				isShown;

	int					fileSelection;
	idList<idStr>		prtFiles;
	idStr				fileName;

	idStr				inFileText;

	bool				buttonSaveParticleEntitiesEnabled;

	ColorPicker			colorDlg;
	ColorPicker			fadeColorDlg;
	ColorPicker			entityColorDlg;

	int					materialDeclSelection;
	idList<idStr>		materialDecls;
	idStr				materialDeclName;

	// edit controls
	bool				editControlsEnabled;

	// stage controls
	bool				stageControlsEnabled;
	bool				editMaterialEnabled;
	bool				buttonBrowseMaterialEnabled;
	bool				editAnimFramesEnabled;
	bool				editAnimRateEnabled;
	bool				editColorEnabled;
	bool				buttonBrowseColorEnabled;
	bool				editFadeColorEnabled;
	bool				buttonBrowseFadeColorEnabled;
	bool				editFadeInEnabled;
	bool				sliderFadeInEnabled;
	bool				editFadeOutEnabled;
	bool				editFadeFractionEnabled;
	bool				sliderFadeOutEnabled;
	bool				sliderFadeFractionEnabled;
	bool				editBunchingEnabled;
	bool				sliderBunchingEnabled;
	bool				editCountEnabled;
	bool				sliderCountEnabled;
	bool				editTimeEnabled;
	bool				sliderTimeEnabled;
	bool				editCyclesEnabled;
	bool				editTimeOffsetEnabled;
	bool				editDeadTimeEnabled;
	bool				checkWorldGravityEnabled;
	bool				editGravityEnabled;
	bool				sliderGravityEnabled;
	bool				radioRectEnabled;
	bool				radioCylinderEnabled;
	bool				radioSphereEnabled;
	bool				editOffsetEnabled;
	bool				editXSizeEnabled;
	bool				editYSizeEnabled;
	bool				editZSizeEnabled;
	bool				editRingOffsetEnabled;
	bool				radioConeEnabled;
	bool				radioOutwardEnabled;
	bool				editDirectionParmEnabled;
	bool				radioAimedEnabled;
	bool				radioViewEnabled;
	bool				radioXEnabled;
	bool				radioYEnabled;
	bool				radioZEnabled;
	bool				editOrientationParm1Enabled;
	bool				editOrientationParm2Enabled;
	bool				sliderSpeedFromEnabled;
	bool				editSpeedFromEnabled;
	bool				editSpeedToEnabled;
	bool				sliderSpeedToEnabled;
	bool				sliderRotationFromEnabled;
	bool				editRotationFromEnabled;
	bool				editRotationToEnabled;
	bool				sliderRotationToEnabled;
	bool				sliderSizeFromEnabled;
	bool				editSizeFromEnabled;
	bool				editSizeToEnabled;
	bool				sliderSizeToEnabled;
	bool				sliderAspectFromEnabled;
	bool				editAspectFromEnabled;
	bool				editAspectToEnabled;
	bool				editSliderAspectToEnabled;
	bool				comboCustomPathEnabled;
	bool				checkEntityColorEnabled;
	
	bool				shouldPopulate;
	idList<idStr>		comboParticle;
	int					comboParticleSel;
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

private:
	void				EnumParticles();
	void				AddStage( bool clone );
	void				RemoveStage();
	void				ShowStage();
	void				HideStage();
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
	bool				mapModified;
};

}

#endif /* !__PARTICLEEDITOR_H__ */
