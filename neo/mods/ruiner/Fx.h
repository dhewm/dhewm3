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

#ifndef __GAME_FX_H__
#define __GAME_FX_H__

#include "renderer/RenderWorld.h"

#include "Entity.h"

/*
===============================================================================

  Special effects.

  Ivan notes: 
  - duration 0  = endless ( it lasts al long as the others are active). 
  - duration -1 = like 0, but disabled when fade starts
  If there are only endless ones, the fx is self-removed instantly unless "manualRemove" is set.

  - delay -1 = activated when fade starts
  If "manualRemove" is not set the fx will self-remove after the max "duration" value.

===============================================================================
*/

typedef struct {
	renderLight_t			renderLight;			// light presented to the renderer
	qhandle_t				lightDefHandle;			// handle to renderer light def
	renderEntity_t			renderEntity;			// used to present a model to the renderer
	int						modelDefHandle;			// handle to static renderer model
	float					delay;
	int						particleSystem;
	int						start;
	bool					soundStarted;
	bool					shakeStarted;
	bool					decalDropped;
	bool					launched;
} idFXLocalAction;

class idEntityFx : public idEntity {
public:
	CLASS_PROTOTYPE( idEntityFx );

							idEntityFx();
	virtual					~idEntityFx();

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Think( void );
	void					Setup( const char *fx );
	void					Run( int time );
	virtual void			Start( int time ); //ivan - virtual added
	void					Stop( void );
	const int				Duration( void );
	const char *			EffectName( void );
	const char *			Joint( void );
	const bool				Done( void );

	//ivan start
	void					FadeOutFx( void ); 
	const int				FadeDuration( void );
	//ivan end


	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );
	virtual void			ClientPredictionThink( void );

	//static idEntityFx *		StartFx( const char *fx, const idVec3 *useOrigin, const idMat3 *useAxis, idEntity *ent, bool bind );
	static idEntityFx *		StartFx( const char *fx, const idVec3 *useOrigin, const idMat3 *useAxis, idEntity *ent, bool bind, bool orientated = true, jointHandle_t jointnum = INVALID_JOINT ); //ivan
	static void				StartFxUtility( idEntityFx *nfx, const idVec3 *useOrigin, const idMat3 *useAxis, idEntity *ent, bool bind, bool orientated = true, jointHandle_t jointnum = INVALID_JOINT ); //ivan

protected:
	void					Event_Trigger( idEntity *activator );
	void					Event_ClearFx( void );

	//ivan start
	void					Event_FadeFx( void ); 
	void					SetupFade( void );
	void					ResetShaderParms( void );
	//ivan end

	void					CleanUp( void );
	void					CleanUpSingleAction( const idFXSingleAction& fxaction, idFXLocalAction& laction );
	void					ApplyFade( const idFXSingleAction& fxaction, idFXLocalAction& laction, const int time, const int actualStart );

	int						started;
	int						nextTriggerTime;
	const idDeclFX *		fxEffect;				// GetFX() should be called before using fxEffect as a pointer
	idList<idFXLocalAction>	actions;
	idStr					systemName;
	//ivan start
	bool					manualRemove; 
	bool					manualFadeIsOn;
	bool					endlessSounds;
	//ivan end
};

class idTeleporter : public idEntityFx {
public:
	CLASS_PROTOTYPE( idTeleporter );

private:
	// teleporters to this location
	void					Event_DoAction( idEntity *activator );
};

//ivan start

// see 'dmgFxTypeString' array 
enum {
	DMGFX_NONE = 0, //no fx
	DMGFX_FLAMES,
	DMGFX_ELECTRO,
	DMGFX_TOXIC,
	NUM_DMGFX_TYPES //always the last
};

class idDamagingFx : public idEntityFx {
public:
	CLASS_PROTOTYPE( idDamagingFx );

							idDamagingFx();
	virtual					~idDamagingFx();

	void					Spawn( void );

	virtual void			Think( void );
	static idDamagingFx *	StartDamagingFx( int type, idEntity *victimEnt );
	virtual void			Start( int time );
	void					Restart( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );
	int						GetDmgFxType( void ){ return dmgfxType; };

private:
	
	idStr					damageDefString;
	idEntityPtr<idEntity>	victim;
	int						nextDamageTime;
	int						damageRate;
	int						endDamageTime;
	int						dmgfxType;
};

//ivan end

#endif /* !__GAME_FX_H__ */
