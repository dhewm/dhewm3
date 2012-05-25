// sikk---> Object Manipulation

#ifndef __FORCE_GRAB_H__
#define __FORCE_GRAB_H__

/*
===============================================================================

	Drag force

===============================================================================
*/

class idForce_Grab : public idForce {

public:
	CLASS_PROTOTYPE( idForce_Grab );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

						idForce_Grab( void );
	virtual				~idForce_Grab( void );
						// initialize the drag force
	void				Init( float damping );
						// set physics object being dragged
	void				SetPhysics( idPhysics *physics, int id, const idVec3 &goal );
						// update the goal position
	void				SetGoalPosition( const idVec3 &goal );


public: // common force interface
	virtual void		Evaluate( int time );
	virtual void		RemovePhysics( const idPhysics *phys );

	// Get the distance from object to goal position
	float				GetDistanceToGoal( void );

private:

	// properties
	float				damping;
	idVec3				goalPosition;

	float				distanceToGoal;

	// positioning
	idPhysics *			physics;		// physics object
	int					id;				// clip model id of physics object
};

#endif /* !__FORCE_GRAB_H__ */

// <---sikk