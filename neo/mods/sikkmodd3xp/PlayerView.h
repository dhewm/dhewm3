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

#ifndef __GAME_PLAYERVIEW_H__
#define __GAME_PLAYERVIEW_H__

#include "idlib/math/Vector.h"
#include "idlib/Dict.h"
#include "renderer/Material.h"
#include "renderer/RenderWorld.h"

class idSaveGame;
class idRestoreGame;

/*
===============================================================================

  Player view.

===============================================================================
*/

// screenBlob_t are for the on-screen damage claw marks, etc
typedef struct {
	const idMaterial *	material;
	float				x, y, w, h;
	float				s1, t1, s2, t2;
	int					finishTime;
	int					startFadeTime;
	float				driftAmount;
} screenBlob_t;

#define	MAX_SCREEN_BLOBS	4	//8	// sikk - decreased max screen blobs value

#ifdef _D3XP
class WarpPolygon_t {
public:
	idVec4					outer1;
	idVec4					outer2;
	idVec4					center;
};

class Warp_t {
public:
	int						id;
	bool					active;

	int						startTime;
	float					initialRadius;

	idVec3					worldOrigin;
	idVec2					screenOrigin;

	int						durationMsec;

	idList<WarpPolygon_t>	polys;
};
#endif

#ifdef _D3XP

class idPlayerView;
class FullscreenFXManager;

/*
==================
FxFader
==================
*/
class FxFader {
	enum {
		FX_STATE_OFF,
		FX_STATE_RAMPUP,
		FX_STATE_RAMPDOWN,
		FX_STATE_ON
	};

	int						time;
	int						state;
	float					alpha;
	int						msec;

public:
							FxFader();

	// primary functions
	bool					SetTriggerState( bool active );

	virtual void			Save( idSaveGame *savefile );
	virtual void			Restore( idRestoreGame *savefile );

	// fader functions
	void					SetFadeTime( int t )		{ msec = t; };
	int						GetFadeTime()				{ return msec; };

	// misc functions
	float					GetAlpha()					{ return alpha; };
};

/*
==================
FullscreenFX
==================
*/
class FullscreenFX {
protected:
	idStr					name;
	FxFader					fader;
	FullscreenFXManager		*fxman;

public:
							FullscreenFX()							{ fxman = NULL; };
	virtual					~FullscreenFX()							{ };

	virtual void			Initialize()							= 0;
	virtual bool			Active()								= 0;
	virtual void			HighQuality()							= 0;
	virtual void			LowQuality()							{ };
	virtual void			AccumPass( const renderView_t *view )	{ };
	virtual bool			HasAccum()								{ return false; };

	void					SetName( idStr n )						{ name = n; };
	idStr					GetName()								{ return name; };

	void					SetFXManager( FullscreenFXManager *fx )	{ fxman = fx; };

	bool					SetTriggerState( bool state )			{ return fader.SetTriggerState( state ); };
	void					SetFadeSpeed( int msec )				{ fader.SetFadeTime( msec ); };
	float					GetFadeAlpha()							{ return fader.GetAlpha(); };

	virtual void			Save( idSaveGame *savefile );
	virtual void			Restore( idRestoreGame *savefile );
};

/*
==================
FullscreenFX_Helltime
==================
*/
class FullscreenFX_Helltime : public FullscreenFX {
	const idMaterial*		acInitMaterials[3];
	const idMaterial*		acCaptureMaterials[3];
	const idMaterial*		acDrawMaterials[3];
	const idMaterial*		crCaptureMaterials[3];
	const idMaterial*		crDrawMaterials[3];
	bool					clearAccumBuffer;

	int						DetermineLevel();

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();
	virtual void			AccumPass( const renderView_t *view );
	virtual bool			HasAccum()		{ return true; };

	virtual void			Restore( idRestoreGame *savefile );
};

/*
==================
FullscreenFX_Multiplayer
==================
*/
class FullscreenFX_Multiplayer : public FullscreenFX {
	const idMaterial*		acInitMaterials;
	const idMaterial*		acCaptureMaterials;
	const idMaterial*		acDrawMaterials;
	const idMaterial*		crCaptureMaterials;
	const idMaterial*		crDrawMaterials;
	bool					clearAccumBuffer;

	int						DetermineLevel();

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();
	virtual void			AccumPass( const renderView_t *view );
	virtual bool			HasAccum()		{ return true; };

	virtual void			Restore( idRestoreGame *savefile );
};

/*
==================
FullscreenFX_Warp
==================
*/
class FullscreenFX_Warp : public FullscreenFX {
	const idMaterial*		material;
	bool					grabberEnabled;
	int						startWarpTime;

	void					DrawWarp( WarpPolygon_t wp, float interp );

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();

	void					EnableGrabber( bool active )			{ grabberEnabled = active; startWarpTime = gameLocal.slow.time; };

	virtual void			Save( idSaveGame *savefile );
	virtual void			Restore( idRestoreGame *savefile );
};

