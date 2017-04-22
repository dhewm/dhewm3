cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(dhewm3)

# TODO

# osx: place game .dylib's in the bundle (next to the binary)
# osx: -weak_framework ?

# maybe add these as options:
# TARGET_MONO
# SETUP
# SDK				-D_D3SDK

# don't add these as options, but document them?
# IDNET_HOST		-DIDNET_HOST=\\"%s\\"' % IDNET_HOST
# DEBUG_MEMORY		-DID_DEBUG_MEMORY', '-DID_REDIRECT_NEWDELETE
# LIBC_MALLOC		-DUSE_LIBC_MALLOC=0
# ID_NOLANADDRESS	-DID_NOLANADDRESS

# fallback for cmake versions without add_compile_options
if(NOT COMMAND add_compile_options)
	function(add_compile_options)
		foreach(arg ${ARGN})
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${arg}" PARENT_SCOPE)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${arg}" PARENT_SCOPE)
		endforeach()
	endfunction()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sys/cmake")
set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)

set(DHEWM3BINARY "dhewm3")

include(CheckCXXCompilerFlag)
include(GNUInstallDirs OPTIONAL RESULT_VARIABLE GNUINSTALLDIRS)

option(CORE			"Build the core" ON)
option(BASE			"Build the base game code" ON)
option(D3XP			"Build the d3xp game code" ON)
option(DEDICATED	"Build the dedicated server" OFF)
option(ONATIVE		"Optimize for the host CPU" OFF)
option(SDL2			"Use SDL2 instead of SDL1.2" ON)

if(NOT CMAKE_SYSTEM_PROCESSOR)
	message(FATAL_ERROR "No target CPU architecture set")
endif()

if(NOT CMAKE_SYSTEM_NAME)
	message(FATAL_ERROR "No target OS set")
endif()

# target cpu
set(cpu ${CMAKE_SYSTEM_PROCESSOR})
if(cpu STREQUAL "powerpc")
	set(cpu "ppc")
elseif(cpu MATCHES "i.86")
	set(cpu "x86")
endif()

if(MSVC AND CMAKE_CL_64)
	set(cpu "amd64")
endif()

# target os
if(APPLE)
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

# build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# precompiled libraries from the dhewm3-libs repo
if(DHEWM3LIBS)
	if(CMAKE_CROSSCOMPILING)
		set(CMAKE_FIND_ROOT_PATH ${DHEWM3LIBS})
	else()
		set(ENV{CMAKE_PREFIX_PATH} ${DHEWM3LIBS})
	endif()

	# these are too stupid, give them a hint
	set(ENV{OPENALDIR} ${DHEWM3LIBS})
	set(ENV{SDLDIR} ${DHEWM3LIBS})
	set(ENV{SDL2DIR} ${DHEWM3LIBS})
endif()

# libs
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

set(CMAKE_REQUIRED_INCLUDES ${JPEG_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARY})

find_package(OGG REQUIRED)
include_directories(${OGG_INCLUDE_DIR})

find_package(Vorbis REQUIRED)
include_directories(${VORBIS_INCLUDE_DIR})

find_package(VorbisFile REQUIRED)
include_directories(${VORBISFILE_INCLUDE_DIR})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

if(NOT AROS)
	find_package(X11 REQUIRED)
	include_directories(${X11_INCLUDE_DIR})
endif()

if (SDL2)
	# skip SDL2main
	if(APPLE OR WIN32)
		set(SDL2_BUILDING_LIBRARY TRUE)
	endif()
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIR})
	set(SDLx_LIBRARY ${SDL2_LIBRARY})
else()
	# skip SDLmain
	if(APPLE OR WIN32)
		set(SDL_BUILDING_LIBRARY TRUE)
	endif()
	find_package(SDL REQUIRED)
	include_directories(${SDL_INCLUDE_DIR})
	set(SDLx_LIBRARY ${SDL_LIBRARY})
endif()

find_package(CURL QUIET)
if(CURL_FOUND)
	set(ID_ENABLE_CURL ON)
	include_directories(${CURL_INCLUDE_DIR})
else()
	message(STATUS "libcurl not found, server downloads won't be available")
	set(ID_ENABLE_CURL OFF)
	set(CURL_LIBRARY "")
endif()

