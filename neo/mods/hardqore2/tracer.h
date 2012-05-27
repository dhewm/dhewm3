
#ifdef _DENTONMOD

#ifndef __GAME_TRACER_H__
#define __GAME_TRACER_H__


// TypeInfo flags
enum {
	TR_TYPE_TRACEREFFECT			= 1,		// dnTracerEffect Type
	TR_TYPE_TRACER					= 2,		// dnTracer Type
	TR_TYPE_SPEEDTRACER				= 4,		// dnSpeedTracer Type
	TR_TYPE_BEAMTRACER				= 8,		// dnBeamTracer Type
	TR_TYPE_BEAMSPEEDTRACER			= 16,		// dnBeamSpeedTracer Type
	TR_TYPE_RAILBEAM				= 32,		// dnBeamSpeedTracer Type
	TR_TYPE_BARRELLAUNCHEDTRACER	= 64,		// dnBarrelLaunchedBeamTracer Type
};

/*
===============================================================================
  dnTracerEffect:

  Abstract Base Class for all types of tracers
===============================================================================
*/


class idEntity;

class dnTracerEffect {
protected:
	idEntity *owner;

public :
					dnTracerEffect( idEntity *owner ); // a projectileEntity
 virtual			~dnTracerEffect() {  };

//	virtual void	Save( idSaveGame *savefile ) const; // tracers are not important enough to be saved
//	virtual void	Restore( idRestoreGame *savefile ); // Implement these if you wish to save the tracer data

	virtual void	Think( void ) = 0;

//-------------------------------------------------
// for simple and rather simulated RTTI, since we don't need
//	to store hoards of type-info for a tracer effect.
//-------------------------------------------------
private:
	int typeInfo;

protected:
	void			SetType( int typeFlag )	{ typeInfo |= typeFlag; } 

public:
	static int		Type( void )				{ return TR_TYPE_TRACEREFFECT; } 
	bool			IsType( int typeFlag )	const	{ return (typeInfo & typeFlag)? true:false;	}
//-------------------------------------------------
};

ID_INLINE dnTracerEffect::dnTracerEffect( idEntity *owner ) {
	this->owner = owner;
	typeInfo = TR_TYPE_TRACEREFFECT;
}


/*
===============================================================================
  dnTracer:

  A simple tracer Effect that uses projectiles renderEntity rather than it's own.
===============================================================================
*/

class dnTracer : public dnTracerEffect {
protected:
	idVec3	playerViewOrigin;
	idVec3	weaponMuzzleOrigin;
	idMat3	axis;
	float	distanceRatio;

public :
			 dnTracer	( idEntity *owner ) : dnTracerEffect(owner) { SetType( TR_TYPE_TRACER ); }	// this constructor only used by derived classes
			 dnTracer	( idEntity *owner, const float distanceRatio, const idVec3 &viewOrigin, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ); 
	virtual ~dnTracer	() {  }

	virtual void	Think( void );
	static int		Type( void )					{ return TR_TYPE_TRACER; } 
};

/*
===============================================================================
  dnSpeedTracer:

  For tracer effect with user defined speeds, uses it's own renderEntity model.
===============================================================================
*/

class dnSpeedTracer : public dnTracerEffect {
protected:
	idVec3			lastPos;
	float			speed;
	float			flyTime;			
	float			distance;
	qhandle_t		modelDefHandle;
	renderEntity_t	renderEntity;

public :
	dnSpeedTracer	( idEntity *owner ):dnTracerEffect(owner) { SetType(TR_TYPE_SPEEDTRACER); }
			 dnSpeedTracer	( idEntity *owner, const float speed, const float distance, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis );
	virtual	~dnSpeedTracer	();

	bool			UpdatePosition( void );
	bool			IsDead( void ) { return (modelDefHandle==-1); }
	virtual void	Think( void ) ;
	static int		Type( void ) { return TR_TYPE_SPEEDTRACER; } 
};

ID_INLINE dnSpeedTracer::~dnSpeedTracer() {
	if ( modelDefHandle >= 0){
		gameRenderWorld->FreeEntityDef( modelDefHandle );
	}
}

/*
===============================================================================
  dnBeamTracer:

  Beam model based dnTracer
===============================================================================
*/

class dnBeamTracer : public dnTracer {

protected:
	float		length;
public :
			 dnBeamTracer( idEntity *owner, const float distanceRatio, const idVec3 &viewOrigin, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ); 
	virtual	~dnBeamTracer(){};

	void		Think( void ) ;
	static int	Type( void )	{ return TR_TYPE_BEAMTRACER; } 
};

/*
===============================================================================
  dnBarrelLaunchedBeamTracer:

  Beam model based tracer for barrel launched projectiles.
===============================================================================
*/

class dnBarrelLaunchedBeamTracer : public dnTracerEffect {

protected:
	idVec3		weaponMuzzleOrigin;
	float		length;
public :
			 dnBarrelLaunchedBeamTracer( idEntity *owner ); 
	virtual	~dnBarrelLaunchedBeamTracer(){};

	void		Think( void ) ;
	static int	Type( void )	{ return TR_TYPE_BARRELLAUNCHEDTRACER; } 
};

/*
===============================================================================
  dnBeamSpeedTracer:

  Beam model based dnSpeedTracer
===============================================================================
*/

class dnBeamSpeedTracer : public dnSpeedTracer {
private:
	idVec3		muzzleOrigin;
protected:
	float		length;
public :
			 dnBeamSpeedTracer( idEntity *owner, const float speed, const float distance, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ); 
	virtual	~dnBeamSpeedTracer(){};

	void		Think( void ) ;
	static int	Type( void )	{ return TR_TYPE_BEAMSPEEDTRACER; } 
};

/*
===============================================================================
  dnRailBeam:

  For Railgun like beam effect
===============================================================================
*/

class dnRailBeam : public dnTracerEffect {
private:
	int				fadeOut;			
	int				time;		// In seconds
	int				previousTime;

	float			offset;
	idVec3			smokeOffset;
	float			beamEndWidth;
	float			deltaWidthPerMsec;

	idVec4			fadeColor;
	idVec4			fadeOutIntervals;

	qhandle_t		modelDefHandle;
	renderEntity_t	renderEntity;

	float			smokeLength;
	int				nSmokeParticles;
	int				smokeStartTime;
	const			idDeclParticle* smokeParticle;//Used for smoke particles

public :
			 dnRailBeam( idEntity *owner, const idVec3 &beamStart );	
	virtual	~dnRailBeam() {
		if ( modelDefHandle >= 0){
			gameRenderWorld->FreeEntityDef( modelDefHandle );
		}
	};

	void		Create( const idVec3 &beamEnd );
	void		Think( void ) ;
	static int	Type( void )	{ return TR_TYPE_RAILBEAM; } 
};
#endif /* !__GAME_TRACER_H__ */

#endif /* !_DENTONMOD */
