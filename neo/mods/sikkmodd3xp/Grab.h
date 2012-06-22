// sikk---> Object Manipulation
/*
===============================================================================

	Grab Object - Class to extend idWeapon to include functionality for 
						manipulating physics objects.

===============================================================================
*/

#include "physics/Force_Grab.h"

class idGrabEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idGrabEntity );

							idGrabEntity( void );
							~idGrabEntity( void );

	void					Clear();

	void					Update( idPlayer *player );
	void					StartDrag( idPlayer *player, idEntity *grabEnt, int id );
	void					StopDrag( idPlayer *player, bool drop );

	idEntity *				GetGrabEntity( void ) const { return dragEnt.GetEntity(); }
	int						GetThrownTime( void ) const { return lastThrownTime; }
	void					SetThrownTime( int time ) { lastThrownTime = time; }

private:
	idEntityPtr<idEntity>	dragEnt;			// entity being dragged
	idForce_Grab			drag;
	idPlayer*				owner;

	idVec3					saveGravity;
	idVec3					localPlayerPoint;	// dragged point in player space
	idAngles				prevViewAngles;		// Holds previous frame's player view angles

	int						id;					// id of body being dragged
	int						oldUcmdFlags;
	int						dragFailTime;
	int						lastThrownTime;

};
// <---sikk
