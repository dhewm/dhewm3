//#ifdef _D3XP

/*
===============================================================================

	Grabber Object - Class to extend idWeapon to include functionality for 
						manipulating physics objects.

===============================================================================
*/

#include "physics/Force_Grab.h"

class idBeam;

class idGrabber : public idEntity {
public:
	CLASS_PROTOTYPE( idGrabber );

							idGrabber( void );
							~idGrabber( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Initialize( void );
	void					SetDragDistance( float dist );
	int						Update( idPlayer *player, bool hide );

private:
	idEntityPtr<idEntity>	dragEnt;			// entity being dragged
	idForce_Grab			drag;
	idVec3					saveGravity;

	int						id;					// id of body being dragged
	idVec3					localPlayerPoint;	// dragged point in player space
	idEntityPtr<idPlayer>	owner;
	int						oldUcmdFlags;
	bool					holdingAF;
	bool					shakeForceFlip;
	int						endTime;
	int						lastFiredTime;
	int						dragFailTime;
	int						startDragTime;
	float					dragTraceDist;
	int						savedContents;
	int						savedClipmask;

	idBeam*					beam;
	idBeam*					beamTarget;

//	int						warpId;

	bool					grabbableAI( const char *aiName );
	void					StartDrag( idEntity *grabEnt, int id );
	void					StopDrag( bool dropOnly );
	void					UpdateBeams( void );
	void					ApplyShake( void );
};

//#endif