# compiler specific flags
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-pipe)
	add_compile_options(-Wall)

	if(NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_compile_options(-march=native)
	elseif(NOT APPLE AND cpu STREQUAL "x86")
		add_compile_options(-march=pentium3)
	endif()

	set(CMAKE_C_FLAGS_DEBUG "-g -D_DEBUG -O1")
	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_PROFILE "-g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")

	set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})
	set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE})

	add_compile_options(-fno-strict-aliasing)

	if(NOT AROS)
		CHECK_CXX_COMPILER_FLAG("-fvisibility=hidden" cxx_has_fvisibility)
		if(NOT cxx_has_fvisibility)
			message(FATAL_ERROR "Compiler does not support -fvisibility")
		endif()
		add_compile_options(-fvisibility=hidden)
	endif()

	# TODO fix these warnings
	add_compile_options(-Wno-sign-compare)
	add_compile_options(-Wno-switch)
	add_compile_options(-Wno-format-security)

	CHECK_CXX_COMPILER_FLAG("-Woverloaded-virtual" cxx_has_Woverload_virtual)
	if(cxx_has_Woverload_virtual)
		add_compile_options(-Woverloaded-virtual)
	endif()

	if(AROS)
		set(CMAKE_SHARED_LIBRARY_SUFFIX ".aros-${cpu}")
		add_definitions(-DIOAPI_NO_64)
	elseif(APPLE)
		add_definitions(-DMACOS_X=1)

		if(cpu STREQUAL "x86_64")
			add_compile_options(-arch x86_64 -mmacosx-version-min=10.6)
			set(ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.6")
		elseif(cpu STREQUAL "x86")
			CHECK_CXX_COMPILER_FLAG("-arch i386" cxx_has_arch_i386)
			if(cxx_has_arch_i386)
				add_compile_options(-arch i386)
				set(ldflags "${ldflags} -arch i386")
			endif()

			add_compile_options(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		elseif(cpu STREQUAL "ppc")
			CHECK_CXX_COMPILER_FLAG("-arch ppc" cxx_has_arch_ppc)
			if(cxx_has_arch_ppc)
				add_compile_options(-arch ppc)
				set(ldflags "${ldflags} -arch ppc")
			endif()

			add_compile_options(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		else()
			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
		endif()

		set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework IOKit")
	elseif(WIN32)
		set(ldflags "${ldflags} -static-libgcc -static-libstdc++")
	elseif(os STREQUAL "linux")
		set(sys_libs ${sys_libs} dl)
	endif()
elseif(MSVC)
	add_compile_options(/W4)
	add_compile_options(/wd4100) # unreferenced formal parameter
	add_compile_options(/wd4127) # conditional expression is constant
	add_compile_options(/wd4244) # possible loss of data
	add_compile_options(/wd4245) # signed/unsigned mismatch
	add_compile_options(/wd4267) # possible loss of data
	add_compile_options(/wd4714) # 'function' marked as __forceinline not inlined
	add_compile_options(/wd4996) # 'function': was declared deprecated
	add_compile_options(/wd4068) # unknown pragma
	add_definitions(-D_ALLOW_KEYWORD_MACROS) # because of the "#define private public" and "#define protected public" in TypeInfo.cpp
	set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MDd")
	set(CMAKE_C_FLAGS_RELEASE "/Ox /Oy /MD")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /Oy /Zi /MD")
	set(CMAKE_C_FLAGS_MINSIZEREL "/Ox /Oy /Os /MD")
else()
	message(FATAL_ERROR "Unsupported compiler")
endif()

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})

# mingw and msvc
if(WIN32)
	add_definitions(-DWINVER=0x0501)
	add_definitions(-D_WIN32_WINNT=0x0501)

	set(sys_libs ${sys_libs}
		winmm
		iphlpapi
		wsock32
		ole32
	)
endif()

# fallback for cmake versions without GNUInstallDirs
if(GNUINSTALLDIRS MATCHES "NOTFOUND")
	set(CMAKE_INSTALL_BINDIR		"bin"
		CACHE PATH "user executables (bin)")
	set(CMAKE_INSTALL_LIBDIR		"lib${LIB_SUFFIX}"
		CACHE PATH "object code libraries (lib${LIB_SUFFIX})")
	set(CMAKE_INSTALL_DATAROOTDIR	"share"
		CACHE PATH "read-only architecture-independent data root (share)")
	set(CMAKE_INSTALL_DATADIR		"${CMAKE_INSTALL_DATAROOTDIR}"
		CACHE PATH "read-only architecture-independent data (DATAROOTDIR)")

	mark_as_advanced(CMAKE_INSTALL_BINDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_DATAROOTDIR CMAKE_INSTALL_DATADIR)
	foreach(dir BINDIR LIBDIR DATAROOTDIR DATADIR)
		if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_${dir}})
			set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${dir}}")
		else()
			set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_${dir}}")
		endif()
	endforeach()
