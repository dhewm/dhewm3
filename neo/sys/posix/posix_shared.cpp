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

#include <time.h>
#include <sys/time.h>

/*
================
Sys_Microseconds
================
*/
unsigned long long Sys_Microseconds() {
#ifdef __APPLE__
	// OSX didn't have clock_gettime() until recently, so use Mach's clock_get_time()
	// instead. fortunately its mach_timespec_t seems identical to POSIX struct timespec
	// so lots of code can be shared
	clock_serv_t cclock;
	mach_timespec_t now;
	static mach_timespec_t first;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
	clock_get_time(cclock, &now);
	mach_port_deallocate(mach_task_self(), cclock);

#else // not __APPLE__ - other Unix-likes will hopefully support clock_gettime()

	struct timespec now;
	static struct timespec first;
#ifdef _POSIX_MONOTONIC_CLOCK
	clock_gettime(CLOCK_MONOTONIC, &now);
#else
	clock_gettime(CLOCK_REALTIME, &now);
#endif

#endif // not __APPLE__

	if(first.tv_sec == 0)
	{
		long long nsec = now.tv_nsec;
		long long sec = now.tv_sec;
		// set back first by 1ms so neither this function nor Sys_Milliseconds()
		// (which calls this) will ever return 0
		nsec -= 1000000;
		if(nsec < 0)
		{
			nsec += 1000000000ll; // 1s in ns => definitely positive now
			--sec;
		}

		first.tv_sec = sec;
		first.tv_nsec = nsec;
	}

	long long sec = now.tv_sec - first.tv_sec;
	long long nsec = now.tv_nsec - first.tv_nsec;

	if(nsec < 0)
	{
		nsec += 1000000000ll; // 1s in ns
		--sec;
	}

	return sec*1000000ll + nsec/1000ll;
}

/*
================
Sys_Milliseconds
================
*/
unsigned int Sys_Milliseconds() {
	return (int)(Sys_Microseconds()/1000ll);
}
