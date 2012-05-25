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

#include "sys/platform.h"
#include "renderer/RenderWorld.h"

#include "gamesys/SysCvar.h"
#include "gamesys/SaveGame.h"
#include "GameBase.h"
#include "Player.h"

#include "PlayerView.h"

const int IMPULSE_DELAY = 150;

// sikk---> Portal Sky Box
static int MakePowerOfTwo( int num ) {
	int pot;
	for ( pot = 1; pot < num; pot <<= 1 ) {}
	return pot;
}
// <---sikk

/*
==============
idPlayerView::idPlayerView
==============
*/
idPlayerView::idPlayerView() {
	memset( screenBlobs, 0, sizeof( screenBlobs ) );
	memset( &view, 0, sizeof( view ) );
	player					= NULL;

	armorMaterial			= declManager->FindMaterial( "armorViewEffect" );
	tunnelMaterial			= declManager->FindMaterial( "textures/decals/tunnel" );
	berserkMaterial			= declManager->FindMaterial( "textures/decals/berserk" );
//	irGogglesMaterial		= declManager->FindMaterial( "textures/decals/irblend" );
	irGogglesMaterial		= declManager->FindMaterial( "postProcess/irGoggles" );	// sikk - Infrared Goggles PostProcess Effect
	bloodSprayMaterial		= declManager->FindMaterial( "textures/decals/bloodspray" );
	bfgMaterial				= declManager->FindMaterial( "textures/decals/bfgvision" );
	lagoMaterial			= declManager->FindMaterial( LAGO_MATERIAL, false );

// sikk---> PostProcess Effects
	blackMaterial			= declManager->FindMaterial( "_black" );
	whiteMaterial			= declManager->FindMaterial( "_white" );
	currentRenderMaterial	= declManager->FindMaterial( "_currentRender" );
	scratchMaterial			= declManager->FindMaterial( "_scratch" );
	depthMaterial			= declManager->FindMaterial( "render/depth" );
	normalsMaterial			= declManager->FindMaterial( "render/normals" );
	softShadowsMaterial		= declManager->FindMaterial( "postProcess/softShadows" );
	edgeAAMaterial			= declManager->FindMaterial( "postProcess/edgeAA" );
	hdrLumBaseMaterial		= declManager->FindMaterial( "postProcess/hdrLumBase" );
	hdrLumAverageMaterial	= declManager->FindMaterial( "postProcess/hdrLumAverage" );
	hdrLumAdaptedMaterial	= declManager->FindMaterial( "postProcess/hdrLumAdapted" );
	hdrBrightPass1Material	= declManager->FindMaterial( "postProcess/hdrBrightPass1" );
	hdrBrightPass2Material	= declManager->FindMaterial( "postProcess/hdrBrightPass2" );
	hdrBrightPass3Material	= declManager->FindMaterial( "postProcess/hdrBrightPass3" );
	hdrBrightPass4Material	= declManager->FindMaterial( "postProcess/hdrBrightPass4" );
	hdrBrightPass5Material	= declManager->FindMaterial( "postProcess/hdrBrightPass5" );
	hdrBloomMaterial		= declManager->FindMaterial( "postProcess/hdrBloom" );
	hdrGlareMaterial		= declManager->FindMaterial( "postProcess/hdrGlare" );
	hdrFinalMaterial		= declManager->FindMaterial( "postProcess/hdrFinal" );
	bloomMaterial			= declManager->FindMaterial( "postProcess/bloom" );
	ssilMaterial			= declManager->FindMaterial( "postProcess/ssil" );
	ssaoMaterial			= declManager->FindMaterial( "postProcess/ssao" );
	sunShaftsMaterial		= declManager->FindMaterial( "postProcess/sunShafts" );
	lensFlareMaterial		= declManager->FindMaterial( "postProcess/lensFlare" );
	dofMaterial				= declManager->FindMaterial( "postProcess/dof" );
	motionBlurMaterial		= declManager->FindMaterial( "postProcess/motionBlur" );
	colorGradingMaterial	= declManager->FindMaterial( "postProcess/colorGrading" );
	celShadingMaterial		= declManager->FindMaterial( "postProcess/celShading" );
	filmgrainMaterial		= declManager->FindMaterial( "postProcess/filmgrain" );
	vignettingMaterial		= declManager->FindMaterial( "postProcess/vignetting" );
	tunnel2Material			= declManager->FindMaterial( "postProcess/tunnel" );
	bSoftShadows			= false;
// <---sikk

	bfgVision				= false;
	dvFinishTime			= 0;
	kickFinishTime			= 0;
	kickAngles.Zero();
	lastDamageTime			= 0.0f;
	fadeTime				= 0;
	fadeRate				= 0.0;
	fadeFromColor.Zero();
	fadeToColor.Zero();
	fadeColor.Zero();
	shakeAng.Zero();

	ClearEffects();
}

/*
==============
idPlayerView::Save
==============
*/
void idPlayerView::Save( idSaveGame *savefile ) const {
	int i;
	const screenBlob_t *blob;

	blob = &screenBlobs[ 0 ];
	for( i = 0; i < MAX_SCREEN_BLOBS; i++, blob++ ) {
		savefile->WriteMaterial( blob->material );
		savefile->WriteFloat( blob->x );
		savefile->WriteFloat( blob->y );
		savefile->WriteFloat( blob->w );
		savefile->WriteFloat( blob->h );
		savefile->WriteFloat( blob->s1 );
		savefile->WriteFloat( blob->t1 );
		savefile->WriteFloat( blob->s2 );
		savefile->WriteFloat( blob->t2 );
		savefile->WriteInt( blob->finishTime );
		savefile->WriteInt( blob->startFadeTime );
		savefile->WriteFloat( blob->driftAmount );
	}

	savefile->WriteInt( dvFinishTime );
	savefile->WriteMaterial( scratchMaterial );
	savefile->WriteInt( kickFinishTime );
	savefile->WriteAngles( kickAngles );
	savefile->WriteBool( bfgVision );

	savefile->WriteMaterial( tunnelMaterial );
	savefile->WriteMaterial( armorMaterial );
	savefile->WriteMaterial( berserkMaterial );
	savefile->WriteMaterial( irGogglesMaterial );
	savefile->WriteMaterial( bloodSprayMaterial );
	savefile->WriteMaterial( bfgMaterial );
	savefile->WriteFloat( lastDamageTime );

	savefile->WriteVec4( fadeColor );
	savefile->WriteVec4( fadeToColor );
	savefile->WriteVec4( fadeFromColor );
	savefile->WriteFloat( fadeRate );
	savefile->WriteInt( fadeTime );

	savefile->WriteAngles( shakeAng );

	savefile->WriteObject( player );
	savefile->WriteRenderView( view );
}

/*
==============
idPlayerView::Restore
==============
*/
void idPlayerView::Restore( idRestoreGame *savefile ) {
	int i;
	screenBlob_t *blob;

	blob = &screenBlobs[ 0 ];
	for ( i = 0; i < MAX_SCREEN_BLOBS; i++, blob++ ) {
		savefile->ReadMaterial( blob->material );
		savefile->ReadFloat( blob->x );
		savefile->ReadFloat( blob->y );
		savefile->ReadFloat( blob->w );
		savefile->ReadFloat( blob->h );
		savefile->ReadFloat( blob->s1 );
		savefile->ReadFloat( blob->t1 );
		savefile->ReadFloat( blob->s2 );
		savefile->ReadFloat( blob->t2 );
		savefile->ReadInt( blob->finishTime );
		savefile->ReadInt( blob->startFadeTime );
		savefile->ReadFloat( blob->driftAmount );
	}

	savefile->ReadInt( dvFinishTime );
	savefile->ReadMaterial( scratchMaterial );
	savefile->ReadInt( kickFinishTime );
	savefile->ReadAngles( kickAngles );
	savefile->ReadBool( bfgVision );

	savefile->ReadMaterial( tunnelMaterial );
	savefile->ReadMaterial( armorMaterial );
	savefile->ReadMaterial( berserkMaterial );
	savefile->ReadMaterial( irGogglesMaterial );
	savefile->ReadMaterial( bloodSprayMaterial );
	savefile->ReadMaterial( bfgMaterial );
	savefile->ReadFloat( lastDamageTime );

	savefile->ReadVec4( fadeColor );
	savefile->ReadVec4( fadeToColor );
	savefile->ReadVec4( fadeFromColor );
	savefile->ReadFloat( fadeRate );
	savefile->ReadInt( fadeTime );

	savefile->ReadAngles( shakeAng );

	savefile->ReadObject( reinterpret_cast<idClass *&>( player ) );
	savefile->ReadRenderView( view );
}