endif()

set(bindir		"${CMAKE_INSTALL_FULL_BINDIR}")
set(libdir		"${CMAKE_INSTALL_FULL_LIBDIR}/dhewm3")
set(datadir		"${CMAKE_INSTALL_FULL_DATADIR}/dhewm3")

configure_file(
	"${CMAKE_SOURCE_DIR}/config.h.in"
	"${CMAKE_BINARY_DIR}/config.h"
)

message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${cpu}")

if(NOT APPLE AND NOT WIN32)
	message(STATUS "The install target will use the following directories:")
	message(STATUS "  Binary directory:  ${bindir}")
	message(STATUS "  Library directory: ${libdir}")
	message(STATUS "  Data directory:    ${datadir}")
endif()

set(src_renderer
	renderer/Cinematic.cpp
	renderer/GuiModel.cpp
	renderer/Image_files.cpp
	renderer/Image_init.cpp
	renderer/Image_load.cpp
	renderer/Image_process.cpp
	renderer/Image_program.cpp
	renderer/Interaction.cpp
	renderer/Material.cpp
	renderer/MegaTexture.cpp
	renderer/Model.cpp
	renderer/ModelDecal.cpp
	renderer/ModelManager.cpp
	renderer/ModelOverlay.cpp
	renderer/Model_beam.cpp
	renderer/Model_ase.cpp
	renderer/Model_liquid.cpp
	renderer/Model_lwo.cpp
	renderer/Model_ma.cpp
	renderer/Model_md3.cpp
	renderer/Model_md5.cpp
	renderer/Model_prt.cpp
	renderer/Model_sprite.cpp
	renderer/RenderEntity.cpp
	renderer/RenderSystem.cpp
	renderer/RenderSystem_init.cpp
	renderer/RenderWorld.cpp
	renderer/RenderWorld_demo.cpp
	renderer/RenderWorld_load.cpp
	renderer/RenderWorld_portals.cpp
	renderer/VertexCache.cpp
	renderer/draw_arb2.cpp
	renderer/draw_common.cpp
	renderer/tr_backend.cpp
	renderer/tr_deform.cpp
	renderer/tr_font.cpp
	renderer/tr_guisurf.cpp
	renderer/tr_light.cpp
	renderer/tr_lightrun.cpp
	renderer/tr_main.cpp
	renderer/tr_orderIndexes.cpp
	renderer/tr_polytope.cpp
	renderer/tr_render.cpp
	renderer/tr_rendertools.cpp
	renderer/tr_shadowbounds.cpp
	renderer/tr_stencilshadow.cpp
	renderer/tr_subview.cpp
	renderer/tr_trace.cpp
	renderer/tr_trisurf.cpp
	renderer/tr_turboshadow.cpp
)

set(src_framework
	framework/CVarSystem.cpp
	framework/CmdSystem.cpp
	framework/Common.cpp
	framework/Compressor.cpp
	framework/Console.cpp
	framework/DemoFile.cpp
	framework/DeclAF.cpp
	framework/DeclEntityDef.cpp
	framework/DeclFX.cpp
	framework/DeclManager.cpp
	framework/DeclParticle.cpp
	framework/DeclPDA.cpp
	framework/DeclSkin.cpp
	framework/DeclTable.cpp
	framework/EditField.cpp
	framework/EventLoop.cpp
	framework/File.cpp
	framework/FileSystem.cpp
	framework/KeyInput.cpp
	framework/UsercmdGen.cpp
	framework/Session_menu.cpp
	framework/Session.cpp
	framework/async/AsyncClient.cpp
	framework/async/AsyncNetwork.cpp
	framework/async/AsyncServer.cpp
	framework/async/MsgChannel.cpp
	framework/async/NetworkSystem.cpp
	framework/async/ServerScan.cpp
	framework/minizip/ioapi.c
	framework/minizip/unzip.cpp
)

