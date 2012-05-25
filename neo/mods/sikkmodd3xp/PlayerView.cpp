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

// _D3XP : rename all gameLocal.time to gameLocal.slow.time for merge!

#ifdef _D3XP
static int MakePowerOfTwo( int num ) {
	int	pot;
	for ( pot = 1 ; pot < num; pot <<=1 ) {}
	return pot;
}
#endif

const int IMPULSE_DELAY = 150;
/*
==============
idPlayerView::idPlayerView
==============
*/
idPlayerView::idPlayerView() {
	memset( screenBlobs, 0, sizeof( screenBlobs ) );
	memset( &view, 0, sizeof( view ) );
	player					= NULL;

	dvMaterial				= declManager->FindMaterial( "_scratch" );
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
	hdrFlareMaterial		= declManager->FindMaterial( "postProcess/hdrFlare" );
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
	explosionFXMaterial		= declManager->FindMaterial( "postProcess/explosionFX" );
	screenFrostMaterial		= declManager->FindMaterial( "postProcess/screenFrost" );
	celShadingMaterial		= declManager->FindMaterial( "postProcess/celShading" );
	filmgrainMaterial		= declManager->FindMaterial( "postProcess/filmgrain" );
	vignettingMaterial		= declManager->FindMaterial( "postProcess/vignetting" );
	tunnel2Material			= declManager->FindMaterial( "postProcess/tunnel" );
	adrenalineMaterial		= declManager->FindMaterial( "postProcess/adrenaline" );
	bSoftShadows			= false;
	bDepthRendered			= false;
	bDitherRendered			= false;
	focusDistance			= 0.0f;
	prevViewAngles.Zero();
// <---sikk

// sikk---> Explosion FX PostProcess
	gameLocal.explosionOrigin.Zero();
	gameLocal.explosionRadius = 0;
	gameLocal.explosionDamage = 0;
	gameLocal.explosionTime = 0;
// <---sikk

	bfgVision = false;
	dvFinishTime = 0;
	kickFinishTime = 0;
	kickAngles.Zero();
	lastDamageTime = 0.0f;
	fadeTime = 0;
	fadeRate = 0.0;
	fadeFromColor.Zero();
	fadeToColor.Zero();
	fadeColor.Zero();
	shakeAng.Zero();
#ifdef _D3XP
	fxManager = NULL;

	if ( !fxManager ) {
		fxManager = new FullscreenFXManager;
		fxManager->Initialize( this );
	}
#endif

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
	for ( i = 0; i < MAX_SCREEN_BLOBS; i++, blob++ ) {
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
	savefile->WriteMaterial( dvMaterial );
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

#ifdef _D3XP
	if ( fxManager )
		fxManager->Save( savefile );
#endif
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
	savefile->ReadMaterial( dvMaterial );
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

#ifdef _D3XP
	if ( fxManager )
		fxManager->Restore( savefile );
#endif
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
	lastDamageTime = MS2SEC( gameLocal.slow.time - 99999 );

	dvFinishTime = ( gameLocal.fast.time - 99999 );
	kickFinishTime = ( gameLocal.slow.time - 99999 );

	for ( int i = 0 ; i < MAX_SCREEN_BLOBS ; i++ )
		screenBlobs[i].finishTime = gameLocal.slow.time;

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

	for ( int i = 1 ; i < MAX_SCREEN_BLOBS ; i++ )
		if ( screenBlobs[i].finishTime < oldest->finishTime )
			oldest = &screenBlobs[i];

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
	// double vision effect
	if ( lastDamageTime > 0.0f && SEC2MS( lastDamageTime ) + IMPULSE_DELAY > gameLocal.slow.time )
		return;	// keep shotgun from obliterating the view

	float	dvTime = damageDef->GetFloat( "dv_time" );
	if ( dvTime ) {
		if ( dvFinishTime < gameLocal.fast.time )
			dvFinishTime = gameLocal.fast.time;

		dvFinishTime += g_dvTime.GetFloat() * dvTime;

		// don't let it add up too much in god mode
		if ( dvFinishTime > gameLocal.fast.time + 5000 )
			dvFinishTime = gameLocal.fast.time + 5000;
	}

	// head angle kick
	float kickTime = damageDef->GetFloat( "kick_time" );
	if ( kickTime ) {
		kickFinishTime = gameLocal.slow.time + g_kickTime.GetFloat() * kickTime;
		// forward / back kick will pitch view
		kickAngles[0] = localKickDir[0];
		// side kick will yaw view
		kickAngles[1] = localKickDir[1]*0.5f;
		// up / down kick will pitch view
		kickAngles[0] += localKickDir[2];
		// roll will come from  side
		kickAngles[2] = localKickDir[1];

		float kickAmplitude = damageDef->GetFloat( "kick_amplitude" );
		if ( kickAmplitude )
			kickAngles *= kickAmplitude;
	}

	// screen blob
	float blobTime = damageDef->GetFloat( "blob_time" );
	if ( blobTime ) {
		screenBlob_t *blob = GetScreenBlob();
		blob->startFadeTime = gameLocal.slow.time;
		blob->finishTime = gameLocal.slow.time + blobTime * g_blobTime.GetFloat() * ( (float)gameLocal.msec / USERCMD_MSEC );

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
		blob->s1 = 0.0f;
		blob->t1 = 0.0f;
		blob->s2 = 1.0f;
		blob->t2 = 1.0f;
	}

	// save lastDamageTime for tunnel vision accentuation
	lastDamageTime = MS2SEC( gameLocal.slow.time );
}

/*
==================
idPlayerView::AddBloodSpray

If we need a more generic way to add blobs then we can do that
but having it localized here lets the material be pre-looked up etc.
==================
*/
void idPlayerView::AddBloodSpray( float duration ) {
	if ( duration <= 0 || bloodSprayMaterial == NULL || g_skipViewEffects.GetBool() )
		return;

// sikk---> Blood Spray Screen Effect
	// Use random material
	if ( gameLocal.random.RandomFloat() < 0.5f )
		bloodSprayMaterial = declManager->FindMaterial( "postProcess/bloodspray1" );
	else
		bloodSprayMaterial = declManager->FindMaterial( "postProcess/bloodspray2" );
// <---sikk

	// visit this for chainsaw
	screenBlob_t *blob = GetScreenBlob();
	blob->startFadeTime = gameLocal.slow.time;
	blob->finishTime = gameLocal.slow.time + ( duration * 1000 );
	blob->material = bloodSprayMaterial;
	blob->x = ( gameLocal.random.RandomInt() & 63 ) - 32;
	blob->y = ( gameLocal.random.RandomInt() & 63 ) - 32;
	blob->driftAmount = 0.5f + gameLocal.random.CRandomFloat() * 0.5;
	float scale = ( 256 + ( ( gameLocal.random.RandomInt()&63 ) - 32 ) ) / 256.0f;
	blob->w = 640 * g_blobSize.GetFloat() * scale;	// sikk - This was "600". Typo?
	blob->h = 480 * g_blobSize.GetFloat() * scale;
	float s1 = 0.0f;
	float t1 = 0.0f;
	float s2 = 1.0f;
	float t2 = 1.0f;
// sikk---> No more drifting
/*	if ( blob->driftAmount < 0.6 ) {
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
	}*/
	float f = gameLocal.random.CRandomFloat();
	if ( f < 0.25 ) {
		s1 = 1.0f;
		s2 = 0.0f;
	} else if ( f < 0.5 ) {
		t1 = 1.0f;
		t2 = 0.0f;
	} else if ( f < 0.75 ) {
		s1 = 1.0f;
		s2 = 0.0f;
		t1 = 1.0f;
		t2 = 0.0f;
	}
// <---sikk	blob->s1 = s1;
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
	int recoilTime;

	recoilTime = weaponDef->GetInt( "recoilTime" );
	// don't shorten a damage kick in progress
	if ( recoilTime && kickFinishTime < gameLocal.slow.time ) {
		idAngles angles;
		weaponDef->GetAngles( "recoilAngles", "5 0 0", angles );
		kickAngles = angles;
		int	finish = gameLocal.slow.time + g_kickTime.GetFloat() * recoilTime;
		kickFinishTime = finish;
	}

}

/*
===================
idPlayerView::CalculateShake
===================
*/
void idPlayerView::CalculateShake() {
	idVec3 origin, matrix;

	float shakeVolume = gameSoundWorld->CurrentShakeAmplitudeForPosition( gameLocal.slow.time, player->firstPersonViewOrigin );

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

	if ( gameLocal.slow.time < kickFinishTime ) {
		float offset = kickFinishTime - gameLocal.slow.time;

		ang = kickAngles * offset * offset * g_kickAmplitude.GetFloat();

		for ( int i = 0 ; i < 3 ; i++ ) {
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
void idPlayerView::SingleView( idUserInterface *hud, const renderView_t *view ) {
	// normal rendering
	if ( !view )
		return;

	// place the sound origin for the player
	gameSoundWorld->PlaceListener( view->vieworg, view->viewaxis, player->entityNumber + 1, gameLocal.slow.time, hud ? hud->State().GetString( "location" ) : "Undefined" );

	// if the objective system is up, don't do normal drawing
	if ( player->objectiveSystemOpen ) {
		player->objectiveSystem->Redraw( gameLocal.fast.time );
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
	renderView_t	hackedView = *view;
	hackedView.viewaxis = hackedView.viewaxis * ShakeAxis();

#ifdef _D3XP
	if ( gameLocal.portalSkyEnt.GetEntity() && gameLocal.IsPortalSkyAcive() && g_enablePortalSky.GetBool() ) {
		renderView_t	portalView = hackedView;
		portalView.vieworg = gameLocal.portalSkyEnt.GetEntity()->GetPhysics()->GetOrigin();

		// setup global fixup projection vars
		hackedView.shaderParms[4] = shiftScale.x;
		hackedView.shaderParms[5] = shiftScale.y;

		gameRenderWorld->RenderScene( &portalView );
		renderSystem->CaptureRenderToImage( "_currentRender" );

		hackedView.forceUpdate = true;				// FIX: for smoke particles not drawing when portalSky present
	}

#endif

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
#ifdef _D3XP
	SetTimeState ts( player->timeGroup );
#endif

	if ( !fadeTime )
		fadeFromColor.Set( 0.0f, 0.0f, 0.0f, 1.0f - color[ 3 ] );
	else
		fadeFromColor = fadeColor;

	fadeToColor = color;

	if ( time <= 0 ) {
		fadeRate = 0;
		time = 0;
		fadeColor = fadeToColor;
	} else {
		fadeRate = 1.0f / ( float )time;
	}

	if ( gameLocal.realClientTime == 0 && time == 0 )
		fadeTime = 1;
	else
		fadeTime = gameLocal.realClientTime + time;
}

/*
=================
idPlayerView::ScreenFade
=================
*/
void idPlayerView::ScreenFade() {
	int		msec;
	float	t;

	if ( !fadeTime )
		return;

#ifdef _D3XP
	SetTimeState ts( player->timeGroup );
#endif

	msec = fadeTime - gameLocal.realClientTime;

	if ( msec <= 0 ) {
		fadeColor = fadeToColor;
		if ( fadeColor[ 3 ] == 0.0f )
			fadeTime = 0;
	} else {
		t = ( float )msec * fadeRate;
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

	SingleView( hud, view );

	// process the frame
	if ( !player->objectiveSystemOpen )
		fxManager->Process( &hackedView );

	if ( !g_skipViewEffects.GetBool() && !player->objectiveSystemOpen )
		DoPostFX();

	// if the objective system is up, don't draw hud
	if ( hud && !player->objectiveSystemOpen )
		player->DrawHUD( hud );

	ScreenFade();

	if ( net_clientLagOMeter.GetBool() && lagoMaterial && gameLocal.isClient ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
		renderSystem->DrawStretchPic( 10.0f, 380.0f, 64.0f, 64.0f, 0.0f, 0.0f, 1.0f, 1.0f, lagoMaterial );
	}
    
	prevTime = gameLocal.time;	// sikk - update prevTime
}

#ifdef _D3XP
/*
===================
idPlayerView::WarpVision
===================
*/
int idPlayerView::AddWarp( idVec3 worldOrigin, float centerx, float centery, float initialRadius, float durationMsec ) {
	FullscreenFX_Warp *fx = (FullscreenFX_Warp*)( fxManager->FindFX( "warp" ) );

	if ( fx ) {
		fx->EnableGrabber( true );
		return 1;
	}

	return 1;
}

void idPlayerView::FreeWarp( int id ) {
	FullscreenFX_Warp *fx = (FullscreenFX_Warp*)( fxManager->FindFX( "warp" ) );

	if ( fx ) {
		fx->EnableGrabber( false );
		return;
	}
}

/*
==================
FxFader::FxFader
==================
*/
FxFader::FxFader() {
	time = 0;
	state = FX_STATE_OFF;
	alpha = 0.0f;
	msec = 1000;
}

/*
==================
FxFader::SetTriggerState
==================
*/
bool FxFader::SetTriggerState( bool active ) {
	// handle on/off states
	if ( active && state == FX_STATE_OFF ) {
		state = FX_STATE_RAMPUP;
		time = gameLocal.slow.time + msec;
	} else if ( !active && state == FX_STATE_ON ) {
		state = FX_STATE_RAMPDOWN;
		time = gameLocal.slow.time + msec;
	}

	// handle rampup/rampdown states
	if ( state == FX_STATE_RAMPUP ) {
		if ( gameLocal.slow.time >= time )
			state = FX_STATE_ON;
	} else if ( state == FX_STATE_RAMPDOWN ) {
		if ( gameLocal.slow.time >= time )
			state = FX_STATE_OFF;
	}

	// compute alpha
	switch ( state ) {
		case FX_STATE_ON:		alpha = 1.0f; break;
		case FX_STATE_OFF:		alpha = 0.0f; break;
		case FX_STATE_RAMPUP:	alpha = 1.0f - (float)( time - gameLocal.slow.time ) / msec; break;
		case FX_STATE_RAMPDOWN:	alpha = (float)( time - gameLocal.slow.time ) / msec; break;
	}

	if ( alpha > 0.0f )
		return true;
	else
		return false;
}

/*
==================
FxFader::Save
==================
*/
void FxFader::Save( idSaveGame *savefile ) {
	savefile->WriteInt( time );
	savefile->WriteInt( state );
	savefile->WriteFloat( alpha );
	savefile->WriteInt( msec );
}

/*
==================
FxFader::Restore
==================
*/
void FxFader::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( time );
	savefile->ReadInt( state );
	savefile->ReadFloat( alpha );
	savefile->ReadInt( msec );
}

/*
==================
FullscreenFX_Helltime::Save
==================
*/
void FullscreenFX::Save( idSaveGame *savefile ) {
	fader.Save( savefile );
}

/*
==================
FullscreenFX_Helltime::Restore
==================
*/
void FullscreenFX::Restore( idRestoreGame *savefile ) {
	fader.Restore( savefile );
}

/*
==================
FullscreenFX_Helltime::Initialize
==================
*/
void FullscreenFX_Helltime::Initialize() {
	acInitMaterials[0] = declManager->FindMaterial( "textures/smf/bloodorb1/ac_init" );
	acInitMaterials[1] = declManager->FindMaterial( "textures/smf/bloodorb2/ac_init" );
	acInitMaterials[2] = declManager->FindMaterial( "textures/smf/bloodorb3/ac_init" );

	acCaptureMaterials[0] = declManager->FindMaterial( "textures/smf/bloodorb1/ac_capture" );
	acCaptureMaterials[1] = declManager->FindMaterial( "textures/smf/bloodorb2/ac_capture" );
	acCaptureMaterials[2] = declManager->FindMaterial( "textures/smf/bloodorb3/ac_capture" );

	acDrawMaterials[0] = declManager->FindMaterial( "textures/smf/bloodorb1/ac_draw" );
	acDrawMaterials[1] = declManager->FindMaterial( "textures/smf/bloodorb2/ac_draw" );
	acDrawMaterials[2] = declManager->FindMaterial( "textures/smf/bloodorb3/ac_draw" );

	crCaptureMaterials[0] = declManager->FindMaterial( "textures/smf/bloodorb1/cr_capture" );
	crCaptureMaterials[1] = declManager->FindMaterial( "textures/smf/bloodorb2/cr_capture" );
	crCaptureMaterials[2] = declManager->FindMaterial( "textures/smf/bloodorb3/cr_capture" );

	crDrawMaterials[0] = declManager->FindMaterial( "textures/smf/bloodorb1/cr_draw" );
	crDrawMaterials[1] = declManager->FindMaterial( "textures/smf/bloodorb2/cr_draw" );
	crDrawMaterials[2] = declManager->FindMaterial( "textures/smf/bloodorb3/cr_draw" );

	clearAccumBuffer = true;
}

/*
==================
FullscreenFX_Helltime::DetermineLevel
==================
*/
int FullscreenFX_Helltime::DetermineLevel() {
	idPlayer *player;
	int testfx = g_testHelltimeFX.GetInteger();

	// for testing purposes
	if ( testfx >= 0 && testfx < 3 )
		return testfx;

	player = fxman->GetPlayer();

	if ( player->PowerUpActive( INVULNERABILITY ) )
		return 2;
	else if ( player->PowerUpActive( BERSERK ) )
		return 1;
	else if ( player->PowerUpActive( HELLTIME ) )
		return 0;

	return -1;
}

/*
==================
FullscreenFX_Helltime::Active
==================
*/
bool FullscreenFX_Helltime::Active() {
	if ( gameLocal.inCinematic || gameLocal.isMultiplayer )
		return false;

	if ( DetermineLevel() >= 0 )
		return true;
	else if ( fader.GetAlpha() == 0 ) // latch the clear flag
		clearAccumBuffer = true;

	return false;
}

/*
==================
FullscreenFX_Helltime::AccumPass
==================
*/
void FullscreenFX_Helltime::AccumPass( const renderView_t *view ) {
	idVec2 shiftScale;
	int level = DetermineLevel();

	// for testing
	if ( level < 0 || level > 2 )
		level = 0;

	shiftScale = fxman->GetShiftScale();
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );

	// capture pass
	if ( clearAccumBuffer ) {
		clearAccumBuffer = false;
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, acInitMaterials[level] );
	} else {
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, acCaptureMaterials[level] );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, crCaptureMaterials[level] );
	}

	renderSystem->CaptureRenderToImage( "_accum" );
}

/*
==================
FullscreenFX_Helltime::HighQuality
==================
*/
void FullscreenFX_Helltime::HighQuality() {
	idVec2 shiftScale;
	int level = DetermineLevel();

	// for testing
	if ( level < 0 || level > 2 )
		level = 0;

	shiftScale = fxman->GetShiftScale();
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );

	// draw pass
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, acDrawMaterials[level] );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, crDrawMaterials[level] );
}