/*
==================
FullscreenFX_EnviroSuit
==================
*/
class FullscreenFX_EnviroSuit : public FullscreenFX {
	const idMaterial*		material;

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();
};

/*
==================
FullscreenFX_DoubleVision
==================
*/
class FullscreenFX_DoubleVision : public FullscreenFX {
	const idMaterial*		material;

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();
};

/*
==================
FullscreenFX_InfluenceVision
==================
*/
class FullscreenFX_InfluenceVision : public FullscreenFX {

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();
};

/*
==================
FullscreenFX_Bloom
==================
*/
class FullscreenFX_Bloom : public FullscreenFX {
	const idMaterial*		drawMaterial;
	const idMaterial*		initMaterial;
	const idMaterial*		currentMaterial;

	float					currentIntensity;
	float					targetIntensity;

public:
	virtual void			Initialize();
	virtual bool			Active();
	virtual void			HighQuality();

	virtual void			Save( idSaveGame *savefile );
	virtual void			Restore( idRestoreGame *savefile );
};

/*
==================
FullscreenFXManager
==================
*/
class FullscreenFXManager {
	idList<FullscreenFX*>	fx;
	bool					highQualityMode;
	idVec2					shiftScale;

	idPlayerView			*playerView;
	const idMaterial*		blendBackMaterial;

	void					CreateFX( idStr name, idStr fxtype, int fade );

public:
							FullscreenFXManager();
	virtual					~FullscreenFXManager();

	void					Initialize( idPlayerView *pv );

	void					Process( const renderView_t *view );
	void					CaptureCurrentRender();
	void					Blendback( float alpha );

	idVec2					GetShiftScale()			{ return shiftScale; };
	idPlayerView*			GetPlayerView()			{ return playerView; };
	idPlayer*				GetPlayer()				{ return gameLocal.GetLocalPlayer(); };

	int						GetNum()				{ return fx.Num(); };
	FullscreenFX*			GetFX( int index )		{ return fx[index]; };
	FullscreenFX*			FindFX( idStr name );

	void					Save( idSaveGame *savefile );
	void					Restore( idRestoreGame *savefile );
};
#endif

class idPlayerView {
public:
						idPlayerView();

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				SetPlayerEntity( class idPlayer *playerEnt );

	void				ClearEffects( void );

	void				DamageImpulse( idVec3 localKickDir, const idDict *damageDef );

	void				WeaponFireFeedback( const idDict *weaponDef );

	idAngles			AngleOffset( void ) const;			// returns the current kick angle

	idMat3				ShakeAxis( void ) const;			// returns the current shake angle

	void				CalculateShake( void );

	// this may involve rendering to a texture and displaying
	// that with a warp model or in double vision mode
	void				RenderPlayerView( idUserInterface *hud );

	void				Fade( idVec4 color, int time );

	void				Flash( idVec4 color, int time );

	void				AddBloodSpray( float duration );

	// temp for view testing
	void				EnableBFGVision( bool b ) { bfgVision = b; };

private:
	void				SingleView( idUserInterface *hud, const renderView_t *view );
	void				ScreenFade();

	screenBlob_t *		GetScreenBlob();

	screenBlob_t		screenBlobs[MAX_SCREEN_BLOBS];

public:
	int					dvFinishTime;		// double vision will be stopped at this time
	const idMaterial *	dvMaterial;			// material to take the double vision screen shot

	int					kickFinishTime;		// view kick will be stopped at this time
	idAngles			kickAngles;

	bool				bfgVision;			//

	const idMaterial *	tunnelMaterial;		// health tunnel vision
	const idMaterial *	armorMaterial;		// armor damage view effect
	const idMaterial *	berserkMaterial;	// berserk effect
	const idMaterial *	irGogglesMaterial;	// ir effect
	const idMaterial *	bloodSprayMaterial; // blood spray
	const idMaterial *	bfgMaterial;		// when targeted with BFG
	const idMaterial *	lagoMaterial;		// lagometer drawing
	float				lastDamageTime;		// accentuate the tunnel effect for a while

	idVec4				fadeColor;			// fade color
	idVec4				fadeToColor;		// color to fade to
	idVec4				fadeFromColor;		// color to fade from
	float				fadeRate;			// fade rate
	int					fadeTime;			// fade time

	idAngles			shakeAng;			// from the sound sources

	idPlayer *			player;
	renderView_t		view;

#ifdef _D3XP
	FullscreenFXManager	*fxManager;

public:
	int					AddWarp( idVec3 worldOrigin, float centerx, float centery, float initialRadius, float durationMsec );
	void				FreeWarp( int id );
#endif

// sikk---> PostProcess Effects
	void				DoPostFX( void );
	void				PostFX_SoftShadows( void );
	void				PostFX_EdgeAA( void );
	void				PostFX_HDR( void );
	void				PostFX_Bloom( void );
	void				PostFX_MotionBlur( void );
	void				PostFX_DoF( void );
	void				PostFX_SSIL( void );
	void				PostFX_SSAO( void );
	void				PostFX_SunShafts( void );
	void				PostFX_LensFlare( void );
	void				PostFX_ColorGrading( void );
	void				PostFX_ExplosionFX( void );
	void				PostFX_IRGoggles( void );
	void				PostFX_ScreenFrost( void );
	void				PostFX_CelShading( void );
	void				PostFX_Filmgrain( void );
	void				PostFX_Vignetting( void );