set(src_cm
	cm/CollisionModel_contacts.cpp
	cm/CollisionModel_contents.cpp
	cm/CollisionModel_debug.cpp
	cm/CollisionModel_files.cpp
	cm/CollisionModel_load.cpp
	cm/CollisionModel_rotate.cpp
	cm/CollisionModel_trace.cpp
	cm/CollisionModel_translate.cpp
)

set(src_dmap
	tools/compilers/dmap/dmap.cpp
	tools/compilers/dmap/facebsp.cpp
	tools/compilers/dmap/gldraw.cpp
	tools/compilers/dmap/glfile.cpp
	tools/compilers/dmap/leakfile.cpp
	tools/compilers/dmap/map.cpp
	tools/compilers/dmap/optimize.cpp
	tools/compilers/dmap/output.cpp
	tools/compilers/dmap/portals.cpp
	tools/compilers/dmap/shadowopt3.cpp
	tools/compilers/dmap/tritjunction.cpp
	tools/compilers/dmap/tritools.cpp
	tools/compilers/dmap/ubrush.cpp
	tools/compilers/dmap/usurface.cpp
)

set(src_aas
	tools/compilers/aas/AASBuild.cpp
	tools/compilers/aas/AASBuild_file.cpp
	tools/compilers/aas/AASBuild_gravity.cpp
	tools/compilers/aas/AASBuild_ledge.cpp
	tools/compilers/aas/AASBuild_merge.cpp
	tools/compilers/aas/AASCluster.cpp
	tools/compilers/aas/AASFile.cpp
	tools/compilers/aas/AASFile_optimize.cpp
	tools/compilers/aas/AASFile_sample.cpp
	tools/compilers/aas/AASReach.cpp
	tools/compilers/aas/AASFileManager.cpp
	tools/compilers/aas/Brush.cpp
	tools/compilers/aas/BrushBSP.cpp
)

set(src_roq
	tools/compilers/roqvq/NSBitmapImageRep.cpp
	tools/compilers/roqvq/codec.cpp
	tools/compilers/roqvq/roq.cpp
	tools/compilers/roqvq/roqParam.cpp
)

set(src_renderbump
	tools/compilers/renderbump/renderbump.cpp
)

set(src_snd
	sound/snd_cache.cpp
	sound/snd_decoder.cpp
	sound/snd_efxfile.cpp
	sound/snd_emitter.cpp
	sound/snd_shader.cpp
	sound/snd_system.cpp
	sound/snd_wavefile.cpp
	sound/snd_world.cpp
)

set(src_ui
	ui/BindWindow.cpp
	ui/ChoiceWindow.cpp
	ui/DeviceContext.cpp
	ui/EditWindow.cpp
	ui/FieldWindow.cpp
	ui/GameBearShootWindow.cpp
	ui/GameBustOutWindow.cpp
	ui/GameSSDWindow.cpp
	ui/GuiScript.cpp
	ui/ListGUI.cpp
	ui/ListWindow.cpp
	ui/MarkerWindow.cpp
	ui/RegExp.cpp
	ui/RenderWindow.cpp
	ui/SimpleWindow.cpp
	ui/SliderWindow.cpp
	ui/UserInterface.cpp
	ui/Window.cpp
	ui/Winvar.cpp
)

set(src_tools
	tools/guied/GEWindowWrapper_stub.cpp
)