/*
==================
FullscreenFX_Helltime::Restore
==================
*/
void FullscreenFX_Helltime::Restore( idRestoreGame *savefile ) {
	FullscreenFX::Restore( savefile );

	// latch the clear flag
	clearAccumBuffer = true;
}

/*
==================
FullscreenFX_Multiplayer::Initialize
==================
*/
void FullscreenFX_Multiplayer::Initialize() {
	acInitMaterials		= declManager->FindMaterial( "textures/smf/multiplayer1/ac_init" );
	acCaptureMaterials	= declManager->FindMaterial( "textures/smf/multiplayer1/ac_capture" );
	acDrawMaterials		= declManager->FindMaterial( "textures/smf/multiplayer1/ac_draw" );
	crCaptureMaterials	= declManager->FindMaterial( "textures/smf/multiplayer1/cr_capture" );
	crDrawMaterials		= declManager->FindMaterial( "textures/smf/multiplayer1/cr_draw" );
	clearAccumBuffer	= true;
}

/*
==================
FullscreenFX_Multiplayer::DetermineLevel
==================
*/
int FullscreenFX_Multiplayer::DetermineLevel() {
	idPlayer *player;
	int testfx = g_testMultiplayerFX.GetInteger();

	// for testing purposes
	if ( testfx >= 0 && testfx < 3 )
		return testfx;

	player = fxman->GetPlayer();

	if ( player->PowerUpActive( INVULNERABILITY ) )
		return 2;
	//else if ( player->PowerUpActive( HASTE ) )
	//	return 1;
	else if ( player->PowerUpActive( BERSERK ) )
		return 0;

	return -1;
}

