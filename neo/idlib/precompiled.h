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

#ifndef __PRECOMPILED_H__
#define __PRECOMPILED_H__

#ifdef __cplusplus

//-----------------------------------------------------

#ifdef _WIN32

#if defined(_MFC_VER) && !defined(_D3SDK) && !defined(GAME_DLL)
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// prevent auto literal to string conversion
#include "tools/comafx/StdAfx.h"
#endif

#include <winsock2.h>
#include <mmsystem.h>
#include <mmreg.h>

#define DIRECTINPUT_VERSION  0x0800			// was 0x0700 with the old mssdk
#include <dinput.h>

#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4996)				// unsafe string operations

#include <malloc.h>							// no malloc.h on mac or unix

#endif /* _WIN32 */

//-----------------------------------------------------

#if !defined( _DEBUG ) && !defined( NDEBUG )
	// don't generate asserts
	#define NDEBUG
#endif

//-----------------------------------------------------

// non-portable system services
#include "sys/platform.h"
#include "sys/sys_public.h"

// id lib
#include "idlib/Lib.h"

// memory management and arrays
#include "idlib/Heap.h"
#include "idlib/containers/List.h"

// math
#include "idlib/math/Simd.h"
#include "idlib/math/Math.h"
#include "idlib/math/Random.h"
#include "idlib/math/Complex.h"
#include "idlib/math/Vector.h"
#include "idlib/math/Matrix.h"
#include "idlib/math/Angles.h"
#include "idlib/math/Quat.h"
#include "idlib/math/Rotation.h"
#include "idlib/math/Plane.h"
#include "idlib/math/Pluecker.h"
#include "idlib/math/Polynomial.h"
#include "idlib/math/Extrapolate.h"
#include "idlib/math/Interpolate.h"
#include "idlib/math/Curve.h"
#include "idlib/math/Ode.h"
#include "idlib/math/Lcp.h"

// bounding volumes
#include "idlib/bv/Sphere.h"
#include "idlib/bv/Bounds.h"
#include "idlib/bv/Box.h"
#include "idlib/bv/Frustum.h"

// geometry
#include "idlib/geometry/DrawVert.h"
#include "idlib/geometry/JointTransform.h"
#include "idlib/geometry/Winding.h"
#include "idlib/geometry/Winding2D.h"
#include "idlib/geometry/Surface.h"
#include "idlib/geometry/Surface_Patch.h"
#include "idlib/geometry/Surface_Polytope.h"
#include "idlib/geometry/Surface_SweptSpline.h"
#include "idlib/geometry/TraceModel.h"

// text manipulation
#include "idlib/Str.h"
#include "idlib/Token.h"
#include "idlib/Lexer.h"
#include "idlib/Parser.h"
#include "idlib/Base64.h"
#include "idlib/CmdArgs.h"

// containers
#include "idlib/containers/BTree.h"
#include "idlib/containers/BinSearch.h"
#include "idlib/containers/HashIndex.h"
#include "idlib/containers/HashTable.h"
#include "idlib/containers/StaticList.h"
#include "idlib/containers/LinkList.h"
#include "idlib/containers/Hierarchy.h"
#include "idlib/containers/Queue.h"
#include "idlib/containers/Stack.h"
#include "idlib/containers/StrList.h"
#include "idlib/containers/StrPool.h"
#include "idlib/containers/VectorSet.h"
#include "idlib/containers/PlaneSet.h"

// hashing
#include "idlib/hashing/CRC32.h"
#include "idlib/hashing/MD4.h"
#include "idlib/hashing/MD5.h"

// misc
#include "idlib/Dict.h"
#include "idlib/LangDict.h"
#include "idlib/BitMsg.h"
#include "idlib/MapFile.h"
#include "idlib/Timer.h"

// framework
#include "framework/BuildVersion.h"
#include "framework/Licensee.h"
#include "framework/CmdSystem.h"
#include "framework/CVarSystem.h"
#include "framework/Common.h"
#include "framework/File.h"
#include "framework/FileSystem.h"
#include "framework/UsercmdGen.h"

// decls
#include "framework/DeclManager.h"
#include "framework/DeclTable.h"
#include "framework/DeclSkin.h"
#include "framework/DeclEntityDef.h"
#include "framework/DeclFX.h"
#include "framework/DeclParticle.h"
#include "framework/DeclAF.h"
#include "framework/DeclPDA.h"

// We have expression parsing and evaluation code in multiple places:
// materials, sound shaders, and guis. We should unify them.

// renderer
#include "renderer/qgl.h"
#include "renderer/Cinematic.h"
#include "renderer/Material.h"
#include "renderer/Model.h"
#include "renderer/ModelManager.h"
#include "renderer/RenderSystem.h"
#include "renderer/RenderWorld.h"

// sound engine
#include "sound/sound.h"

// asynchronous networking
#include "framework/async/NetworkSystem.h"

// user interfaces
#include "ui/ListGUI.h"
#include "ui/UserInterface.h"

// collision detection system
#include "cm/CollisionModel.h"

// AAS files and manager
#include "tools/compilers/aas/AASFile.h"
#include "tools/compilers/aas/AASFileManager.h"

// game interface
#include "framework/Game.h"

//-----------------------------------------------------

#ifndef _D3SDK

#ifdef GAME_DLL
#include "Game_local.h"
#else

#include "framework/DemoChecksum.h"

// framework
#include "framework/Compressor.h"
#include "framework/EventLoop.h"
#include "framework/KeyInput.h"
#include "framework/EditField.h"
#include "framework/Console.h"
#include "framework/DemoFile.h"
#include "framework/Session.h"

// asynchronous networking
#include "framework/async/AsyncNetwork.h"

// The editor entry points are always declared, but may just be
// stubbed out on non-windows platforms.
#include "tools/edit_public.h"

// Compilers for map, model, video etc. processing.
#include "tools/compilers/compiler_public.h"

#endif /* !GAME_DLL */

#endif /* !_D3SDK */

//-----------------------------------------------------

#endif	/* __cplusplus */

#endif /* !__PRECOMPILED_H__ */