set(src_idlib
	idlib/bv/Bounds.cpp
	idlib/bv/Frustum.cpp
	idlib/bv/Sphere.cpp
	idlib/bv/Box.cpp
	idlib/geometry/DrawVert.cpp
	idlib/geometry/Winding2D.cpp
	idlib/geometry/Surface_SweptSpline.cpp
	idlib/geometry/Winding.cpp
	idlib/geometry/Surface.cpp
	idlib/geometry/Surface_Patch.cpp
	idlib/geometry/TraceModel.cpp
	idlib/geometry/JointTransform.cpp
	idlib/hashing/CRC32.cpp
	idlib/hashing/MD4.cpp
	idlib/hashing/MD5.cpp
	idlib/math/Angles.cpp
	idlib/math/Lcp.cpp
	idlib/math/Math.cpp
	idlib/math/Matrix.cpp
	idlib/math/Ode.cpp
	idlib/math/Plane.cpp
	idlib/math/Pluecker.cpp
	idlib/math/Polynomial.cpp
	idlib/math/Quat.cpp
	idlib/math/Rotation.cpp
	idlib/math/Simd.cpp
	idlib/math/Simd_Generic.cpp
	idlib/math/Simd_AltiVec.cpp
	idlib/math/Simd_MMX.cpp
	idlib/math/Simd_3DNow.cpp
	idlib/math/Simd_SSE.cpp
	idlib/math/Simd_SSE2.cpp
	idlib/math/Simd_SSE3.cpp
	idlib/math/Vector.cpp
	idlib/BitMsg.cpp
	idlib/LangDict.cpp
	idlib/Lexer.cpp
	idlib/Lib.cpp
	idlib/containers/HashIndex.cpp
	idlib/Dict.cpp
	idlib/Str.cpp
	idlib/Parser.cpp
	idlib/MapFile.cpp
	idlib/CmdArgs.cpp
	idlib/Token.cpp
	idlib/Base64.cpp
	idlib/Timer.cpp
	idlib/Heap.cpp
)

set(src_game
	game/AF.cpp
	game/AFEntity.cpp
	game/Actor.cpp
	game/Camera.cpp
	game/Entity.cpp
	game/BrittleFracture.cpp
	game/Fx.cpp
	game/GameEdit.cpp
	game/Game_local.cpp
	game/Game_network.cpp
	game/Item.cpp
	game/IK.cpp
	game/Light.cpp
	game/Misc.cpp
	game/Mover.cpp
	game/Moveable.cpp
	game/MultiplayerGame.cpp
	game/Player.cpp
	game/PlayerIcon.cpp
	game/PlayerView.cpp
	game/Projectile.cpp
	game/Pvs.cpp
	game/SecurityCamera.cpp
	game/SmokeParticles.cpp
	game/Sound.cpp
	game/Target.cpp
	game/Trigger.cpp
	game/Weapon.cpp
	game/WorldSpawn.cpp
	game/ai/AAS.cpp
	game/ai/AAS_debug.cpp
	game/ai/AAS_pathing.cpp
	game/ai/AAS_routing.cpp
	game/ai/AI.cpp
	game/ai/AI_events.cpp
	game/ai/AI_pathing.cpp
	game/ai/AI_Vagary.cpp
	game/gamesys/DebugGraph.cpp
	game/gamesys/Class.cpp
	game/gamesys/Event.cpp
	game/gamesys/SaveGame.cpp
	game/gamesys/SysCmds.cpp
	game/gamesys/SysCvar.cpp
	game/gamesys/TypeInfo.cpp
	game/anim/Anim.cpp
	game/anim/Anim_Blend.cpp
	game/anim/Anim_Import.cpp
	game/anim/Anim_Testmodel.cpp
	game/script/Script_Compiler.cpp
	game/script/Script_Interpreter.cpp
	game/script/Script_Program.cpp
	game/script/Script_Thread.cpp
	game/physics/Clip.cpp
	game/physics/Force.cpp
	game/physics/Force_Constant.cpp
	game/physics/Force_Drag.cpp
	game/physics/Force_Field.cpp
	game/physics/Force_Spring.cpp
	game/physics/Physics.cpp
	game/physics/Physics_AF.cpp
	game/physics/Physics_Actor.cpp
	game/physics/Physics_Base.cpp
	game/physics/Physics_Monster.cpp
	game/physics/Physics_Parametric.cpp
	game/physics/Physics_Player.cpp
	game/physics/Physics_RigidBody.cpp
	game/physics/Physics_Static.cpp
	game/physics/Physics_StaticMulti.cpp
	game/physics/Push.cpp
)