/*
==================
FullscreenFX_Multiplayer::Active
==================
*/
bool FullscreenFX_Multiplayer::Active() {

	if ( !gameLocal.isMultiplayer && g_testMultiplayerFX.GetInteger() == -1 )
		return false;

	if ( DetermineLevel() >= 0 ) 
		return true;
	else if ( fader.GetAlpha() == 0 ) // latch the clear flag
		clearAccumBuffer = true;

	return false;
}

/*
==================
FullscreenFX_Multiplayer::AccumPass
==================
*/
void FullscreenFX_Multiplayer::AccumPass( const renderView_t *view ) {
	idVec2 shiftScale;
	int level = DetermineLevel();

	// for testing
	if ( level < 0 || level > 2 )
		level = 0;

	shiftScale = fxman->GetShiftScale();
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );

	// capture pass
	if ( clearAccumBuffer ) {
		clearAccumBuffer = false;
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, acInitMaterials );
	} else {
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, acCaptureMaterials );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, crCaptureMaterials );
	}

	renderSystem->CaptureRenderToImage( "_accum" );
}

/*
==================
FullscreenFX_Multiplayer::HighQuality
==================
*/
void FullscreenFX_Multiplayer::HighQuality() {
	idVec2 shiftScale;
	int level = DetermineLevel();

	// for testing
	if ( level < 0 || level > 2 )
		level = 0;

	shiftScale = fxman->GetShiftScale();
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );

	// draw pass
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, acDrawMaterials );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, crDrawMaterials );
}

/*
==================
FullscreenFX_Multiplayer::Restore
==================
*/
void FullscreenFX_Multiplayer::Restore( idRestoreGame *savefile ) {
	FullscreenFX::Restore( savefile );

	// latch the clear flag
	clearAccumBuffer = true;
}

/*
==================
FullscreenFX_Warp::Initialize
==================
*/
void FullscreenFX_Warp::Initialize() {
	material = declManager->FindMaterial( "textures/smf/warp" );
	grabberEnabled = false;
	startWarpTime = 0;
}

/*
==================
FullscreenFX_Warp::Active
==================
*/
bool FullscreenFX_Warp::Active() {
	if ( grabberEnabled )
		return true;

	return false;
}

/*
==================
FullscreenFX_Warp::Save
==================
*/
void FullscreenFX_Warp::Save( idSaveGame *savefile ) {
	FullscreenFX::Save( savefile );

	savefile->WriteBool( grabberEnabled );
	savefile->WriteInt( startWarpTime );
}

/*
==================
FullscreenFX_Warp::Restore
==================
*/
void FullscreenFX_Warp::Restore( idRestoreGame *savefile ) {
	FullscreenFX::Restore( savefile );

	savefile->ReadBool( grabberEnabled );
	savefile->ReadInt( startWarpTime );
}

