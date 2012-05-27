#ifdef _WATER_PHYSICS

#ifndef __LIQUID_H__
#define __LIQUID_H__

/*
===============================================================================

  idLiquid

	Base class for all liquid object.  The entity part of the liquid is
	responsible for spawning splashes and sounds to match.

	The physics portion is as usual, responsible for the physics.
===============================================================================
*/

class idRenderModelLiquid;

class idLiquid : public idEntity {
public:
	CLASS_PROTOTYPE( idLiquid );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual bool		Collide( const trace_t &collision, const idVec3 &velocity );

private:
	void				Event_Touch( idEntity *other, trace_t *trace );

	idPhysics_Liquid	physicsObj;

	idRenderModelLiquid *model;

	const idDeclParticle *splash[3];
	const idDeclParticle *waves;

	//idStr				smokeName;
	//idStr				soundName;
	bool				skipSound;  //ivan
};

#endif // __LIQUID_H__

#endif // _WATER_PHYSICS