set(src_d3xp
	d3xp/AF.cpp
	d3xp/AFEntity.cpp
	d3xp/Actor.cpp
	d3xp/Camera.cpp
	d3xp/Entity.cpp
	d3xp/BrittleFracture.cpp
	d3xp/Fx.cpp
	d3xp/GameEdit.cpp
	d3xp/Game_local.cpp
	d3xp/Game_network.cpp
	d3xp/Item.cpp
	d3xp/IK.cpp
	d3xp/Light.cpp
	d3xp/Misc.cpp
	d3xp/Mover.cpp
	d3xp/Moveable.cpp
	d3xp/MultiplayerGame.cpp
	d3xp/Player.cpp
	d3xp/PlayerIcon.cpp
	d3xp/PlayerView.cpp
	d3xp/Projectile.cpp
	d3xp/Pvs.cpp
	d3xp/SecurityCamera.cpp
	d3xp/SmokeParticles.cpp
	d3xp/Sound.cpp
	d3xp/Target.cpp
	d3xp/Trigger.cpp
	d3xp/Weapon.cpp
	d3xp/WorldSpawn.cpp
	d3xp/ai/AAS.cpp
	d3xp/ai/AAS_debug.cpp
	d3xp/ai/AAS_pathing.cpp
	d3xp/ai/AAS_routing.cpp
	d3xp/ai/AI.cpp
	d3xp/ai/AI_events.cpp
	d3xp/ai/AI_pathing.cpp
	d3xp/ai/AI_Vagary.cpp
	d3xp/gamesys/DebugGraph.cpp
	d3xp/gamesys/Class.cpp
	d3xp/gamesys/Event.cpp
	d3xp/gamesys/SaveGame.cpp
	d3xp/gamesys/SysCmds.cpp
	d3xp/gamesys/SysCvar.cpp
	d3xp/gamesys/TypeInfo.cpp
	d3xp/anim/Anim.cpp
	d3xp/anim/Anim_Blend.cpp
	d3xp/anim/Anim_Import.cpp
	d3xp/anim/Anim_Testmodel.cpp
	d3xp/script/Script_Compiler.cpp
	d3xp/script/Script_Interpreter.cpp
	d3xp/script/Script_Program.cpp
	d3xp/script/Script_Thread.cpp
	d3xp/physics/Clip.cpp
	d3xp/physics/Force.cpp
	d3xp/physics/Force_Constant.cpp
	d3xp/physics/Force_Drag.cpp
	d3xp/physics/Force_Field.cpp
	d3xp/physics/Force_Spring.cpp
	d3xp/physics/Physics.cpp
	d3xp/physics/Physics_AF.cpp
	d3xp/physics/Physics_Actor.cpp
	d3xp/physics/Physics_Base.cpp
	d3xp/physics/Physics_Monster.cpp
	d3xp/physics/Physics_Parametric.cpp
	d3xp/physics/Physics_Player.cpp
	d3xp/physics/Physics_RigidBody.cpp
	d3xp/physics/Physics_Static.cpp
	d3xp/physics/Physics_StaticMulti.cpp
	d3xp/physics/Push.cpp
	d3xp/Grabber.cpp
	d3xp/physics/Force_Grab.cpp
)

set(src_core
	${src_renderer}
	${src_framework}
	${src_cm}
	${src_dmap}
	${src_aas}
	${src_roq}
	${src_renderbump}
	${src_snd}
	${src_ui}
	${src_tools}
)

set(src_stub_openal		sys/stub/openal_stub.cpp)
set(src_stub_gl			sys/stub/stub_gl.cpp)

if(AROS)
	set(DHEWM3BINARY "ADoom3")
	set(sys_libs ${sys_libs} dll)

	set(src_arosdll
		sys/aros/dll/dllstartup.c
		sys/aros/dll/dll.c
		sys/aros/dll/dllimport.c
	)

	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/aros/aros_net.cpp
		sys/aros/aros_signal.cpp
		sys/aros/aros_main.cpp
		sys/aros/aros_dos.cpp
	)

	set(src_sys_core
		sys/glimp.cpp
	)