/*
==================
FullscreenFX_Warp::DrawWarp
==================
*/
void FullscreenFX_Warp::DrawWarp( WarpPolygon_t wp, float interp ) {
	idVec4 mid1_uv, mid2_uv;
	idVec4 mid1, mid2;
	idVec2 drawPts[6], shiftScale;
	WarpPolygon_t trans;

	trans = wp;
	shiftScale = fxman->GetShiftScale();

	// compute mid points
	mid1 = trans.outer1 * ( interp ) + trans.center * ( 1.0f - interp );
	mid2 = trans.outer2 * ( interp ) + trans.center * ( 1.0f - interp );
	mid1_uv = trans.outer1 * 0.5f + trans.center * 0.5f;
	mid2_uv = trans.outer2 * 0.5f + trans.center * 0.5f;

	// draw [outer1, mid2, mid1]
	drawPts[0].Set( trans.outer1.x, trans.outer1.y );
	drawPts[1].Set( mid2.x, mid2.y );
	drawPts[2].Set( mid1.x, mid1.y );
	drawPts[3].Set( trans.outer1.z, trans.outer1.w );
	drawPts[4].Set( mid2_uv.z, mid2_uv.w );
	drawPts[5].Set( mid1_uv.z, mid1_uv.w );
	for ( int j = 0; j < 3; j++ ) {
		drawPts[ j + 3 ].x *= shiftScale.x;
		drawPts[ j + 3 ].y *= shiftScale.y;
	}
	renderSystem->DrawStretchTri( drawPts[0], drawPts[1], drawPts[2], drawPts[3], drawPts[4], drawPts[5], material );

	// draw [outer1, outer2, mid2]
	drawPts[0].Set( trans.outer1.x, trans.outer1.y );
	drawPts[1].Set( trans.outer2.x, trans.outer2.y );
	drawPts[2].Set( mid2.x, mid2.y );
	drawPts[3].Set( trans.outer1.z, trans.outer1.w );
	drawPts[4].Set( trans.outer2.z, trans.outer2.w );
	drawPts[5].Set( mid2_uv.z, mid2_uv.w );
	for ( int j = 0; j < 3; j++ ) {
		drawPts[ j + 3 ].x *= shiftScale.x;
		drawPts[ j + 3 ].y *= shiftScale.y;
	}
	renderSystem->DrawStretchTri( drawPts[0], drawPts[1], drawPts[2], drawPts[3], drawPts[4], drawPts[5], material );

	// draw [mid1, mid2, center]
	drawPts[0].Set( mid1.x, mid1.y );
	drawPts[1].Set( mid2.x, mid2.y );
	drawPts[2].Set( trans.center.x, trans.center.y );
	drawPts[3].Set( mid1_uv.z, mid1_uv.w );
	drawPts[4].Set( mid2_uv.z, mid2_uv.w );
	drawPts[5].Set( trans.center.z, trans.center.w );
	for ( int j = 0; j < 3; j++ ) {
		drawPts[ j + 3 ].x *= shiftScale.x;
		drawPts[ j + 3 ].y *= shiftScale.y;
	}
	renderSystem->DrawStretchTri( drawPts[0], drawPts[1], drawPts[2], drawPts[3], drawPts[4], drawPts[5], material );
}

/*
==================
FullscreenFX_Warp::HighQuality
==================
*/
void FullscreenFX_Warp::HighQuality() {
	float x1, y1, x2, y2, radius, interp;
	idVec2 center;
	int STEP = 9;

	interp = ( idMath::Sin( (float)( gameLocal.slow.time - startWarpTime ) / 1000 ) + 1 ) / 2.0f;
	interp = 0.7f * ( 1 - interp ) + 0.3f * ( interp );

	// draw the warps
	center.x = 320.0f;
	center.y = 240.0f;
	radius = 200.0f;

	for ( float i = 0; i < 360; i += STEP ) {
		// compute the values
		x1 = idMath::Sin( DEG2RAD( i ) );
		y1 = idMath::Cos( DEG2RAD( i ) );

		x2 = idMath::Sin( DEG2RAD( i + STEP ) );
		y2 = idMath::Cos( DEG2RAD( i + STEP ) );

		// add warp polygon
		WarpPolygon_t p;

		p.outer1.x = center.x + x1 * radius;
		p.outer1.y = center.y + y1 * radius;
		p.outer1.z = p.outer1.x / SCREEN_WIDTH;
		p.outer1.w = 1 - ( p.outer1.y / SCREEN_HEIGHT );

		p.outer2.x = center.x + x2 * radius;
		p.outer2.y = center.y + y2 * radius;
		p.outer2.z = p.outer2.x / SCREEN_WIDTH;
		p.outer2.w = 1 - ( p.outer2.y / SCREEN_HEIGHT );

		p.center.x = center.x;
		p.center.y = center.y;
		p.center.z = p.center.x / SCREEN_WIDTH;
		p.center.w = 1 - ( p.center.y / SCREEN_HEIGHT );

		// draw it
		DrawWarp( p, interp );
	}
}





/*
==================
FullscreenFX_EnviroSuit::Initialize
==================
*/
void FullscreenFX_EnviroSuit::Initialize() {
	material = declManager->FindMaterial( "textures/smf/enviro_suit" );
}

/*
==================
FullscreenFX_EnviroSuit::Active
==================
*/
bool FullscreenFX_EnviroSuit::Active() {
	idPlayer *player = fxman->GetPlayer();

	if ( player->PowerUpActive( ENVIROSUIT ) )
		return true;

	return false;
}

/*
==================
FullscreenFX_EnviroSuit::HighQuality
==================
*/
void FullscreenFX_EnviroSuit::HighQuality() {
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, material );
}

/*
==================
FullscreenFX_DoubleVision::Initialize
==================
*/
void FullscreenFX_DoubleVision::Initialize() {
	material = declManager->FindMaterial( "textures/smf/doubleVision" );
}

/*
==================
FullscreenFX_DoubleVision::Active
==================
*/
bool FullscreenFX_DoubleVision::Active() {
	if ( gameLocal.fast.time < fxman->GetPlayerView()->dvFinishTime )
		return true;

	return false;
}

/*
==================
FullscreenFX_DoubleVision::HighQuality
==================
*/
void FullscreenFX_DoubleVision::HighQuality() {
	int offset = fxman->GetPlayerView()->dvFinishTime - gameLocal.fast.time;
	float scale = offset * g_dvAmplitude.GetFloat();
	idPlayer *player;
	idVec2 shiftScale;

	// for testing purposes
	if ( !Active() ) {
		static int test = 0;
		if ( test > 312 )
			test = 0;

		offset = test++;
		scale = offset * g_dvAmplitude.GetFloat();
	}

	player = fxman->GetPlayer();
	shiftScale = fxman->GetShiftScale();

	offset *= 2;		// crutch up for higher res

	// set the scale and shift
	if ( scale > 0.5f ) {
		scale = 0.5f;
	}
	float shift = scale * sin( sqrtf( (float)offset ) * g_dvFrequency.GetFloat() );
	shift = fabs( shift );

	// carry red tint if in berserk mode
	idVec4 color(1, 1, 1, 1);
	if ( gameLocal.fast.time < player->inventory.powerupEndTime[ BERSERK ] ) {
		color.y = 0;
		color.z = 0;
	}

	if ( !gameLocal.isMultiplayer && (gameLocal.fast.time < player->inventory.powerupEndTime[ HELLTIME ] || gameLocal.fast.time < player->inventory.powerupEndTime[ INVULNERABILITY ])) {
		color.y = 0;
		color.z = 0;
	}

	renderSystem->SetColor4( color.x, color.y, color.z, 1.0f );
	renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, shift, shiftScale.y, shiftScale.x, 0, material );
	renderSystem->SetColor4( color.x, color.y, color.z, 0.5f );
	renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, shiftScale.y, (1-shift) * shiftScale.x, 0, material );
}




