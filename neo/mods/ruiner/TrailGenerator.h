// Created by ivan_the_B
//

#ifndef __GAME_TRAILGENERATOR_H__
#define __GAME_TRAILGENERATOR_H__


#define	MAX_TRAILS			32

/*
===============================================================================

idTrailManager

===============================================================================
*/
class idTrailGenerator;

class idTrailManager {

public:
								idTrailManager( void );
								~idTrailManager( void );

	void						Init( void );
	void						Shutdown( void );

	void						Save( idSaveGame *savefile ) const;
	void						Restore( idRestoreGame *savefile );

	void						Think( void );

	idTrailGenerator*			NewTrailGen( void );	
	void						RemoveTrailGen( idTrailGenerator* trailGen );
	idTrailGenerator*			FindTrailByUniqueId( int id );
	idTrailGenerator*			FindTrailByLocalPos( int pos );

	int							GetSafeUniqueId( idTrailGenerator* trailGen );

private:
	friend class				idTrailGenerator; //idTrailGenerator can directly access trails array to save performace 

	//global
	bool						initialized;
	int							lastUniqueId;

	int							minFreePos;
	int							maxAllocPos;
	

	//points list
	idTrailGenerator*			trails[MAX_TRAILS];
	int							uniqueIds[MAX_TRAILS];
};


/*
===============================================================================

idTrailGenerator

===============================================================================
*/

class idTrailGenerator {

public:
								idTrailGenerator( void );
								~idTrailGenerator( void );

	int							GetUniqueId(){ return uniqueId; }
	int							GetLocalPos(){ return localPos; }

	void						Save( idSaveGame *savefile ) const;
	void						Restore( idRestoreGame *savefile );

	void						Fade( void );

	void						AddNewPoints( const idVec3 &newPosL, const idVec3 &newPosH );
	void						RemoveOldestPoints( void );

	void						StartTrail( const char *newTrailDefName );
	void						RestartTrail( void );
	void						FadeTrail( void );
	void						StopTrail( void );

	bool						IsEnabled( void ){ return enabled; };
	bool						IsFading( void ){ return fading; };
	
private:
	friend class				idTrailManager; //idTrailManager is the onyl class that can call "init"


	//global
	int							uniqueId; //used by other entities to find this after reload. Also used by and ModelCallback().
	int							localPos;
	bool						initialized;
	//int							thinkFlags;
	
	// settings
	bool						enabled;
	bool						fading;
	idVec3						fadingColorStep;
	int							fadingTime; //how long fading takes
	int							fadingEndTime; //end fade time
	int							maxPoints;
	const idMaterial *			material;

	//trail def
	const idDict *				trailDef;
	idStr						trailDefName; //this is just to SAVE/RESTORE trailDef

	//points list
	idList<idVec3>				points;

	// rendering
	renderEntity_t				renderEntity;			// used to present a model to the renderer
	int							renderEntityHandle;		// handle to static renderer model
	mutable int					lastRenderEntityUpdate;
	mutable bool				modelChanged;

	void						Init( int uniId, int locId );

	void						FreeEntityDef( void );
	void						UpdateVisuals( void );

	bool						UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) const;
	static bool					ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView );
};

/*
ID_INLINE bool idTrailGenerator::IsFading( void ) {
	return fading;
}
*/

#endif /* !__GAME_TRAILGENERATOR_H__ */