/*
==============
idPlayerView::SetPlayerEntity
==============
*/
void idPlayerView::SetPlayerEntity( idPlayer *playerEnt ) {
	player = playerEnt;
}

/*
==============
idPlayerView::ClearEffects
==============
*/
void idPlayerView::ClearEffects() {
	lastDamageTime = MS2SEC( gameLocal.time - 99999 );

	dvFinishTime = ( gameLocal.time - 99999 );
	kickFinishTime = ( gameLocal.time - 99999 );

	for ( int i = 0; i < MAX_SCREEN_BLOBS; i++ ) {
		screenBlobs[i].finishTime = gameLocal.time;
	}

	fadeTime = 0;
	bfgVision = false;
}

/*
==============
idPlayerView::GetScreenBlob
==============
*/
screenBlob_t *idPlayerView::GetScreenBlob() {
	screenBlob_t *oldest = &screenBlobs[0];

	for ( int i = 1 ; i < MAX_SCREEN_BLOBS ; i++ ) {
		if ( screenBlobs[i].finishTime < oldest->finishTime ) {
			oldest = &screenBlobs[i];
		}
	}
	return oldest;
}

/*
==============
idPlayerView::DamageImpulse

LocalKickDir is the direction of force in the player's coordinate system,
which will determine the head kick direction
==============
*/
void idPlayerView::DamageImpulse( idVec3 localKickDir, const idDict *damageDef ) {
	// keep shotgun from obliterating the view
	if ( lastDamageTime > 0.0f && SEC2MS( lastDamageTime ) + IMPULSE_DELAY > gameLocal.time ) {
		return;
	}

	float dvTime = damageDef->GetFloat( "dv_time" );
	if ( dvTime ) {
		if ( dvFinishTime < gameLocal.time ) {
			dvFinishTime = gameLocal.time;
		}
		dvFinishTime += g_dvTime.GetFloat() * dvTime;
		// don't let it add up too much in god mode
		if ( dvFinishTime > gameLocal.time + 5000 ) {
			dvFinishTime = gameLocal.time + 5000;
		}
	}

	// head angle kick
	float kickTime = damageDef->GetFloat( "kick_time" );
	if ( kickTime ) {
		kickFinishTime = gameLocal.time + g_kickTime.GetFloat() * kickTime;

		// forward / back kick will pitch view
		kickAngles[0] = localKickDir[0];
		// side kick will yaw view
		kickAngles[1] = localKickDir[1] * 0.5f;
		// up / down kick will pitch view
		kickAngles[0] += localKickDir[2];
		// roll will come from side
		kickAngles[2] = localKickDir[1];

		float kickAmplitude = damageDef->GetFloat( "kick_amplitude" );
		if ( kickAmplitude ) {
			kickAngles *= kickAmplitude;
		}
	}

	//
	// screen blob
	//
	float	blobTime = damageDef->GetFloat( "blob_time" );
	if ( blobTime ) {
		screenBlob_t	*blob = GetScreenBlob();
		blob->startFadeTime = gameLocal.time;
		blob->finishTime = gameLocal.time + blobTime * g_blobTime.GetFloat();

		blob->driftAmount = 0.0f;	// sikk - Blood Spray Screen Effect - keeps damage blood splats from drifting 

		const char *materialName = damageDef->GetString( "mtr_blob" );
		blob->material = declManager->FindMaterial( materialName );
		blob->x = damageDef->GetFloat( "blob_x" );
		blob->x += ( gameLocal.random.RandomInt()&63 ) - 32;
		blob->y = damageDef->GetFloat( "blob_y" );
		blob->y += ( gameLocal.random.RandomInt()&63 ) - 32;

		float scale = ( 256 + ( ( gameLocal.random.RandomInt()&63 ) - 32 ) ) / 256.0f;
		blob->w = damageDef->GetFloat( "blob_width" ) * g_blobSize.GetFloat() * scale;
		blob->h = damageDef->GetFloat( "blob_height" ) * g_blobSize.GetFloat() * scale;
		blob->s1 = 0;
		blob->t1 = 0;
		blob->s2 = 1;
		blob->t2 = 1;
	}

	// save lastDamageTime for tunnel vision attenuation
	lastDamageTime = MS2SEC( gameLocal.time );
}

/*
==================
idPlayerView::AddBloodSpray

If we need a more generic way to add blobs then we can do that
but having it localized here lets the material be pre-looked up etc.
==================
*/
void idPlayerView::AddBloodSpray( float duration ) {
	if ( duration <= 0 || bloodSprayMaterial == NULL || g_skipViewEffects.GetBool() ) {
		return;
	}

// sikk---> Blood Spray Screen Effect
	// Use random material
	if ( gameLocal.random.RandomFloat() < 0.5f )
		bloodSprayMaterial = declManager->FindMaterial( "postProcess/bloodspray1" );
	else
		bloodSprayMaterial = declManager->FindMaterial( "postProcess/bloodspray2" );
// <---sikk

	// visit this for chainsaw
	screenBlob_t *blob = GetScreenBlob();
	blob->startFadeTime = gameLocal.time;
	blob->finishTime = gameLocal.time + ( duration * 1000 );
	blob->material = bloodSprayMaterial;
	blob->x = ( gameLocal.random.RandomInt() & 63 ) - 32;
	blob->y = ( gameLocal.random.RandomInt() & 63 ) - 32;
	blob->driftAmount = 0.5f + gameLocal.random.CRandomFloat() * 0.5f;
	float scale = ( 256 + ( ( gameLocal.random.RandomInt() & 63 ) - 32 ) ) / 256.0f;
	blob->w = 640 * g_blobSize.GetFloat() * scale;	// sikk - This was "600". Typo?
	blob->h = 480 * g_blobSize.GetFloat() * scale;
	float s1 = 0.0f;
	float t1 = 0.0f;
	float s2 = 1.0f;
	float t2 = 1.0f;
	if ( blob->driftAmount < 0.6 ) {
		s1 = 1.0f;
		s2 = 0.0f;
	} else if ( blob->driftAmount < 0.75 ) {
		t1 = 1.0f;
		t2 = 0.0f;
	} else if ( blob->driftAmount < 0.85 ) {
		s1 = 1.0f;
		s2 = 0.0f;
		t1 = 1.0f;
		t2 = 0.0f;
	}
	blob->s1 = s1;
	blob->t1 = t1;
	blob->s2 = s2;
	blob->t2 = t2;
}

/*
==================
idPlayerView::WeaponFireFeedback

Called when a weapon fires, generates head twitches, etc
==================
*/
void idPlayerView::WeaponFireFeedback( const idDict *weaponDef ) {
	int recoilTime = weaponDef->GetInt( "recoilTime" );

	// don't shorten a damage kick in progress
	if ( recoilTime && kickFinishTime < gameLocal.time ) {
		idAngles angles;
		weaponDef->GetAngles( "recoilAngles", "5 0 0", angles );
		kickAngles = angles;
		int	finish = gameLocal.time + g_kickTime.GetFloat() * recoilTime;
		kickFinishTime = finish;
	}

}

/*
===================
idPlayerView::CalculateShake
===================
*/
void idPlayerView::CalculateShake() {
	idVec3	origin, matrix;
	float shakeVolume = gameSoundWorld->CurrentShakeAmplitudeForPosition( gameLocal.time, player->firstPersonViewOrigin );

	// shakeVolume should somehow be molded into an angle here
	// it should be thought of as being in the range 0.0 -> 1.0, although
	// since CurrentShakeAmplitudeForPosition() returns all the shake sounds
	// the player can hear, it can go over 1.0 too.
	shakeAng[0] = gameLocal.random.CRandomFloat() * shakeVolume;
	shakeAng[1] = gameLocal.random.CRandomFloat() * shakeVolume;
	shakeAng[2] = gameLocal.random.CRandomFloat() * shakeVolume;
}

/*
===================
idPlayerView::ShakeAxis
===================
*/
idMat3 idPlayerView::ShakeAxis() const {
	return shakeAng.ToMat3();
}