/*
==================
FullscreenFX_InfluenceVision::Initialize
==================
*/
void FullscreenFX_InfluenceVision::Initialize() {

}

/*
==================
FullscreenFX_InfluenceVision::Active
==================
*/
bool FullscreenFX_InfluenceVision::Active() {
	idPlayer *player = fxman->GetPlayer();

	if ( player->GetInfluenceMaterial() || player->GetInfluenceEntity() )
		return true;

	return false;
}

/*
==================
FullscreenFX_InfluenceVision::HighQuality
==================
*/
void FullscreenFX_InfluenceVision::HighQuality() {
	float distance = 0.0f;
	float pct = 1.0f;
	idPlayer *player;
	idVec2 shiftScale;

	shiftScale = fxman->GetShiftScale();
	player = fxman->GetPlayer();

	if ( player->GetInfluenceEntity() ) {
		distance = ( player->GetInfluenceEntity()->GetPhysics()->GetOrigin() - player->GetPhysics()->GetOrigin() ).Length();
		if ( player->GetInfluenceRadius() != 0.0f && distance < player->GetInfluenceRadius() ) {
			pct = distance / player->GetInfluenceRadius();
			pct = 1.0f - idMath::ClampFloat( 0.0f, 1.0f, pct );
		}
	}

	if ( player->GetInfluenceMaterial() ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, pct );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, player->GetInfluenceMaterial() );
	} else if ( player->GetInfluenceEntity() == NULL ) {
		return;
	} else {
//		int offset =  25 + sinf( gameLocal.slow.time );
//		DoubleVision( hud, view, pct * offset );
	}
}

/*
==================
FullscreenFX_Bloom::Initialize
==================
*/
void FullscreenFX_Bloom::Initialize() {
	drawMaterial		= declManager->FindMaterial( "textures/smf/bloom2/draw" );
	initMaterial		= declManager->FindMaterial( "textures/smf/bloom2/init" );
	currentMaterial		= declManager->FindMaterial( "textures/smf/bloom2/currentMaterial" );

	currentIntensity	= 0.0f;
	targetIntensity		= 0.0f;
}

/*
==================
FullscreenFX_Bloom::Active
==================
*/
bool FullscreenFX_Bloom::Active() {
	idPlayer *player;

	player = fxman->GetPlayer();

	if ( player && player->bloomEnabled )
		return true;

	return false;
}

/*
==================
FullscreenFX_Bloom::HighQuality
==================
*/
void FullscreenFX_Bloom::HighQuality() {
	float shift, delta;
	idVec2 shiftScale;
	idPlayer *player;
	int num;

	shift = 1.0f;
	player = fxman->GetPlayer();
	shiftScale = fxman->GetShiftScale();
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );

	// if intensity value is different, start the blend
	targetIntensity = g_testBloomIntensity.GetFloat();

	if ( player && player->bloomEnabled )
		targetIntensity = player->bloomIntensity;

	delta = targetIntensity - currentIntensity;
	float step = 0.001f;

	if ( step < fabs( delta ) ) {
		if ( delta < 0.0f )
			step = -step;

		currentIntensity += step;
	}

	// draw the blends
	num = g_testBloomNumPasses.GetInteger();

	for ( int i = 0; i < num; i++ ) {
		float s1 = 0.0f, t1 = 0.0f, s2 = 1.0f, t2 = 1.0f;
		float alpha;

		// do the center scale
		s1 -= 0.5f;
		s1 *= shift;
		s1 += 0.5f;
		s1 *= shiftScale.x;

		t1 -= 0.5f;
		t1 *= shift;
		t1 += 0.5f;
		t1 *= shiftScale.y;

		s2 -= 0.5f;
		s2 *= shift;
		s2 += 0.5f;
		s2 *= shiftScale.x;

		t2 -= 0.5f;
		t2 *= shift;
		t2 += 0.5f;
		t2 *= shiftScale.y;

		// draw it
		if ( num == 1.0f )
			alpha = 1.0f;
		else
			alpha = 1.0f - (float)i / ( num - 1.0f );

		renderSystem->SetColor4( alpha, alpha, alpha, 1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, s1, t2, s2, t1, drawMaterial );

		shift += currentIntensity;
	}
}

/*
==================
FullscreenFX_Bloom::Save
==================
*/
void FullscreenFX_Bloom::Save( idSaveGame *savefile ) {
	FullscreenFX::Save( savefile );
	savefile->WriteFloat( currentIntensity );
	savefile->WriteFloat( targetIntensity );
}

/*
==================
FullscreenFX_Bloom::Restore
==================
*/
void FullscreenFX_Bloom::Restore( idRestoreGame *savefile ) {
	FullscreenFX::Restore( savefile );
	savefile->ReadFloat( currentIntensity );
	savefile->ReadFloat( targetIntensity );
}

/*
==================
FullscreenFXManager::FullscreenFXManager
==================
*/
FullscreenFXManager::FullscreenFXManager() {
	highQualityMode = false;
	playerView = NULL;
	blendBackMaterial = NULL;
	shiftScale.Set( 0.0f, 0.0f );
}

/*
==================
FullscreenFXManager::~FullscreenFXManager
==================
*/
FullscreenFXManager::~FullscreenFXManager() {
}

/*
==================
FullscreenFXManager::FindFX
==================
*/
FullscreenFX* FullscreenFXManager::FindFX( idStr name ) {
	for ( int i = 0; i < fx.Num(); i++ )
		if ( fx[i]->GetName() == name )
			return fx[i];

	return NULL;
}

/*
==================
FullscreenFXManager::CreateFX
==================
*/
void FullscreenFXManager::CreateFX( idStr name, idStr fxtype, int fade ) {
	FullscreenFX *pfx = NULL;

	if ( fxtype == "helltime" )
		pfx = new FullscreenFX_Helltime;
	else if ( fxtype == "warp" )
		pfx = new FullscreenFX_Warp;
	else if ( fxtype == "envirosuit" )
		pfx = new FullscreenFX_EnviroSuit;
	else if ( fxtype == "doublevision" )
		pfx = new FullscreenFX_DoubleVision;
	else if ( fxtype == "multiplayer" )
		pfx = new FullscreenFX_Multiplayer;
	else if ( fxtype == "influencevision" )
		pfx = new FullscreenFX_InfluenceVision;
	else if ( fxtype == "bloom" )
		pfx = new FullscreenFX_Bloom;
	else
		assert( 0 );

	if ( pfx ) {
		pfx->Initialize();
		pfx->SetFXManager( this );
		pfx->SetName( name );
		pfx->SetFadeSpeed( fade );
		fx.Append( pfx );
	}
}

/*
==================
FullscreenFXManager::Initialize
==================
*/
void FullscreenFXManager::Initialize( idPlayerView *pv ) {
	// set the playerview
	playerView = pv;
	blendBackMaterial = declManager->FindMaterial( "textures/smf/blendBack" );

	// allocate the fx
	CreateFX( "helltime", "helltime", 1000 );
	CreateFX( "warp", "warp", 0 );
	CreateFX( "envirosuit", "envirosuit", 500 );
	CreateFX( "doublevision", "doublevision", 0 );
	CreateFX( "multiplayer", "multiplayer", 1000 );
	CreateFX( "influencevision", "influencevision", 1000 );
	CreateFX( "bloom", "bloom", 0 );

	// pre-cache the texture grab so we dont hitch
	renderSystem->CropRenderSize( 512, 512, true );
	renderSystem->CaptureRenderToImage( "_accum" );
	renderSystem->UnCrop();

	renderSystem->CropRenderSize( 512, 256, true );
	renderSystem->CaptureRenderToImage( "_scratch" );
	renderSystem->UnCrop();

	renderSystem->CaptureRenderToImage( "_currentRender" );
}

/*
==================
FullscreenFXManager::Blendback
==================
*/
void FullscreenFXManager::Blendback( float alpha ) {
	// alpha fade
	if ( alpha < 1.0f ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f - alpha );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, blendBackMaterial );
	}
}