elseif(APPLE)
	set(OSX_RESOURCE_FILES
		"${CMAKE_SOURCE_DIR}/sys/osx/Doom3.icns"
		"${CMAKE_SOURCE_DIR}/sys/osx/Doom 3.rsrc"
	)

	set_source_files_properties(${OSX_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/posix/posix_net.cpp
		sys/posix/posix_main.cpp
	)

	set(src_sys_core
		sys/glimp.cpp
		sys/osx/DOOMController.mm
		sys/osx/macosx_misc.mm
		sys/osx/SDLMain.m
		${OSX_RESOURCE_FILES}
	)
elseif(WIN32)
	set(WIN32_RESOURCE_FILES
		"${CMAKE_SOURCE_DIR}/sys/win32/rc/dhewm3.rc"
	)

	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/win32/win_input.cpp
		sys/win32/win_main.cpp
		sys/win32/win_net.cpp
		sys/win32/win_shared.cpp
		sys/win32/win_syscon.cpp
		sys/win32/SDL_win32_main.c
	)

	set(src_sys_core
		sys/glimp.cpp
		${WIN32_RESOURCE_FILES}
	)
else()
	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/posix/posix_net.cpp
		sys/posix/posix_main.cpp
		sys/linux/main.cpp
	)

	set(src_sys_core
		sys/glimp.cpp
	)
endif()

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_library(idlib STATIC ${src_idlib})
if (AROS)
	add_library(dll STATIC ${src_arosdll})
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
		set(AROS_ARCH "x86")
        else()
		set(AROS_ARCH ${CMAKE_SYSTEM_PROCESSOR})
	endif()
else()
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT MINGW)
		set_target_properties(idlib PROPERTIES COMPILE_FLAGS "-fPIC")
	endif()
endif()

if(CORE)
	add_executable(${DHEWM3BINARY} WIN32 MACOSX_BUNDLE
		${src_core}
		${src_sys_base}
		${src_sys_core}
	)

	set_target_properties(${DHEWM3BINARY} PROPERTIES COMPILE_DEFINITIONS "__DOOM_DLL__")
	set_target_properties(${DHEWM3BINARY} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(${DHEWM3BINARY} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/sys/osx/Info.plist)

	target_link_libraries(${DHEWM3BINARY}
		idlib
		${OPENAL_LIBRARY}
		${VORBISFILE_LIBRARIES}
		${VORBIS_LIBRARIES}
		${OGG_LIBRARIES}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
		${ZLIB_LIBRARY}
		${SDLx_LIBRARY}
		${sys_libs}
	)

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS ${DHEWM3BINARY}
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

if(DEDICATED)
	add_executable(${DHEWM3BINARY}ded WIN32 MACOSX_BUNDLE
		${src_core}
		${src_stub_openal}
		${src_stub_gl}
		${src_sys_base}
	)

	set_target_properties(${DHEWM3BINARY}ded PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED;__DOOM_DLL__")
	set_target_properties(${DHEWM3BINARY}ded PROPERTIES LINK_FLAGS "${ldflags}")
	target_link_libraries(${DHEWM3BINARY}ded
		idlib
		${VORBISFILE_LIBRARIES}
		${VORBIS_LIBRARIES}
		${OGG_LIBRARIES}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
		${ZLIB_LIBRARY}
		${SDLx_LIBRARY}
		${sys_libs}
	)

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS ${DHEWM3BINARY}ded
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

if(BASE)
	if (AROS)
		add_executable(base sys/aros/dll/dllglue.c ${src_game})
		set_target_properties(base PROPERTIES OUTPUT_NAME "base.aros-${AROS_ARCH}")
	else()
		add_library(base SHARED ${src_game})
	endif()
	set_target_properties(base PROPERTIES PREFIX "")
	set_target_properties(base PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
	set_target_properties(base PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/game")
	set_target_properties(base PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(base PROPERTIES INSTALL_NAME_DIR "@executable_path")
	if (AROS)
		target_link_libraries(base idlib dll)
	else()
		target_link_libraries(base idlib)
	endif()

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS base
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

if(D3XP)
	if (AROS)
		add_executable(d3xp sys/aros/dll/dllglue.c ${src_d3xp})
		set_target_properties(d3xp PROPERTIES OUTPUT_NAME "d3xp.aros-${AROS_ARCH}")
	else()
		add_library(d3xp SHARED ${src_d3xp})
	endif()
	set_target_properties(d3xp PROPERTIES PREFIX "")
	set_target_properties(d3xp PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
	set_target_properties(d3xp PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/d3xp")
	set_target_properties(d3xp PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(d3xp PROPERTIES INSTALL_NAME_DIR "@executable_path")
	if (AROS)
		target_link_libraries(d3xp idlib dll)
	else()
		target_link_libraries(d3xp idlib)
	endif()

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS d3xp
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()