/*
===================
idPlayerView::AngleOffset

  kickVector, a world space direction that the attack should
===================
*/
idAngles idPlayerView::AngleOffset() const {
	idAngles ang;

	ang.Zero();

	if ( gameLocal.time < kickFinishTime ) {
		float offset = kickFinishTime - gameLocal.time;
		ang = kickAngles * offset * offset * g_kickAmplitude.GetFloat();
		for ( int i = 0; i < 3; i++ ) {
			if ( ang[i] > 70.0f )
				ang[i] = 70.0f;
			else if ( ang[i] < -70.0f )
				ang[i] = -70.0f;
		}
	}
	return ang;
}

/*
==================
idPlayerView::SingleView
==================
*/
void idPlayerView::SingleView( const renderView_t *view ) {
	// normal rendering
	if ( !view ) {
		return;
	}

	// if the objective system is up, don't do normal drawing
	if ( player->objectiveSystemOpen ) {
		player->objectiveSystem->Redraw( gameLocal.time );
		return;
	}

// sikk---> PostProccess Scaling Fix
	if ( screenHeight != renderSystem->GetScreenHeight() || screenWidth != renderSystem->GetScreenWidth() ) {
		renderSystem->GetGLSettings( screenWidth, screenHeight );
		float f = MakePowerOfTwo( screenWidth );
		shiftScale.x = (float)screenWidth / f;
		f = MakePowerOfTwo( screenHeight );
		shiftScale.y = (float)screenHeight / f;
	}
// <---sikk

	// hack the shake in at the very last moment, so it can't cause any consistency problems
	hackedView = *view;
	hackedView.viewaxis = hackedView.viewaxis * ShakeAxis();

// sikk---> Portal Sky Box
	if ( gameLocal.portalSkyEnt.GetEntity() && gameLocal.IsPortalSkyAcive() && g_enablePortalSky.GetBool() ) {
		renderView_t portalView = hackedView;
		portalView.vieworg = gameLocal.portalSkyEnt.GetEntity()->GetPhysics()->GetOrigin();

		// setup global fixup projection vars
		hackedView.shaderParms[4] = shiftScale.x;
		hackedView.shaderParms[5] = shiftScale.y;

		gameRenderWorld->RenderScene( &portalView );
		renderSystem->CaptureRenderToImage( "_currentRender" );

		hackedView.forceUpdate = true;	// FIX: for smoke particles not drawing when portalSky present
	}
// <---sikk

// sikk---> Soft Shadows PostProcess 
	if ( r_useSoftShadows.GetBool() && !g_skipViewEffects.GetBool() ) {
		playerPVS = gameLocal.pvs.SetupCurrentPVS( player->GetPVSAreas(), player->GetNumPVSAreas() );

		ToggleShadows( false );
		gameRenderWorld->RenderScene( &hackedView );
		renderSystem->CaptureRenderToImage( "_ssRender" );
		ToggleShadows( true );

		gameLocal.pvs.FreeCurrentPVS( playerPVS );
	}
// <---sikk

//	RenderDepth( true );
	gameRenderWorld->RenderScene( &hackedView );
}

/*
=================
idPlayerView::Flash

flashes the player view with the given color
=================
*/
void idPlayerView::Flash( idVec4 color, int time ) {
	Fade( idVec4( 0.0f, 0.0f, 0.0f, 0.0f ), time);
	fadeFromColor = colorWhite;
}

/*
=================
idPlayerView::Fade

used for level transition fades
assumes: color.w is 0 or 1
=================
*/
void idPlayerView::Fade( idVec4 color, int time ) {
	if ( !fadeTime ) {
		fadeFromColor.Set( 0.0f, 0.0f, 0.0f, 1.0f - color[ 3 ] );
	} else {
		fadeFromColor = fadeColor;
	}
	fadeToColor = color;

	if ( time <= 0 ) {
		fadeRate = 0;
		time = 0;
		fadeColor = fadeToColor;
	} else {
		fadeRate = 1.0f / (float)time;
	}

	if ( gameLocal.realClientTime == 0 && time == 0 ) {
		fadeTime = 1;
	} else {
		fadeTime = gameLocal.realClientTime + time;
	}
}

/*
=================
idPlayerView::ScreenFade
=================
*/
void idPlayerView::ScreenFade() {
	int msec = fadeTime - gameLocal.realClientTime;
	float t;

	if ( msec <= 0 ) {
		fadeColor = fadeToColor;
		if ( fadeColor[ 3 ] == 0.0f )
			fadeTime = 0;
	} else {
		t = (float)msec * fadeRate;
		fadeColor = fadeFromColor * t + fadeToColor * ( 1.0f - t );
	}

	if ( fadeColor[ 3 ] != 0.0f ) {
		renderSystem->SetColor4( fadeColor[ 0 ], fadeColor[ 1 ], fadeColor[ 2 ], fadeColor[ 3 ] );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, whiteMaterial );
	}
}

/*
===================
idPlayerView::RenderPlayerView
===================
*/
void idPlayerView::RenderPlayerView( idUserInterface *hud ) {
	const renderView_t *view = player->GetRenderView();
	SingleView( view );

	// place the sound origin for the player
	gameSoundWorld->PlaceListener( view->vieworg, view->viewaxis, player->entityNumber + 1, gameLocal.time, hud ? hud->State().GetString( "location" ) : "Undefined" );

	if ( !g_skipViewEffects.GetBool() && !player->objectiveSystemOpen )
		DoPostFX();

	// if the objective system is up, don't draw hud
	if ( !player->objectiveSystemOpen )
		player->DrawHUD( hud );

	if ( fadeTime )
		ScreenFade();

	if ( net_clientLagOMeter.GetBool() && lagoMaterial && gameLocal.isClient ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
		renderSystem->DrawStretchPic( 10.0f, 380.0f, 64.0f, 64.0f, 0.0f, 0.0f, 1.0f, 1.0f, lagoMaterial );
	}

	prevTime = gameLocal.time;	// sikk - update prevTime
}

// sikk---> PostProcess Effects
/*
===================
idPlayerView::DoPostFX
===================
*/
void idPlayerView::DoPostFX() {
	if ( r_useSoftShadows.GetBool() )
		PostFX_SoftShadows();
	else if ( bSoftShadows )
		ResetShadows();

	if ( r_useEdgeAA.GetBool() )
		PostFX_EdgeAA();

	if ( r_useCelShading.GetBool() )
		PostFX_CelShading();

	if ( r_useSSIL.GetBool() )
		PostFX_SSIL();

	if ( r_useSSAO.GetBool() )
		PostFX_SSAO();

	if ( r_useSunShafts.GetBool() )
		PostFX_SunShafts();

	if ( r_useHDR.GetBool() ) {
		cvarSystem->SetCVarBool( "r_testARBProgram", true );
		PostFX_HDR();
	} else {
		cvarSystem->SetCVarBool( "r_testARBProgram", false );
	}

	if ( r_useBloom.GetBool() )
		PostFX_Bloom();

	if ( r_useLensFlare.GetBool() )
		PostFX_LensFlare();

	if ( r_useColorGrading.GetBool() )
		PostFX_ColorGrading();

	if ( r_useDepthOfField.GetBool() )
		PostFX_DoF();

	if ( r_useMotionBlur.GetBool() )
		PostFX_MotionBlur();

	if ( bfgVision )
		PostFX_BFGVision();

	PostFX_ScreenBlobs();

	PostFX_ArmorPulse();

	if ( !gameLocal.inCinematic )
		PostFX_TunnelVision();
			
	PostFX_IRGoggles();

	if ( r_useVignetting.GetBool() && !r_useHDR.GetBool() )	// HDR uses it's own vignette solution
		PostFX_Vignetting();

	if ( r_useFilmgrain.GetBool() )
		PostFX_Filmgrain();

	if ( player->GetInfluenceMaterial() || player->GetInfluenceEntity() )
		PostFX_InfluenceVision();

	if ( g_doubleVision.GetBool() && gameLocal.time < dvFinishTime )
		PostFX_DoubleVision();

	if ( player->PowerUpActive( BERSERK ) )
		PostFX_BerserkVision();

	// test a single material drawn over everything
	if ( g_testPostProcess.GetString()[0] && !player->spectating ) {
		const idMaterial *mtr = declManager->FindMaterial( g_testPostProcess.GetString(), false );
		if ( !mtr ) {
			common->Printf( "Material not found.\n" );
			g_testPostProcess.SetString( "" );
		} else {
			renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, mtr );
		}
	}
}