/*
==================
FullscreenFXManager::Save
==================
*/
void FullscreenFXManager::Save( idSaveGame *savefile ) {
	savefile->WriteBool( highQualityMode );
	savefile->WriteVec2( shiftScale );

	for ( int i = 0; i < fx.Num(); i++ ) {
		FullscreenFX *pfx = fx[i];
		pfx->Save( savefile );
	}
}

/*
==================
FullscreenFXManager::Restore
==================
*/
void FullscreenFXManager::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( highQualityMode );
	savefile->ReadVec2( shiftScale );

	for ( int i = 0; i < fx.Num(); i++ ) {
		FullscreenFX *pfx = fx[i];
		pfx->Restore( savefile );
	}
}

/*
==================
FullscreenFXManager::CaptureCurrentRender
==================
*/
void FullscreenFXManager::CaptureCurrentRender() {
	renderSystem->CaptureRenderToImage( "_currentRender" );
}

/*
==================
FullscreenFXManager::Process
==================
*/
void FullscreenFXManager::Process( const renderView_t *view ) {
	bool allpass = false;

	if ( g_testFullscreenFX.GetInteger() == -2 ) {
		allpass = true;
	}

	if ( g_lowresFullscreenFX.GetBool() ) {
		highQualityMode = false;
	}
	else {
		highQualityMode = true;
	}

	// compute the shift scale
	if ( highQualityMode ) {
		int vidWidth, vidHeight;
		renderSystem->GetGLSettings( vidWidth, vidHeight );

		float pot;
		int	 w = vidWidth;
		pot = MakePowerOfTwo( w );
		shiftScale.x = (float)w / pot;

		int	 h = vidHeight;
		pot = MakePowerOfTwo( h );
		shiftScale.y = (float)h / pot;
	}
	else {
		// if we're in low-res mode, shrink view down
		shiftScale.x = 1;
		shiftScale.y = 1;
		renderSystem->CropRenderSize( 512, 512, true );
	}

	// do the first render
	gameRenderWorld->RenderScene( view );

	// do the process
	for ( int i = 0; i < fx.Num(); i++ ) {
		FullscreenFX *pfx = fx[i];
		bool drawIt = false;

		// determine if we need to draw
		if ( pfx->Active() || g_testFullscreenFX.GetInteger() == i || allpass ) {
			drawIt = pfx->SetTriggerState( true );
		}
		else {
			drawIt = pfx->SetTriggerState( false );
		}

		// do the actual drawing
		if ( drawIt ) {
			// we need to dump to _currentRender
			CaptureCurrentRender();

			// handle the accum pass if we have one
			if ( pfx->HasAccum() ) {

				// if we're in high quality mode, we need to crop the accum pass
				if ( highQualityMode ) {
					renderSystem->CropRenderSize( 512, 512, true );
					pfx->AccumPass( view );
					renderSystem->UnCrop();
				}
				else {
					pfx->AccumPass( view );
				}
			}

			// do the high quality pass
			pfx->HighQuality();

			// do the blendback
			Blendback( pfx->GetFadeAlpha() );
		}
	}

	if ( !highQualityMode ) {
		// we need to dump to _currentRender
		CaptureCurrentRender();

		// uncrop view
		renderSystem->UnCrop();

		// draw the final full-screen image
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, blendBackMaterial );
	}
}
#endif

