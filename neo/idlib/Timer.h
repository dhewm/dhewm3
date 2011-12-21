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

#ifndef __TIMER_H__
#define __TIMER_H__

#include "idlib/containers/StrList.h"
#include "idlib/Lib.h"
#include "sys/sys_public.h"

/*
===============================================================================

	Clock tick counter. Should only be used for profiling.

===============================================================================
*/

class idTimer {
public:
					idTimer( void );
					idTimer( unsigned int ms );
					~idTimer( void );

	idTimer			operator+( const idTimer &t ) const;
	idTimer			operator-( const idTimer &t ) const;
	idTimer &		operator+=( const idTimer &t );
	idTimer &		operator-=( const idTimer &t );

	void			Start( void );
	void			Stop( void );
	void			Clear( void );
	unsigned int	Milliseconds( void ) const;

private:
	enum			{
						TS_STARTED,
						TS_STOPPED
					} state;
	unsigned int	start;
	unsigned int	ms;
};

/*
=================
idTimer::idTimer
=================
*/
ID_INLINE idTimer::idTimer( void ) {
	state = TS_STOPPED;
	ms = 0;
}

/*
=================
idTimer::idTimer
=================
*/
ID_INLINE idTimer::idTimer( unsigned int _ms ) {
	state = TS_STOPPED;
	ms = _ms;
}

/*
=================
idTimer::~idTimer
=================
*/
ID_INLINE idTimer::~idTimer( void ) {
}

/*
=================
idTimer::operator+
=================
*/
ID_INLINE idTimer idTimer::operator+( const idTimer &t ) const {
	assert( state == TS_STOPPED && t.state == TS_STOPPED );
	return idTimer( ms + t.ms );
}

/*
=================
idTimer::operator-
=================
*/
ID_INLINE idTimer idTimer::operator-( const idTimer &t ) const {
	assert( state == TS_STOPPED && t.state == TS_STOPPED );
	return idTimer( ms - t.ms );
}

/*
=================
idTimer::operator+=
=================
*/
ID_INLINE idTimer &idTimer::operator+=( const idTimer &t ) {
	assert( state == TS_STOPPED && t.state == TS_STOPPED );
	ms += t.ms;
	return *this;
}

/*
=================
idTimer::operator-=
=================
*/
ID_INLINE idTimer &idTimer::operator-=( const idTimer &t ) {
	assert( state == TS_STOPPED && t.state == TS_STOPPED );
	ms -= t.ms;
	return *this;
}

/*
=================
idTimer::Start
=================
*/
ID_INLINE void idTimer::Start( void ) {
	assert( state == TS_STOPPED );
	state = TS_STARTED;
	start = idLib::sys->GetMilliseconds();
}

/*
=================
idTimer::Stop
=================
*/
ID_INLINE void idTimer::Stop( void ) {
	assert( state == TS_STARTED );
	ms += idLib::sys->GetMilliseconds() - start;
	state = TS_STOPPED;
}

/*
=================
idTimer::Clear
=================
*/
ID_INLINE void idTimer::Clear( void ) {
	ms = 0;
}

/*
=================
idTimer::Milliseconds
=================
*/
ID_INLINE unsigned int idTimer::Milliseconds( void ) const {
	assert( state == TS_STOPPED );
	return ms;
}


/*
===============================================================================

	Report of multiple named timers.

===============================================================================
*/

class idTimerReport {
public:
					idTimerReport( void );
					~idTimerReport( void );

	void			SetReportName( const char *name );
	int				AddReport( const char *name );
	void			Clear( void );
	void			Reset( void );
	void			PrintReport( void );
	void			AddTime( const char *name, idTimer *time );

private:
	idList<idTimer*>timers;
	idStrList		names;
	idStr			reportName;
};

#endif /* !__TIMER_H__ */