/*
===================
idPlayerView::RenderDepth
===================
*/
void idPlayerView::RenderDepth( bool bCrop ) {
	// modify player related models in depth render.
	if ( !player->IsHidden() && !pm_thirdPerson.GetBool() )
		player->ToggleSuppression( true );

	if ( bCrop ) {
		int	nWidth = renderSystem->GetScreenWidth() / 2;
		int	nHeight = renderSystem->GetScreenHeight() / 2;

		renderSystem->CropRenderSize( nWidth, nHeight, true );
		// set our depthView parms
		renderView_t depthView = hackedView;
		depthView.viewID = -8;
		depthView.globalMaterial = depthMaterial;
		// render scene
		gameRenderWorld->RenderScene( &depthView );
		// capture image for our depth buffer
		renderSystem->CaptureRenderToImage( "_depth" );
		renderSystem->UnCrop();
	} else {	// uncropped depth is used specifically for soft shadows
		// set our depthView parms
		renderView_t depthView = hackedView;
		depthView.viewID = -8;
		depthView.globalMaterial = depthMaterial;
		// render scene
		gameRenderWorld->RenderScene( &depthView );
		// capture image for our depth buffer
		renderSystem->CaptureRenderToImage( "_ssDepth" );
	}

	// Restore player models
	if ( !player->IsHidden() && !pm_thirdPerson.GetBool() && player->bViewModelsModified )
		player->ToggleSuppression( false );
}

/*
===================
idPlayerView::RenderNormals
===================
*/
void idPlayerView::RenderNormals( bool bFace ) {
	int	nWidth = renderSystem->GetScreenWidth() / 2;
	int	nHeight = renderSystem->GetScreenHeight() / 2;

	if ( bFace ) {
		renderSystem->CropRenderSize( nWidth, nHeight, true );
		renderSystem->SetColor4( g_fov.GetFloat(), 1.0f, 1.0f, bFace );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, normalsMaterial );
		renderSystem->CaptureRenderToImage( "_normals" );
		renderSystem->UnCrop();
	} else {
		// modify player related models in normals render.
		if ( !player->IsHidden() && !pm_thirdPerson.GetBool() )
			player->ToggleSuppression( true );

		renderSystem->CropRenderSize( nWidth, nHeight, true );
		// set our normalsView parms
		renderView_t normalsView = hackedView;
		normalsView.viewID = -8;
		normalsView.globalMaterial = normalsMaterial;
		// render scene
		gameRenderWorld->RenderScene( &normalsView );
		// capture image for our normals buffer
		renderSystem->CaptureRenderToImage( "_normals" );
		renderSystem->UnCrop();

		// Restore player models
		if ( !player->IsHidden() && !pm_thirdPerson.GetBool() && player->bViewModelsModified )
			player->ToggleSuppression( false );
	}
}

/*
===================
idPlayerView::PostFX_SoftShadows
===================
*/
void idPlayerView::PostFX_SoftShadows() {
	bSoftShadows = true;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	RenderDepth( false );

	// create shadow mask texture
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, softShadowsMaterial );
	renderSystem->CaptureRenderToImage( "_ssMask" );

	// blur shadow mask texture	and modulate scene in the same pass
	if ( r_softShadowsBlurFilter.GetInteger() && r_softShadowsBlurFilter.GetInteger() < 4 ) {
		renderSystem->SetColor4( r_softShadowsBlurScale.GetFloat(), r_softShadowsBlurEpsilon.GetFloat(), g_fov.GetFloat(), r_softShadowsBlurFilter.GetFloat() );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, softShadowsMaterial );
		if ( r_softShadowsBlurFilter.GetInteger() == 3 ) {
			renderSystem->CaptureRenderToImage( "_ssMask" );
			renderSystem->SetColor4( r_softShadowsBlurScale.GetFloat(), r_softShadowsBlurEpsilon.GetFloat(), g_fov.GetFloat(), ( r_softShadowsBlurFilter.GetFloat() + 1.0f ) );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, softShadowsMaterial );
		}
	} else {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 5.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, softShadowsMaterial );
	}
}

/*
===============
idPlayerView::ToggleShadows
===============
*/
void idPlayerView::ToggleShadows( bool noShadows ) {
	idEntity   *ent;
	idLight	   *light;

	// handle player's flashlight specifically
	if ( player->GetCurrentWeapon() == 11 ) {
		if ( pm_thirdPerson.GetBool() ) {
			renderLight_t *mf	= player->weapon.GetEntity()->GetWorldMuzzleFlash();
			int	mfHandle		= player->weapon.GetEntity()->GetWorldMuzzleFlashHandle();

			mf->noShadows = noShadows;
			if ( mfHandle != -1 )
				gameRenderWorld->UpdateLightDef( mfHandle, mf );
		} else {
			renderLight_t *mf	= player->weapon.GetEntity()->GetMuzzleFlash();
			int	mfHandle		= player->weapon.GetEntity()->GetMuzzleFlashHandle();

			mf->noShadows = noShadows;
			if ( mfHandle != -1 )
				gameRenderWorld->UpdateLightDef( mfHandle, mf );
		}
	}

	for ( int i = 0; i < gameLocal.currentLights.Num(); i++ ) {
		if ( gameLocal.entities[ gameLocal.currentLights[ i ] ] == NULL ) {
			gameLocal.currentLights.RemoveIndex( i );
		} else {
			ent = gameLocal.entities[ gameLocal.currentLights[ i ] ];

			if ( gameLocal.pvs.InCurrentPVS( playerPVS, ent->GetPVSAreas(), ent->GetNumPVSAreas() ) ) {
				light = static_cast<idLight*>( ent );
				light->GetRenderLight()->noShadows = noShadows;
				light->UpdateShadowState();
			} else {
				light = static_cast<idLight*>( ent );

				if ( light->GetRenderLight()->noShadows == true ) {
					light->GetRenderLight()->noShadows = false;
					light->UpdateShadowState();
				}
			}
		}
	}
}

/*
===============
idPlayerView::ResetShadows
===============
*/
void idPlayerView::ResetShadows() {
	idEntity   *ent;
	idLight	   *light;

	// handle player's flashlight specifically
	if ( player->GetCurrentWeapon() == 11 ) {
		if ( pm_thirdPerson.GetBool() ) {
			renderLight_t *mf	= player->weapon.GetEntity()->GetWorldMuzzleFlash();
			int	mfHandle		= player->weapon.GetEntity()->GetWorldMuzzleFlashHandle();

			mf->noShadows = false;
			if ( mfHandle != -1 )
				gameRenderWorld->UpdateLightDef( mfHandle, mf );
		} else {
			renderLight_t *mf	= player->weapon.GetEntity()->GetMuzzleFlash();
			int	mfHandle		= player->weapon.GetEntity()->GetMuzzleFlashHandle();

			mf->noShadows = false;
			if ( mfHandle != -1 )
				gameRenderWorld->UpdateLightDef( mfHandle, mf );
		}
	}

	for ( int i = 0; i < gameLocal.currentLights.Num(); i++ ) {
		if ( gameLocal.entities[ gameLocal.currentLights[ i ] ] == NULL ) {
			gameLocal.currentLights.RemoveIndex( i );
		} else {
			ent = gameLocal.entities[ gameLocal.currentLights[ i ] ];
			light = static_cast<idLight*>( ent );

			if ( light->GetRenderLight()->noShadows == true ) {
				light->GetRenderLight()->noShadows = false;
				light->UpdateShadowState();
			}
		}
	}

	bSoftShadows = false;
}

/*
===================
idPlayerView::PostFX_EdgeAA
===================
*/
void idPlayerView::PostFX_EdgeAA() {
	renderSystem->CaptureRenderToImage( "_currentRender" );
	renderSystem->SetColor4( r_edgeAASampleScale.GetFloat(), r_edgeAAFilterScale.GetFloat(), 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, edgeAAMaterial );
}

/*
===================
idPlayerView::PostFX_CelShading
===================
*/
void idPlayerView::PostFX_CelShading() {
	renderSystem->CaptureRenderToImage( "_currentRender" );

	if ( r_celShadingMethod.GetInteger() == 2 )
		RenderDepth( true );

	renderSystem->SetColor4( r_celShadingScale.GetFloat(), r_celShadingThreshold.GetFloat(), 1.0f, r_celShadingMethod.GetInteger() );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, celShadingMaterial );
}