// sikk---> PostProcess Effects
/*
===================
idPlayerView::DoPostFX
===================
*/
void idPlayerView::DoPostFX() {
	// screen space reflection stuff
	//renderSystem->CaptureRenderToImage( "_currentRender" );
	//renderSystem->CropRenderSize( 256, 128, true, true );
	//renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 0.0f );
	//renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, declManager->FindMaterial( "postProcess/ssReflection", false ) );
	//renderSystem->CaptureRenderToImage( "_ssReflect" );
	//renderSystem->UnCrop();
	//renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	
	bDepthRendered = false;

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

	if ( g_useExplosionFX.GetBool() )
		PostFX_ExplosionFX();

	if ( r_useDepthOfField.GetBool() )
		PostFX_DoF();

	if ( r_useMotionBlur.GetBool() )
		PostFX_MotionBlur();

	if ( player->PowerUpActive( ADRENALINE ) )
		PostFX_AdrenalineVision();

	if ( r_useColorGrading.GetBool() )
		PostFX_ColorGrading();

	if ( g_screenFrostTime.GetInteger() )
		PostFX_ScreenFrost();

	PostFX_ScreenBlobs();

	PostFX_IRGoggles();

	PostFX_ArmorPulse();

	if ( bfgVision )
		PostFX_BFGVision();

	if ( !gameLocal.inCinematic )
		PostFX_TunnelVision();
			
	if ( r_useVignetting.GetBool() && !r_useHDR.GetBool() )	// HDR uses it's own vignette solution
		PostFX_Vignetting();

	if ( r_useFilmgrain.GetBool() )
		PostFX_Filmgrain();

/*
	if ( player->GetInfluenceMaterial() || player->GetInfluenceEntity() )
		PostFX_InfluenceVision();

	if ( g_doubleVision.GetBool() && gameLocal.time < dvFinishTime )
		PostFX_DoubleVision();

	if ( player->PowerUpActive( BERSERK ) )
		PostFX_BerserkVision();
*/
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

	if ( bCrop && !bDepthRendered ) {
		int	nWidth = renderSystem->GetScreenWidth() / 2;
		int	nHeight = renderSystem->GetScreenHeight() / 2;

		renderSystem->CropRenderSize( nWidth, nHeight, true );

//		if ( r_useSoftShadows.GetBool() ) {
//			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, declManager->FindMaterial( "crop/depth", false ) );
//			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0, 1.0, 0.0f, declManager->FindMaterial( "crop/depth", false ) );
//			renderSystem->CaptureRenderToImage( "_depth" );
//		} else {
			// set our depthView parms
			renderView_t depthView = hackedView;
			depthView.viewID = -8;
//			depthView.globalMaterial = depthMaterial;
			cvarSystem->SetCVarString( "r_materialOverride", "render/depth" );
			// render scene
			gameRenderWorld->RenderScene( &depthView );
			// capture image for our depth buffer
			renderSystem->CaptureRenderToImage( "_depth" );
			cvarSystem->SetCVarString( "r_materialOverride", "" );
//		}
		renderSystem->UnCrop();
		bDepthRendered = true;
	} else if ( !bCrop ) {	// uncropped depth is used specifically for soft shadows
		// set our depthView parms
		renderView_t depthView = hackedView;
		depthView.viewID = -8;
//		depthView.globalMaterial = depthMaterial;
		cvarSystem->SetCVarString( "r_materialOverride", "render/depth" );
		// render scene
		gameRenderWorld->RenderScene( &depthView );
		// capture image for our depth buffer
		renderSystem->CaptureRenderToImage( "_ssDepth" );
		cvarSystem->SetCVarString( "r_materialOverride", "" );
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
	if ( player->GetCurrentWeaponNum() == 0 ) {
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
	if ( player->GetCurrentWeaponNum() == 0 ) {
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
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_hdrLum" );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrLumBaseMaterial );
	renderSystem->CaptureRenderToImage( "_hdrLum" );
	renderSystem->CaptureRenderToImage( "_hdrLumAvg" );
	renderSystem->UnCrop();

	// create average scene luminance map by using a 4x4 downsampling chain and box-filtering
	// Output will be a 1x1 pixel of the average luminance
	for ( int i = 256; i > 1; i *= 0.5 ) {
		renderSystem->CropRenderSize( i, i, true, true );
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
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
		renderSystem->CaptureRenderToImage( "_hdrBloom" );
		renderSystem->SetColor4( r_hdrBloomMiddleGray.GetFloat(), r_hdrBloomWhitePoint.GetFloat(), r_hdrBloomThreshold.GetFloat(), r_hdrBloomOffset.GetFloat() );
		if ( r_hdrBloomToneMapper.GetInteger() == 0 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBrightPass1Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 1 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBrightPass2Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 2 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBrightPass3Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 3 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBrightPass4Material );
		else if	 ( r_hdrBloomToneMapper.GetInteger() == 4 )
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrBrightPass5Material );
		renderSystem->CaptureRenderToImage( "_hdrBloom" );
		renderSystem->CaptureRenderToImage( "_hdrFlare" );
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

		// create lens flare texture
		if ( r_hdrFlareScale.GetFloat() ) {
			renderSystem->CropRenderSize( nGlareWidth, nGlareHeight, true, true );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f, 0.0f, 0.0f, 1.0f, declManager->FindMaterial( "_hdrFlare" ) );
			renderSystem->CaptureRenderToImage( "_hdrFlare" );
			renderSystem->SetColor4( r_hdrFlareGamma.GetFloat(), 1.0f, 1.0f, 0.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFlareMaterial );
			renderSystem->CaptureRenderToImage( "_hdrFlare" );
			renderSystem->SetColor4( r_hdrFlareSize.GetFloat(), 1.0f, 1.0f, 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFlareMaterial );
			renderSystem->CaptureRenderToImage( "_hdrFlare" );
			renderSystem->SetColor4( DEG2RAD( r_hdrFlareSize.GetFloat() ), 1.0f, 1.0f, 2.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFlareMaterial );
			renderSystem->CaptureRenderToImage( "_hdrFlare" );
			renderSystem->SetColor4( DEG2RAD( r_hdrFlareSize.GetFloat() ), r_hdrFlareScale.GetFloat(), 1.0f, 2.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFlareMaterial );
			renderSystem->CaptureRenderToImage( "_hdrFlare" );
			renderSystem->UnCrop();
		}
	}

	// create glare textures
	if ( r_hdrGlareStyle.GetInteger() == 0 ) {
		// bloom off (clear textures)
		renderSystem->CropRenderSize( 1, 1, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, blackMaterial );
		renderSystem->CaptureRenderToImage( "_hdrBloom" );
		renderSystem->CaptureRenderToImage( "_hdrFlare" );
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

	if ( r_hdrDither.GetBool() && ( !bDitherRendered || ( fDitherSize != r_hdrDitherSize.GetFloat() ) ) ) {
		float size = 16.0f * r_hdrDitherSize.GetFloat();
		renderSystem->SetColor4( renderSystem->GetScreenWidth() / size, renderSystem->GetScreenHeight() / size, 1.0f, -1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFinalMaterial );
		renderSystem->CaptureRenderToImage( "_hdrDither" );
		bDitherRendered = true;
	} else if ( !r_hdrDither.GetBool() && bDitherRendered ) {
		renderSystem->CropRenderSize( 1, 1, true, true );
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, -2.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, hdrFinalMaterial );
		renderSystem->CaptureRenderToImage( "_hdrDither" );
		renderSystem->UnCrop();
		bDitherRendered = false;
	}
	fDitherSize = r_hdrDitherSize.GetFloat();

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
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_bloom" );
	renderSystem->SetColor4( r_bloomGamma.GetFloat(), 1.0f, 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, bloomMaterial );
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

	RenderDepth( true );
	RenderNormals( false );

	renderSystem->CropRenderSize( nWidth, nHeight, true, true );
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
	if ( r_useDepthOfField.GetInteger() == 1 && !gameLocal.inCinematic ) {
		trace_t trace;
		idVec3 start = hackedView.vieworg;
		idVec3 end = start + hackedView.viewaxis.ToAngles().ToForward() * 8192.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, player );
		focusDistance = focusDistance * 0.9 + trace.fraction * 0.1;
	}

	if ( DoFConditionCheck() ) {
		int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
		int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;
		renderSystem->CaptureRenderToImage( "_currentRender" );

		RenderDepth( true );

		renderSystem->CropRenderSize( nWidth, nHeight, true, true );
		if ( r_useDepthOfField.GetInteger() == 2 )
			renderSystem->SetColor4( r_dofNear.GetInteger(), r_dofFocus.GetInteger(), r_dofFar.GetInteger(), 2.0f );
		else if ( gameLocal.inCinematic )
			renderSystem->SetColor4( r_dofNear.GetInteger(), r_dofFocus.GetInteger(), r_dofFar.GetInteger(), 2.0f );	// don't blur in front of the focal plane for cinematics
		else if ( player->weapon.GetEntity()->IsReloading() )
			renderSystem->SetColor4( -1.0f, 0.5f, 64.0f, 2.0f );	// use specific settings for reloading dof
		else if ( player->bIsZoomed )
			renderSystem->SetColor4( focusDistance, 1.0f, 1.0f, 1.0f );	// zoom uses a mask texture
		else
			renderSystem->SetColor4( focusDistance, 1.0f, 1.0f, 0.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, dofMaterial );
		renderSystem->CaptureRenderToImage( "_dof" );
		renderSystem->UnCrop();

		// blur scene using our depth of field mask
		renderSystem->SetColor4( r_dofBlurScale.GetFloat(), r_dofBlurQuality.GetInteger(), 1.0f, 3.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, dofMaterial );
		if ( r_dofBlurQuality.GetInteger() == 2 ) {
			renderSystem->CaptureRenderToImage( "_currentRender" );
			renderSystem->SetColor4( r_dofBlurScale.GetFloat(), r_dofBlurQuality.GetInteger() + 2.0f, 1.0f, 3.0f );
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
	float fFPS = idMath::ClampFloat( 0.0f, 1.0f, 1.0f / ( (float)( gameLocal.time - prevTime ) * 0.06f ) );

	if ( r_useMotionBlur.GetInteger() > 1 ) {
		renderSystem->CaptureRenderToImage( "_currentRender" );
		renderSystem->SetColor4( r_motionBlurLerp.GetFloat() * fFPS, 1.0f, 1.0f, 0.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
		renderSystem->CaptureRenderToImage( "_prevRender" );
	}

	if ( MBConditionCheck() && ( r_useMotionBlur.GetInteger() == 1 || r_useMotionBlur.GetInteger() == 3 ) ) {
		int	nWidth		= renderSystem->GetScreenWidth() / 2.0f;
		int	nHeight		= renderSystem->GetScreenHeight() / 2.0f;
		int nQuality	= idMath::ClampInt( 1, 4, r_motionBlurQuality.GetInteger() );

		float parm[6];
		parm[0] = player->viewAngles.yaw - prevViewAngles.yaw;
		parm[1] = player->viewAngles.pitch - prevViewAngles.pitch;
		if ( parm[0] > 180.0f ) {
			parm[0] -= 360.0f;
			prevViewAngles.yaw += 360.0f;
		}
		if ( parm[0] < -180.0f ) {
			parm[0] += 360.0f;
			prevViewAngles.yaw -= 360.0f;
		}
		parm[0] = idMath::ClampFloat( -r_motionBlurMaxThreshold.GetInteger(), r_motionBlurMaxThreshold.GetInteger(), parm[0] );
		parm[1] = idMath::ClampFloat( -r_motionBlurMaxThreshold.GetInteger(), r_motionBlurMaxThreshold.GetInteger(), parm[1] );
		float f = idMath::Fabs( player->viewAngles.pitch ) / 90.0f * 0.5f;
		parm[2] = player->viewAngles.pitch < 0.0f ? 1.0f - f : f;
		parm[3] = idMath::Fabs( parm[0] ) > idMath::Fabs( parm[1] ) ? idMath::Fabs( player->viewAngles.pitch ) / 90.0f : 0.0f;
		parm[4] = r_motionBlurFactor.GetFloat();

		parm[0] *= r_motionBlurScale.GetFloat() * fFPS;
		parm[1] *= r_motionBlurScale.GetFloat() * fFPS;

		renderSystem->CaptureRenderToImage( "_currentRender" );

		RenderDepth( true );

		renderSystem->CropRenderSize( nWidth, nHeight, true, true );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
		renderSystem->CaptureRenderToImage( "_mbXY" );
		renderSystem->CaptureRenderToImage( "_mbZ" );
		for ( int i = 0; i < nQuality; i++ ) {
			renderSystem->SetColor4( parm[0], parm[1], r_motionBlurMaskDistance.GetFloat(), 1.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
			renderSystem->CaptureRenderToImage( "_mbXY" );
			renderSystem->SetColor4( parm[0], parm[2], r_motionBlurMaskDistance.GetFloat(), 2.0f );
			renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
			renderSystem->CaptureRenderToImage( "_mbZ" );
		}
		renderSystem->UnCrop();
		
		renderSystem->SetColor4( parm[3], parm[4], r_motionBlurMaskDistance.GetFloat(), 3.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, motionBlurMaterial );
	}

	float fLerp = 0.5f;//idMath::ClampFloat( 0.0f, 0.99f, r_motionBlurLerp.GetFloat() );
	prevViewAngles = prevViewAngles * fLerp + player->viewAngles * ( 1.0 - fLerp );
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

	if ( ( player->viewAngles.pitch >= prevViewAngles.pitch + nThreshold ) ||
		 ( player->viewAngles.pitch <= prevViewAngles.pitch - nThreshold ) ||
		 ( player->viewAngles.yaw >= prevViewAngles.yaw + nThreshold ) ||
		 ( player->viewAngles.yaw <= prevViewAngles.yaw - nThreshold ) )
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
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_blurRender" );
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, colorGradingMaterial );
	renderSystem->CaptureRenderToImage( "_blurRender" );
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, colorGradingMaterial );
	renderSystem->CaptureRenderToImage( "_blurRender" );
	renderSystem->UnCrop();

	renderSystem->SetColor4( r_colorGradingParm.GetInteger(), r_colorGradingSharpness.GetFloat(), 1.0f, ( r_colorGradingType.GetFloat() + 2.0f ) );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, colorGradingMaterial );
}

/*
===================
idPlayerView::PostFX_ExplosionFX
===================
*/
void idPlayerView::PostFX_ExplosionFX() {
	if ( gameLocal.explosionTime >= gameLocal.time ) {
		idMat3 axis;
		idVec3 origin;
		idVec3 viewVector[3];
		player->GetViewPos( origin, axis );
		player->viewAngles.ToVectors( &viewVector[0], &viewVector[1], &viewVector[2] );

		idVec3 expOrigin	= gameLocal.explosionOrigin;
		idVec3 dist			= origin - expOrigin;
		float length		= dist.Length();
		idVec3 expVector	= dist / length;

		float VdotE = idMath::ClampFloat( 0.0f, 1.0f, viewVector[ 0 ] * -expVector );

		idVec3 ndc;
		renderSystem->GlobalToNormalizedDeviceCoordinates( gameLocal.explosionOrigin, ndc );

		renderSystem->CaptureRenderToImage( "_currentRender" );
		ndc.x = ndc.x * 0.5 + 0.5;
		ndc.y = ndc.y * 0.5 + 0.5;

		float time = (float)gameLocal.explosionTime - (float)gameLocal.time;
		float radius = idMath::ClampFloat( 0.0f, 1.0f, gameLocal.explosionRadius / 200.0f );
		float damage = idMath::ClampFloat( 0.0f, 1.0f, gameLocal.explosionDamage / 250.0f );
		float distance	= 1.0f - idMath::ClampFloat( 0.0f, 1.0f, length / 1024.0f );
		float atten	= idMath::ClampFloat( 0.0f, 1.0f, time / ( g_explosionFXTime.GetFloat() * 1000 ) );
		float scale = radius * damage * distance * atten * VdotE * g_explosionFXScale.GetFloat();

		renderSystem->SetColor4( ndc.x, ndc.y, scale, 1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, explosionFXMaterial );
	}
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
idPlayerView::PostFX_ScreenFrost
===================
*/
void idPlayerView::PostFX_ScreenFrost() {
	float alpha = player->nScreenFrostAlpha / ( g_screenFrostTime.GetFloat() * 60.0f );
	if ( alpha ) {
		renderSystem->CaptureRenderToImage( "_currentRender" );
		renderSystem->SetColor4( alpha, 1.0f, 1.0f, 1.0f );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, screenFrostMaterial );
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
idPlayerView::PostFX_AdrenalineVision
===================
*/
void idPlayerView::PostFX_AdrenalineVision() {
	int	nWidth	= renderSystem->GetScreenWidth() / 2.0f;
	int	nHeight	= renderSystem->GetScreenHeight() / 2.0f;

	renderSystem->CaptureRenderToImage( "_currentRender" );

	// unsharp mask buffer
	renderSystem->CropRenderSize( nWidth, nHeight, true, true );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, shiftScale.y, shiftScale.x, 0.0f, currentRenderMaterial );
	renderSystem->CaptureRenderToImage( "_blurRender" );
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 0.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, adrenalineMaterial );
	renderSystem->CaptureRenderToImage( "_blurRender" );
	renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, adrenalineMaterial );
	renderSystem->CaptureRenderToImage( "_blurRender" );
	renderSystem->UnCrop();

	int adrenalTime = player->inventory.powerupEndTime[ ADRENALINE ] - gameLocal.time;
	float alpha = ( adrenalTime < 10000 ) ? (float)adrenalTime / 10000 : 1.0f;
	alpha = ( alpha < 0.0f ) ? 0.0f : alpha;

	renderSystem->SetColor4( alpha, ( alpha + 1.0f ), 1.0f, 2.0f );
	renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, 1.0f, 0.0f, adrenalineMaterial );
}