	void				PostFX_AdrenalineVision( void );
	//void				PostFX_DoubleVision( void );
	//void				PostFX_BerserkVision( void );
	//void				PostFX_InfluenceVision( void );
	void				PostFX_BFGVision( void );
	void				PostFX_TunnelVision( void );
	void				PostFX_ScreenBlobs( void );
	void				PostFX_ArmorPulse( void );

	void				RenderDepth( bool bCrop );
	void				RenderNormals( bool bFace );
	void				ToggleShadows( bool noShadows );
	void				ResetShadows( void );
	bool				DoFConditionCheck( void );
	bool				MBConditionCheck( void );

	const idMaterial *	blackMaterial;			// Black material (for general use) 
	const idMaterial *	whiteMaterial;			// White material (for general use) 
	const idMaterial *	currentRenderMaterial;	// Current Render material (for general use) 
	const idMaterial *	scratchMaterial;		// Scratch material (for general use) 
	const idMaterial *	depthMaterial;			// Depth material (for general use) 
	const idMaterial *	normalsMaterial;		// View Space Normals material (for general use) 
	const idMaterial *	softShadowsMaterial;	// Soft Shadow material
	const idMaterial *	edgeAAMaterial;			// Edge AA material
	const idMaterial *	hdrLumBaseMaterial;		// HDR Luminance Base material
	const idMaterial *	hdrLumAverageMaterial;	// HDR Luminance Average material
	const idMaterial *	hdrLumAdaptedMaterial;	// HDR Luminance Adapted material
	const idMaterial *	hdrBrightPass1Material;	// HDR Bright Pass Filter material (Reinhard RGB)
	const idMaterial *	hdrBrightPass2Material;	// HDR Bright Pass Filter material (Reinhard Yxy)
	const idMaterial *	hdrBrightPass3Material;	// HDR Bright Pass Filter material (Exp)
	const idMaterial *	hdrBrightPass4Material;	// HDR Bright Pass Filter material (Filmic simple)
	const idMaterial *	hdrBrightPass5Material;	// HDR Bright Pass Filter material (Filmic complex)
	const idMaterial *	hdrBloomMaterial;		// HDR Bloom material
	const idMaterial *	hdrFlareMaterial;		// HDR Lens Flare material
	const idMaterial *	hdrGlareMaterial;		// HDR Glare material
	const idMaterial *	hdrFinalMaterial;		// HDR Final Tone Mapping material
	const idMaterial *	bloomMaterial;			// Bloom material
	const idMaterial *	ssilMaterial;			// SSIL material
	const idMaterial *	ssaoMaterial;			// SSAO material
	const idMaterial *	sunShaftsMaterial;		// Sun Shafts material
	const idMaterial *	lensFlareMaterial;		// Lens Flare material
	const idMaterial *	dofMaterial;			// DoF material
	const idMaterial *	motionBlurMaterial;		// Motion Blur material
	const idMaterial *	colorGradingMaterial;	// Color Grading material
	const idMaterial *	screenFrostMaterial;	// Screen Frost material
	const idMaterial *	celShadingMaterial;		// Cel Shading material
	const idMaterial *	filmgrainMaterial;		// Filmgrain material
	const idMaterial *	vignettingMaterial;		// Vignetting material
	const idMaterial *	tunnel2Material;		// health tunnel vision for Health Management System (Health Regen)
	const idMaterial *	adrenalineMaterial;		// Adrenaline Vision material
	const idMaterial *	explosionFXMaterial;	// Explosion FX material

	renderView_t		hackedView;
	float				focusDistance;			// Holds focus distance for depth of field
	idAngles			prevViewAngles;			// Holds previous frame's player view angles
	int					prevTime;				// Holds previous frame's time
	bool				bDepthRendered;			// Holds whether the depth map has been rendered for the current frame
	bool				bDitherRendered;		// Holds whether the dither map has been rendered
	float				fDitherSize;			// Holds previous frame's dither size to check for changes
	pvsHandle_t			playerPVS;				// Holds player's current pvs (for soft shadows)
	bool				bSoftShadows;			// a soft shadows toggle used so ResetShadows() is only run once when r_useSoftShadows = 0
// <---sikk

// sikk---> PostProccess Scaling Fix
	int					screenHeight;
	int					screenWidth;
	idVec2				shiftScale;
// <---sikk
};

#endif /* !__GAME_PLAYERVIEW_H__ */