/*
===================
idPlayerView::PostFX_HDR
===================
*/
void idPlayerView::PostFX_HDR() {
	float	fElapsedTime	= MS2SEC( gameLocal.time - prevTime );
	int		nBloomWidth		= renderSystem->GetScreenWidth() / 4;
	int		nBloomHeight	= renderSystem->GetScreenHeight() / 4;
	int		nGlareWidth		= renderSystem->GetScreenWidth() / 8;
	int		nGlareHeight	= renderSystem->GetScreenHeight() / 8;

	// capture original scene image
	renderSystem->CaptureRenderToImage( "_currentRender" );

	// create lower res luminance map
	renderSystem->CropRenderSize( 256, 256, true, true );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, hdrLumBaseMaterial );
	renderSystem->CaptureRenderToImage( "_hdrLum" );
	renderSystem->CaptureRenderToImage( "_hdrLumAvg" );
	renderSystem->UnCrop();

	// create average scene luminance map by using a 4x4 downsampling chain and box-filtering
	// Output will be a 1x1 pixel of the average luminance
	//for ( int i = 0; i < 6; i++ ) {
	for ( int i = 128; i > 1; i *= 0.25 ) {
		renderSystem->CropRenderSize( i, i, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrLumAverageMaterial );
		renderSystem->CaptureRenderToImage( "_hdrLumAvg" );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrLumAverageMaterial );
		renderSystem->CaptureRenderToImage( "_hdrLumAvg" );
		renderSystem->UnCrop();
	}

	// create adapted luminance map based on current average luminance and previous adapted luminance maps
	renderSystem->CropRenderSize( 2, 2, true, true );
	renderSystem->SetColor4( r_hdrAdaptationRate.GetFloat(), fElapsedTime, r_hdrLumThresholdMin.GetFloat(), r_hdrLumThresholdMax.GetFloat() );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrLumAdaptedMaterial );
	renderSystem->CaptureRenderToImage( "_hdrLumAdpt" );
	renderSystem->UnCrop();

	if ( r_hdrGlareStyle.GetInteger() ) {
		// perform bright pass filter on _currentRender for bloom/glare textures
		renderSystem->CropRenderSize( nBloomWidth, nBloomHeight, true, true );
		renderSystem->SetColor4( r_hdrBloomMiddleGray.GetFloat(), r_hdrBloomWhitePoint.GetFloat(), r_hdrBloomThreshold.GetFloat(), r_hdrBloomOffset.GetFloat() );
		if ( r_hdrBloomToneMapper.GetInteger() == 0 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, hdrBrightPass1Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 1 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, hdrBrightPass2Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 2 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, hdrBrightPass3Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 3 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, hdrBrightPass4Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 4 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, hdrBrightPass5Material );
		renderSystem->CaptureRenderToImage( "_hdrBloom" );
		renderSystem->UnCrop();

		// create bloom texture
		for ( int i = 0; i < 2; i++ ) {
			renderSystem->CropRenderSize( nBloomWidth, nBloomHeight, true, true );
			renderSystem->SetColor4( r_hdrBloomSize.GetFloat(), 0.0f, 1.0f, 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBloomMaterial );
			renderSystem->CaptureRenderToImage( "_hdrBloom" );
			renderSystem->SetColor4( 0.0f, r_hdrBloomSize.GetFloat(), r_hdrBloomScale.GetFloat(), 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBloomMaterial );
			renderSystem->CaptureRenderToImage( "_hdrBloom" );
			renderSystem->UnCrop();
		}
	}

	// create glare textures
	if ( r_hdrGlareStyle.GetInteger() == 0 ) {
		// bloom off (clear textures)
		renderSystem->CropRenderSize( 1, 1, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, blackMaterial );
		renderSystem->CaptureRenderToImage( "_hdrBloom" );
		renderSystem->CaptureRenderToImage( "_hdrGlare" );
		renderSystem->UnCrop();
	} else if ( r_hdrGlareStyle.GetInteger() == 1 ) {
		// natural bloom (clear just _hdrGlare)
		renderSystem->CropRenderSize( 1, 1, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, blackMaterial );
		renderSystem->CaptureRenderToImage( "_hdrGlare" );
		renderSystem->UnCrop();
	} else if ( r_hdrGlareStyle.GetInteger() > 1 ) {
		int nGlareBlend;

		// crop _hdrBloom1 for glare textures
		renderSystem->CropRenderSize( nGlareWidth, nGlareHeight, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, declManager->FindMaterial( "_hdrBloom" ) );
		renderSystem->CaptureRenderToImage( "_hdrGlareX" );
		renderSystem->CaptureRenderToImage( "_hdrGlareY" );
		renderSystem->CaptureRenderToImage( "_hdrGlareZ" );

		switch ( r_hdrGlareStyle.GetInteger() ) {
		case 2:	// star glare
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 0.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 1.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
			}
			nGlareBlend = 2;
			break;
		case 3:	// cross glare
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 2.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 3.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
			}
			nGlareBlend = 2;
			break;
		case 4:	// snow cross glare
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 4.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 5.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 6.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareZ" );
			}
			nGlareBlend = 3;
			break;
		case 5:	// horizontal glare
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 7.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
			}
			nGlareBlend = 0;
			break;
		case 6:	// vertical glare
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 8.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
			}
			nGlareBlend = 1;
			break;
		case 7:	// star glare with chromatic abberation
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 9.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 10.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
			}
			nGlareBlend = 2;
			break;
		case 8:	// cross glare with chromatic abberation
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 11.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 12.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
			}
			nGlareBlend = 2;
			break;
		case 9:	// snow cross glare with chromatic abberation
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 13.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 14.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 15.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareZ" );
			}
			nGlareBlend = 3;
			break;
		case 10:	// horizontal glare with chromatic abberation
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 16.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareX" );
			}
			nGlareBlend = 0;
			break;
		case 11:	// vertical glare with chromatic abberation
			for ( int i = 1; i <= 3; i++ ) {
				renderSystem->SetColor4( r_hdrGlareSize.GetFloat(), i, 1.0f, 17.0f );
				renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
				renderSystem->CaptureRenderToImage( "_hdrGlareY" );
			}
			nGlareBlend = 1;
			break;
		}
		// blend glare textures and capture to a single texture
		renderSystem->SetColor4( r_hdrGlareScale.GetFloat(), 1.0f, nGlareBlend, 18.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrGlareMaterial );
		renderSystem->CaptureRenderToImage( "_hdrGlare" );
		renderSystem->UnCrop();
	}

	if ( r_hdrDither.GetBool() ) {
		float size = 16.0f * r_hdrDitherSize.GetFloat();
		renderSystem->SetColor4( renderSystem->GetScreenWidth() / size, renderSystem->GetScreenHeight() / size, 1.0f, -1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFinalMaterial );
		renderSystem->CaptureRenderToImage( "_hdrDither" );
	} else {
		renderSystem->CropRenderSize( 1, 1, true, true );
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, -2.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFinalMaterial );
		renderSystem->CaptureRenderToImage( "_hdrDither" );
		renderSystem->UnCrop();
	}

	// perform final tone mapping
	renderSystem->SetColor4( r_hdrMiddleGray.GetFloat(), r_hdrWhitePoint.GetFloat(), r_hdrBlueShiftFactor.GetFloat(), r_hdrToneMapper.GetInteger() + 5 * r_useVignetting.GetBool() );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFinalMaterial );
}

/*
===================
idPlayerView::PostFX_Bloom
===================
*/
void idPlayerView::PostFX_Bloom() {
	// determine bloom buffer size
	int nBufferSize = 32;
	for ( int i = 0; i < r_bloomBufferSize.GetInteger() && i < 5; i++ )
		nBufferSize <<= 1;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	// create bloom texture
	renderSystem->CropRenderSize( nBufferSize, nBufferSize, true, true );
	renderSystem->SetColor4( r_bloomGamma.GetFloat(), 1.0f, 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, bloomMaterial );
	renderSystem->CaptureRenderToImage( "_bloom" );

	for ( int i = 0; i < r_bloomBlurIterations.GetInteger(); i++ ) {
		renderSystem->SetColor4( r_bloomBlurScaleX.GetFloat(), 1.0f, 1.0f, 1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, bloomMaterial );
		renderSystem->CaptureRenderToImage( "_bloom" );
		renderSystem->SetColor4( r_bloomBlurScaleY.GetFloat(), 1.0f, 1.0f, 2.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, bloomMaterial );
		renderSystem->CaptureRenderToImage( "_bloom" );
	}
	renderSystem->UnCrop();

	// blend original and bloom textures
	renderSystem->SetColor4( r_bloomScale.GetFloat(), 1.0f, 1.0f, 3.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, bloomMaterial );
}