/*
===================
idPlayerView::PostFX_DoubleVision
===================
*//*
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
}*/

/*
===================
idPlayerView::PostFX_BerserkVision
===================
*//*
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
}*/

/*
===================
idPlayerView::PostFX_InfluenceVision
===================
*//*
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
}*/

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

// sikk---> Health Management System (Health Regen)
	if ( g_healthManagementType.GetInteger() == 2 )
		alpha = idMath::ClampFloat( 0.0f, 1.0f, health / g_healthRegenFeedback.GetFloat() );
	else
		alpha = idMath::ClampFloat( 0.0f, 1.0f, health / 100.0f );
// <---sikk

	if ( alpha < 1.0f  ) {
// sikk---> Health Management System (Health Regen) ( also fix for "g_testHealthVision" )
		if ( g_healthManagementType.GetInteger() == 2 || g_testHealthVision.GetFloat() )
			renderSystem->SetColor4( MS2SEC( gameLocal.time ), 1.0f, 1.0f, alpha );
		else
			renderSystem->SetColor4( ( player->health <= 0.0f ) ? MS2SEC( gameLocal.time ) : lastDamageTime, 1.0f, 1.0f, ( player->health <= 0.0f ) ? 0.0f : alpha );
		if ( g_healthManagementType.GetInteger() == 2 )
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

		if ( blob->finishTime <= gameLocal.slow.time )
			continue;

		blob->y += blob->driftAmount;

		float fade = (float)( blob->finishTime - gameLocal.slow.time ) / ( blob->finishTime - blob->startFadeTime );

		if ( fade > 1.0f )
			fade = 1.0f;

		if ( fade ) {
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
	float armorPulse = ( gameLocal.fast.time - player->lastArmorPulse ) / 250.0f;

	if ( armorPulse > 0.0f && armorPulse < 1.0f ) {
		renderSystem->SetColor4( 1.0f, 1.0f, 1.0f, 1.0f - armorPulse );
		renderSystem->DrawStretchPic( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, armorMaterial );
	}
}
// <---sikk
