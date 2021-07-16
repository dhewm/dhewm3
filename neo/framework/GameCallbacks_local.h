/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2018 Daniel Gibson

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

// Implementation details for idCommon::SetCallback() and idCommon::GetAdditionalFunction()
// Needed in different parts of the Engine (that are supposed to call the callbacks)
// but *not* part of the Game API exported to Game DLLs.
// Common.h (above idCommon::SetCallback()) has a lengthy explanation of what all this is good for..

#ifndef NEO_FRAMEWORK_GAMECALLBACKS_LOCAL_H_
#define NEO_FRAMEWORK_GAMECALLBACKS_LOCAL_H_

#include "Common.h"

struct idGameCallbacks {

	typedef void (*ReloadImagesCallback)(void* userArg, const idCmdArgs &args);
	ReloadImagesCallback	reloadImagesCB;
	void*					reloadImagesUserArg;

	idGameCallbacks();

	// called when Game DLL is unloaded (=> the registered callbacks become invalid)
	void Reset();

};

extern idGameCallbacks gameCallbacks;


#endif /* NEO_FRAMEWORK_GAMECALLBACKS_LOCAL_H_ */