/*
===================
idPlayerView::PostFX_SSIL
===================
*/
void idPlayerView::PostFX_SSIL() {
	int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
	int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	if ( !( r_useCelShading.GetBool() && r_celShadingMethod.GetInteger() == 2 ) )
		RenderDepth( true );
	RenderNormals( false );

	renderSystem->CropRenderSize( nWidth, nHeight, true );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_ssil" );

	renderSystem->SetColor4( r_ssilRadius.GetFloat(), r_ssilAmount.GetFloat(), 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssilMaterial );
	renderSystem->CaptureRenderToImage( "_ssil" );
	// blur ssil buffer
	for ( int i = 0; i < r_ssilBlurQuality.GetInteger(); i++ ) {
		renderSystem->SetColor4( r_ssilBlurScale.GetFloat(), 0.0f, r_ssilBlurEpsilon.GetFloat(), ( r_ssilBlurMethod.GetFloat() + 1.0f ) );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssilMaterial );
		renderSystem->CaptureRenderToImage( "_ssil" );
		renderSystem->SetColor4( 0.0f, r_ssilBlurScale.GetFloat(), r_ssilBlurEpsilon.GetFloat(), ( r_ssilBlurMethod.GetFloat() + 1.0f ) );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssilMaterial );
		renderSystem->CaptureRenderToImage( "_ssil" );
	}
	renderSystem->UnCrop();

	// blend scene with ssil buffer
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 3.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssilMaterial );
}

/*
===================
idPlayerView::PostFX_SSAO
===================
*/
void idPlayerView::PostFX_SSAO() {
	int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
	int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	if ( !r_useSSIL.GetBool() && !( r_useCelShading.GetBool() && r_celShadingMethod.GetInteger() == 2 ) )
		RenderDepth( true );

	renderSystem->CropRenderSize( nWidth, nHeight, true );
	
	// sample occlusion using our depth buffer
	renderSystem->SetColor4( r_ssaoRadius.GetFloat(), r_ssaoBias.GetFloat(), r_ssaoAmount.GetFloat(), ( r_ssaoMethod.GetFloat() < 0.0f ? 0.0f : r_ssaoMethod.GetFloat() ) );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssaoMaterial );
	renderSystem->CaptureRenderToImage( "_ssao" );
	// blur ssao buffer
	for ( int i = 0; i < r_ssaoBlurQuality.GetInteger(); i++ ) {
		renderSystem->SetColor4( r_ssaoBlurScale.GetFloat(), 0.0f, r_ssaoBlurEpsilon.GetFloat(), -( r_ssaoBlurMethod.GetFloat() + 1.0f ) );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssaoMaterial );
		renderSystem->CaptureRenderToImage( "_ssao" );
		if ( r_ssaoBlurMethod.GetInteger() >= 2 ) {
			renderSystem->SetColor4( 0.0f, r_ssaoBlurScale.GetFloat(), r_ssaoBlurEpsilon.GetFloat(), -( r_ssaoBlurMethod.GetFloat() + 1.0f ) );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssaoMaterial );
			renderSystem->CaptureRenderToImage( "_ssao" );
		}
	}
	renderSystem->UnCrop();

	// modulate scene with ssao buffer
	renderSystem->SetColor4( r_ssaoBlendPower.GetFloat(), r_ssaoBlendScale.GetFloat(), 1.0f, -5.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, ssaoMaterial );
}

/*
===================
idPlayerView::PostFX_SunShafts
===================
*/
void idPlayerView::PostFX_SunShafts() {
	idMat3 axis;
	idVec3 origin;
	idVec3 viewVector[3];
	player->GetViewPos( origin, axis );
	player->viewAngles.ToVectors( &viewVector[0], &viewVector[1], &viewVector[2] );

	idVec3 sunOrigin	= idVec3( r_sunOriginX.GetFloat(), r_sunOriginY.GetFloat(), r_sunOriginZ.GetFloat() ) ;
	idVec3 dist			= origin - sunOrigin;
	float length		= dist.Length();
	idVec3 sunVector	= dist / length;

	float VdotS[3];
	for ( int i = 0; i < 3; i++ ) {
		VdotS[i] = viewVector[i] * -sunVector;
	}
//	float sign = VdotS[0];
//	VdotS[0] = idMath::ClampFloat( 0.0f, 1.0f, VdotS[0] );
	idVec3 ndc;
	renderSystem->GlobalToNormalizedDeviceCoordinates( sunOrigin, ndc );
	ndc.x = ndc.x * 0.5 + 0.5;
	ndc.y = ndc.y * 0.5 + 0.5;
	
	int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
	int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;
	renderSystem->CaptureRenderToImage( "_currentRender" );

	if ( !r_useSSIL.GetBool() && !r_useSSAO.GetBool() && !( r_useCelShading.GetBool() && r_celShadingMethod.GetInteger() == 2 ) )
		RenderDepth( true );

	renderSystem->CropRenderSize( nWidth, nHeight, true, true );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_sunShafts" );

	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, sunShaftsMaterial );
	renderSystem->CaptureRenderToImage( "_sunShafts" );

	renderSystem->SetColor4( VdotS[0], 1.0f, 1.0f, 2.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, sunShaftsMaterial );
	renderSystem->CaptureRenderToImage( "_sunShaftsMask" );

	// blur textures
	for ( int i = 0; i < r_sunShaftsQuality.GetInteger(); i++ ) {
		renderSystem->SetColor4( r_sunShaftsSize.GetFloat(), ndc.x, ndc.y, 3.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, sunShaftsMaterial );
		renderSystem->CaptureRenderToImage( "_sunShafts" );
		renderSystem->SetColor4( r_sunShaftsSize.GetFloat(), ndc.x, ndc.y, 4.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, sunShaftsMaterial );
		renderSystem->CaptureRenderToImage( "_sunShaftsMask" );
	}
	renderSystem->UnCrop();

	// add mask to scene
	renderSystem->SetColor4( r_sunShaftsStrength.GetFloat(), r_sunShaftsMaskStrength.GetFloat(), 1.0f, 5.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, sunShaftsMaterial );
}

/*
===================
idPlayerView::PostFX_LensFlare
===================
*/
void idPlayerView::PostFX_LensFlare() {
	idMat3 axis;
	idVec3 origin;
	idVec3 viewVector[3];
	player->GetViewPos( origin, axis );
	player->viewAngles.ToVectors( &viewVector[0], &viewVector[1], &viewVector[2] );

	idVec3 sunOrigin	= idVec3( r_sunOriginX.GetFloat(), r_sunOriginY.GetFloat(), r_sunOriginZ.GetFloat() ) ;
	idVec3 dist			= origin - sunOrigin;
	idVec3 sunVector	= dist / dist.Length();

	float VdotS[ 3 ];
	for ( int i = 0; i < 3; i++ ) {
		VdotS[ i ] = viewVector[ i ] * -sunVector;
	}
	VdotS[ 0 ] = idMath::ClampFloat( 0.0f, 1.0f, VdotS[ 0 ] );
	VdotS[ 0 ] *= VdotS[ 0 ];

	
	if ( VdotS[ 0 ] > 0 ) {
		trace_t trace;
		gameLocal.clip.TracePoint( trace, origin, sunOrigin, MASK_SOLID, player );

		if ( trace.c.material->NoFragment() || trace.fraction == 1.0f ) { // Trace succeeded, or it hit a skybox
			float strength = VdotS[ 0 ] * r_lensFlareStrength.GetFloat();
			float length;
			idVec3 ndc;
			idVec2 ssDir, ssDist, uv;

			renderSystem->GlobalToNormalizedDeviceCoordinates( sunOrigin, ndc );

			ndc.x = ndc.x * 0.5f + 0.5f;
			ndc.y = 1.0f - ( ndc.y * 0.5f + 0.5f );
			ssDist.x = 0.5f - ndc.x;
			ssDist.y = 0.5f - ndc.y;
			length = ssDist.Length();
			ssDir = ssDist / length;

			// Draw a lens flare on the screen
			uv.x = ( ndc.x * SCREEN_WIDTH ) - 256.0f;
			uv.y = ( ndc.y * SCREEN_HEIGHT ) - 256.0f;
			renderSystem->SetColor4( VdotS[ 0 ], VdotS[ 0 ], VdotS[ 0 ], 0.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 512.0f, 512.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * -1.25f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 32.0f;
			uv.y = ( ( length * -1.25f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 32.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 1.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 64.0f, 64.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * 0.05f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 16.0f;
			uv.y = ( ( length * 0.05f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 16.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 1.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 32.0f, 32.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * -0.3333f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 16.0f;
			uv.y = ( ( length * -0.3333f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 16.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 2.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 32.0f, 32.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * 0.75f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 64.0f;
			uv.y = ( ( length * 0.75f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 64.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 2.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 128.0f, 128.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * 0.15f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 32.0;
			uv.y = ( ( length * 0.15f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 32.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 3.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 64.0f, 64.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * -0.6f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 16.0;
			uv.y = ( ( length * -0.6f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 16.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 3.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 32.0f, 32.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * -0.1f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 16.0;
			uv.y = ( ( length * -0.1f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 16.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 3.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 32.0f, 32.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * 0.4f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 16.0f;
			uv.y = ( ( length * 0.4f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 16.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 4.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 32.0f, 32.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * -0.4f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 32.0f;
			uv.y = ( ( length * -0.4f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 32.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 4.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 64.0f, 64.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );

			uv.x = ( ( length * 0.5f * ssDir.x + 0.5f ) * SCREEN_WIDTH ) - 128.0f;
			uv.y = ( ( length * 0.5f * ssDir.y + 0.5f ) * SCREEN_HEIGHT ) - 128.0f;
			renderSystem->SetColor4( strength * 0.1, strength * 0.3, strength * 1.0, 5.0f );
			renderSystem->DrawStretchPic( uv.x, uv.y, 256.0f, 256.0f, 0.0f, 1.0f, 1.0f, 0.0f, lensFlareMaterial );
		}
	}
}

/*
===================
idPlayerView::PostFX_DoF
===================
*/
void idPlayerView::PostFX_DoF() {
	if ( DoFConditionCheck() ) {
		int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
		int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;

		renderSystem->CaptureRenderToImage( "_currentRender" );

		if ( !(r_useCelShading.GetBool() && r_celShadingMethod.GetInteger() == 2) && !r_useSSAO.GetBool() && !r_useSSIL.GetBool() && !r_useSunShafts.GetBool() )
			RenderDepth( true );

		renderSystem->CropRenderSize( nWidth, nHeight, true, true );
		if ( r_useDepthOfField.GetInteger() == 2 )
			renderSystem->SetColor4( r_dofNear.GetInteger(), r_dofFocus.GetInteger(), r_dofFar.GetInteger(), 2.0f );
		else if ( gameLocal.inCinematic )
			renderSystem->SetColor4( r_dofNear.GetInteger(), r_dofFocus.GetInteger(), r_dofFar.GetInteger(), 2.0f );	// don't blur in front of the focal plane for cinematics
		else if ( player->weapon.GetEntity()->IsReloading() )
			renderSystem->SetColor4( -1.0f, 0.5f, 64.0f, 2.0f );	// use specific settings for reloading dof
		else if ( player->bIsZoomed )
			renderSystem->SetColor4( player->focusDistance, 1.0f, 1.0f, 1.0f );	// zoom uses a mask texture
		else
			renderSystem->SetColor4( player->focusDistance, 1.0f, 1.0f, 0.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, dofMaterial );
		renderSystem->CaptureRenderToImage( "_dof" );
		renderSystem->UnCrop();

		// blur scene using our depth of field mask
		renderSystem->SetColor4( r_dofBlurScale.GetFloat(), r_dofBlurQuality.GetInteger(), 1.0f, 2.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, dofMaterial );
		if ( r_dofBlurQuality.GetInteger() == 2 ) {
			renderSystem->CaptureRenderToImage( "_currentRender" );
			renderSystem->SetColor4( r_dofBlurScale.GetFloat(), r_dofBlurQuality.GetInteger() + 1.0f, 1.0f, 2.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, dofMaterial );
		}
	}
}

/*
===================
idPlayerView::DoFConditionCheck
===================
*/
bool idPlayerView::DoFConditionCheck() {
	if ( r_dofConditionCinematic.GetBool() && gameLocal.inCinematic ||
		 r_dofConditionGUI.GetBool() && player->GuiActive() ||
		 r_dofConditionReload.GetBool() && player->weapon.GetEntity()->IsReloading() ||
		 r_dofConditionTalk.GetBool() && player->GetTalkCursor() ||
		 r_dofConditionZoom.GetBool() && player->bIsZoomed ||
		 r_dofConditionAlways.GetBool() )
		return true;
	else
		return false;
}

/*
===================
idPlayerView::PostFX_MotionBlur
===================
*/
void idPlayerView::PostFX_MotionBlur() {
	if ( MBConditionCheck() ) {
		int	nWidth		= renderSystem->GetScreenWidth() / 2.0f;
		int	nHeight		= renderSystem->GetScreenHeight() / 2.0f;
		int nQuality	= idMath::ClampInt( 1, 4, r_motionBlurQuality.GetInteger() );
		float fFPS		= idMath::ClampFloat( 0.0f, 1.0f, 1.0f / ( (float)( gameLocal.time - prevTime ) * 0.06f ) );

		float parm[6];
		parm[0] = player->viewAngles.yaw - mbPrevAngles.yaw;
		parm[1] = player->viewAngles.pitch - mbPrevAngles.pitch;
		if ( parm[0] > 180.0f ) {
			parm[0] -= 360.0f;
			mbPrevAngles.yaw += 360.0f;
		}
		if ( parm[0] < -180.0f ) {
			parm[0] += 360.0f;
			mbPrevAngles.yaw -= 360.0f;
		}
		parm[0] = idMath::ClampFloat( -r_motionBlurMaxThreshold.GetInteger(), r_motionBlurMaxThreshold.GetInteger(), parm[0] );
		parm[1] = idMath::ClampFloat( -r_motionBlurMaxThreshold.GetInteger(), r_motionBlurMaxThreshold.GetInteger(), parm[1] );
		float f = idMath::Fabs( player->viewAngles.pitch ) / 90.0f * 0.5;
		parm[2] = player->viewAngles.pitch < 0.0f ? 1.0 - f : f;
		parm[3] = idMath::Fabs( parm[0] ) > idMath::Fabs( parm[1] ) ? idMath::Fabs( player->viewAngles.pitch ) / 90.0f : 0.0f;
		parm[4] = r_motionBlurFactor.GetFloat();

		parm[0] *= r_motionBlurScale.GetFloat() * fFPS;
		parm[1] *= r_motionBlurScale.GetFloat() * fFPS;

		renderSystem->CaptureRenderToImage( "_currentRender" );

		if ( !( r_useCelShading.GetBool() && r_celShadingMethod.GetInteger() == 2 ) && !r_useSSAO.GetBool() && !r_useSSIL.GetBool() && !r_useSunShafts.GetBool() && !( r_useDepthOfField.GetBool() && DoFConditionCheck() ) )
			RenderDepth( true );

		renderSystem->CropRenderSize( nWidth, nHeight, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
		renderSystem->CaptureRenderToImage( "_mbXY" );
		renderSystem->CaptureRenderToImage( "_mbZ" );
		for ( int i = 0; i < nQuality; i++ ) {
			renderSystem->SetColor4( parm[0], parm[1], r_motionBlurMaskDistance.GetFloat(), 0.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
			renderSystem->CaptureRenderToImage( "_mbXY" );
			renderSystem->SetColor4( parm[0], parm[2], r_motionBlurMaskDistance.GetFloat(), 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
			renderSystem->CaptureRenderToImage( "_mbZ" );
		}
		renderSystem->UnCrop();
		
		renderSystem->SetColor4( parm[3], parm[4], r_motionBlurMaskDistance.GetFloat(), 2.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
	}

	float fLerp = idMath::ClampFloat( 0.0f, 0.99f, r_motionBlurLerp.GetFloat() );
	mbPrevAngles = mbPrevAngles * fLerp + player->viewAngles * ( 1.0 - fLerp );
}

/*
===================
idPlayerView::MBConditionCheck
===================
*/
bool idPlayerView::MBConditionCheck() {
	int nThreshold = r_motionBlurMinThreshold.GetInteger() < 1 ? 1 : r_motionBlurMinThreshold.GetInteger();

	if ( gameLocal.inCinematic || player->GuiActive() || ( gameLocal.time - prevTime ) > SEC2MS( 1.0f / r_motionBlurFPSThreshold.GetFloat() )  )
		return false;

	if ( ( player->viewAngles.pitch >= mbPrevAngles.pitch + nThreshold ) ||
		 ( player->viewAngles.pitch <= mbPrevAngles.pitch - nThreshold ) ||
		 ( player->viewAngles.yaw >= mbPrevAngles.yaw + nThreshold ) ||
		 ( player->viewAngles.yaw <= mbPrevAngles.yaw - nThreshold ) )
		return true;
	else
		return false;
}

/*
===================
idPlayerView::PostFX_ColorGrading
===================
*/
void idPlayerView::PostFX_ColorGrading() {
	int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
	int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	// unsharp mask buffer
	renderSystem->CropRenderSize( nWidth, nHeight, true, true );
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, colorGradingMaterial );
	renderSystem->CaptureRenderToImage( "_unsharpMask" );
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, colorGradingMaterial );
	renderSystem->CaptureRenderToImage( "_unsharpMask" );
	renderSystem->UnCrop();

	renderSystem->SetColor4( r_colorGradingParm.GetInteger(), 1.0f, 1.0f, 2.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, colorGradingMaterial );
}

/*
===================
idPlayerView::PostFX_IRGoggles
===================
*/
void idPlayerView::PostFX_IRGoggles() {
	if ( player->bIRGogglesOn && !player->PowerUpActive( BERSERK ) ) {
		renderSystem->CaptureRenderToImage( "_currentRender" );
		renderSystem->SetColor4( renderSystem->GetScreenWidth() / 256.0f, renderSystem->GetScreenHeight() / 256.0f, 1.0f, g_goggleType.GetFloat() );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, irGogglesMaterial );
	}
}

/*
===================
idPlayerView::PostFX_Vignetting
===================
*/
void idPlayerView::PostFX_Vignetting() {
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f  );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, vignettingMaterial );
}

/*
===================
idPlayerView::PostFX_Filmgrain
===================
*/
void idPlayerView::PostFX_Filmgrain() {
	float size = 128.0f * r_filmgrainScale.GetFloat();
	renderSystem->SetColor4( renderSystem->GetScreenWidth() / size, renderSystem->GetScreenHeight() / size, r_filmgrainStrength.GetFloat(), r_filmgrainBlendMode.GetInteger() );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, filmgrainMaterial );
}

/*
===================
idPlayerView::PostFX_DoubleVision
===================
*/
void idPlayerView::PostFX_DoubleVision() {
	int	nWidth	= renderSystem->GetScreenWidth() / 2;
	int	nHeight	= renderSystem->GetScreenHeight() / 2;
	int offset	= dvFinishTime - gameLocal.time;
	float scale	= ( offset * g_dvAmplitude.GetFloat() ) > 0.5f ? 0.5f : offset * g_dvAmplitude.GetFloat();
	float shift	= fabs( scale * sin( sqrtf( offset ) * g_dvFrequency.GetFloat() ) ); 

	renderSystem->CaptureRenderToImage( "_currentRender" );

	// if double vision, render to a texture
	renderSystem->CropRenderSize( nWidth, nHeight, true, true );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_scratch" );
	renderSystem->UnCrop();

	// carry red tint if in berserk mode
	idVec4 color( 1.0f, 1.0f, 1.0f, 1.0f );
	if ( gameLocal.time < player->inventory.powerupEndTime[ BERSERK ] )
		color.y = color.z = 0.0f;

	renderSystem->SetColor4( color.x, color.y, color.z, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, shift, 1.0f, 1.0f, 0.0f, scratchMaterial );
	renderSystem->SetColor4( color.x, color.y, color.z, 0.5f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f - shift, 0.0f, scratchMaterial );
}

/*
===================
idPlayerView::PostFX_BerserkVision
===================
*/
void idPlayerView::PostFX_BerserkVision() {
	int berserkTime = player->inventory.powerupEndTime[ BERSERK ] - gameLocal.time;
	if ( berserkTime > 0 ) {
		// start fading if within 10 seconds of going away
		float alpha = ( berserkTime < 10000 ) ? (float)berserkTime / 10000 : 1.0f;
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, alpha );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, berserkMaterial );
	}

	int	nWidth = renderSystem->GetScreenWidth() / 2;
	int	nHeight = renderSystem->GetScreenHeight() / 2;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	// if double vision, render to a texture
	renderSystem->CropRenderSize( nWidth, nHeight, true, true );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_scratch" );
	renderSystem->UnCrop();

	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, scratchMaterial );
}

/*
===================
idPlayerView::PostFX_InfluenceVision
===================
*/
void idPlayerView::PostFX_InfluenceVision() {
	float distance = 0.0f;
	float pct = 1.0f;

	if ( player->GetInfluenceEntity() ) {
		distance = ( player->GetInfluenceEntity()->GetPhysics()->GetOrigin() - player->GetPhysics()->GetOrigin() ).Length();
		if ( player->GetInfluenceRadius() != 0.0f && distance < player->GetInfluenceRadius() ) {
			pct = distance / player->GetInfluenceRadius();
			pct = 1.0f - idMath::ClampFloat( 0.0f, 1.0f, pct );
		}
	}
	if ( player->GetInfluenceMaterial() ) {
		renderSystem->CaptureRenderToImage( "_currentRender" );
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, pct );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, player->GetInfluenceMaterial() );
	} else if ( player->GetInfluenceEntity() ) {
		int offset = 25 + sinf( gameLocal.time );
//		PostFX_DoubleVision( view, pct * offset );
	}
}

/*
===================
idPlayerView::PostFX_BFGVision
===================
*/
void idPlayerView::PostFX_BFGVision() {
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, bfgMaterial );
}

/*
===================
idPlayerView::PostFX_TunnelVision
===================
*/
void idPlayerView::PostFX_TunnelVision() {
	float health = 0.0f;
	float alpha;

	if ( g_testHealthVision.GetFloat() != 0.0f )
		health = g_testHealthVision.GetFloat();
	else
		health = player->health;

// sikk---> Regenerative Health System
	if ( g_useHealthRegeneration.GetBool() )
		alpha = idMath::ClampFloat( 0.0f, 1.0f, health / g_healthRegenFeedback.GetFloat() );
	else
		alpha = idMath::ClampFloat( 0.0f, 1.0f, health / 100.0f );
// <---sikk

	if ( alpha < 1.0f  ) {
// sikk---> Regenerative Health System ( also fix for "g_testHealthVision" )
		if ( g_useHealthRegeneration.GetBool() || g_testHealthVision.GetFloat() )
			renderSystem->SetColor4( MS2SEC( gameLocal.time ), 1.0f, 1.0f, alpha );
		else
			renderSystem->SetColor4( ( player->health <= 0.0f ) ? MS2SEC( gameLocal.time ) : lastDamageTime, 1.0f, 1.0f, ( player->health <= 0.0f ) ? 0.0f : alpha );
		if ( g_useHealthRegeneration.GetBool() )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, tunnel2Material );
		else
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, tunnelMaterial );
// <---sikk
	}
}

/*
===================
idPlayerView::PostFX_ScreenBlobs
===================
*/
void idPlayerView::PostFX_ScreenBlobs() {
	for ( int i = 0; i < MAX_SCREEN_BLOBS; i++ ) {
		screenBlob_t *blob = &screenBlobs[ i ];

		if ( blob->finishTime <= gameLocal.time )
			continue;

		blob->y += blob->driftAmount;

		float fade = (float)( blob->finishTime - gameLocal.time ) / ( blob->finishTime - blob->startFadeTime );

		if ( fade > 1.0f )
			fade = 1.0f;

		if ( fade ) {
			renderSystem->CaptureRenderToImage( "_currentRender" );	// sikk - Bloodspray Fix

			renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, fade );
			renderSystem->DrawStretchPic( blob->x, blob->y, blob->w, blob->h, blob->s1, blob->t1, blob->s2, blob->t2, blob->material );
		}
	}
}

/*
===================
idPlayerView::PostFX_ArmorPulse
===================
*/
void idPlayerView::PostFX_ArmorPulse() {
	float armorPulse = ( gameLocal.time - player->lastArmorPulse ) / 250.0f;

	if ( armorPulse > 0.0f && armorPulse < 1.0f ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f - armorPulse );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, armorMaterial );
	}
}
// <---sikk
