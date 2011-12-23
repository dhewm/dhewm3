project(doom3)
cmake_minimum_required(VERSION 2.6)

# TODO

# osx: -weak_framework ?

# how to name the game .so's?

# maybe add these as options:
# BUILD_GAMEPAK
# TARGET_MONO
# SETUP
# SDK				-D_D3SDK

# don't add these as options, but document them?
# TARGET_DEMO		there are no demo pk4s compatible to this 1.3.1 codebase
# IDNET_HOST		-DIDNET_HOST=\\"%s\\"' % IDNET_HOST
# DEBUG_MEMORY		-DID_DEBUG_MEMORY', '-DID_REDIRECT_NEWDELETE
# LIBC_MALLOC		-DUSE_LIBC_MALLOC=0
# ID_NOLANADDRESS	-DID_NOLANADDRESS
# ID_MCHECK			-DID_MCHECK

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sys/cmake")
set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)

include(CheckCXXCompilerFlag)
include(CheckFunctionExists)

option(CORE			"Build the core" ON)
option(GAME			"Build the base game code" ON)
option(D3XP			"Build the d3xp game code" OFF)
option(DEDICATED	"Build the dedicated server" OFF)
option(X86			"Cross compile for x86 (only applicable on x86_64)" OFF)
option(CURL			"Enable usage of libcurl for http/ftp downloads" OFF)
option(ONATIVE		"Optimize for the host CPU" OFF)

if (NOT CMAKE_SYSTEM_PROCESSOR)
	message(FATAL_ERROR "No target CPU architecture set")
endif()

if (NOT CMAKE_SYSTEM_NAME)
	message(FATAL_ERROR "No target OS set")
endif()

# target cpu
set(cpu ${CMAKE_SYSTEM_PROCESSOR})
if (cpu STREQUAL "powerpc")
	set(cpu "ppc")
elseif (cpu MATCHES "i.86")
	set(cpu "x86")
endif()

if (cpu STREQUAL "x86_64" AND X86)
	set(cpu "x86")
endif()

# target os
if (APPLE)
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

# build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# libs
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

set(CMAKE_REQUIRED_INCLUDES ${JPEG_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARY})
CHECK_FUNCTION_EXISTS("jpeg_mem_src" jpeg_has_mem_src)
if (jpeg_has_mem_src)
	add_definitions(-DHAVE_JPEG_MEM_SRC)
endif ()

find_package(OGG REQUIRED)
include_directories(${OGG_INCLUDE_DIR})

find_package(Vorbis REQUIRED)
include_directories(${VORBIS_INCLUDE_DIR})

find_package(VorbisFile REQUIRED)
include_directories(${VORBISFILE_INCLUDE_DIR})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

if (UNIX AND NOT APPLE)
	find_package(X11 REQUIRED)
	add_definitions(-DXTHREADS)
	include_directories(${X11_INCLUDE_DIR})

	if (os MATCHES ".*bsd" OR os STREQUAL "dragonfly")
		set(Xxf86vm /usr/local/lib/libXxf86vm.so)
	else()
		set(Xxf86vm Xxf86vm)
	endif()
endif()

if (CURL)
	find_package(CURL REQUIRED)
	add_definitions(-DID_ENABLE_CURL=1)
	include_directories(${CURL_INCLUDE_DIR})
else()
	add_definitions(-DID_ENABLE_CURL=0)
endif()

# compiler specific flags
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_definitions(-pipe)
	add_definitions(-Wall)

	if (NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_definitions(-march=native)
	elseif (NOT APPLE AND cpu STREQUAL "x86")
		add_definitions(-march=pentium3)
	endif()

	set(CMAKE_C_FLAGS_DEBUG "-g -D_DEBUG -O1")
	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")

	set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})

	add_definitions(-fno-strict-aliasing)

	CHECK_CXX_COMPILER_FLAG("-fvisibility=hidden" cxx_has_fvisibility)
	if (NOT cxx_has_fvisibility)
		message(FATAL_ERROR "Compiler does not support -fvisibility")
	endif()
	add_definitions(-fvisibility=hidden)

	# TODO fix these warnings
	add_definitions(-Wno-sign-compare)
	add_definitions(-Wno-switch)
	add_definitions(-Wno-format-security)

	CHECK_CXX_COMPILER_FLAG("-Woverloaded-virtual" cxx_has_Woverload_virtual)
	if (cxx_has_Woverload_virtual)
		add_definitions(-Woverloaded-virtual)
	endif()

	if (APPLE)
		add_definitions(-DMACOS_X=1)

		if (cpu STREQUAL "x86_64")
			add_definitions(-arch x86_64 -mmacosx-version-min=10.6)
			set(ldflags ${ldflags} "-arch x86_64 -mmacosx-version-min=10.6")
		elseif (cpu STREQUAL "x86")
			CHECK_CXX_COMPILER_FLAG("-arch i386" cxx_has_arch_i386)
			if (cxx_has_arch_i386)
				add_definitions(-arch i386)
				set(ldflags ${ldflags} "-arch i386")
			endif()

			add_definitions(-mmacosx-version-min=10.4)
			set(ldflags ${ldflags} "-mmacosx-version-min=10.4")
		elseif (cpu STREQUAL "ppc")
			CHECK_CXX_COMPILER_FLAG("-arch ppc" cxx_has_arch_ppc)
			if (cxx_has_arch_ppc)
				add_definitions(-arch ppc)
				set(ldflags ${ldflags} "-arch ppc")
			endif()

			add_definitions(-mmacosx-version-min=10.4)
			set(ldflags ${ldflags} "-mmacosx-version-min=10.4")
		else()
			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
		endif()

		set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework OpenGL -framework OpenAL -framework IOKit")
	else()
		if (cpu STREQUAL "x86" AND X86)
			add_definitions(-m32)
			set(ldflags ${ldflags} "-m32")
		endif()

		if (os STREQUAL "linux")
			set(sys_libs ${sys_libs} dl)
		endif()
	endif()
elseif (MSVC)
	add_definitions(/W4)
	add_definitions(/wd4714) # 'function' marked as __forceinline not inlined
	add_definitions(/wd4996) # 'function': was declared deprecated
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
if (WIN32)
	add_definitions(-DWINVER=0x0501)
	add_definitions(-D_WIN32_WINNT=0x0501)

	set(sys_libs ${sys_libs}
		dinput8
		ksuser
		dxguid
		winmm
		iphlpapi
		wsock32
	)

	if (MINGW)
		list(APPEND sys_libs dxerr8)
	else()
		list(APPEND sys_libs dxerr)
	endif()
endif()

message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}.${cpu}")

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
	renderer/draw_arb.cpp
	renderer/draw_arb2.cpp
	renderer/draw_common.cpp
	renderer/draw_nv10.cpp
	renderer/draw_nv20.cpp
	renderer/draw_r200.cpp
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
	framework/Unzip.cpp
	framework/UsercmdGen.cpp
	framework/Session_menu.cpp
	framework/Session.cpp
	framework/async/AsyncClient.cpp
	framework/async/AsyncNetwork.cpp
	framework/async/AsyncServer.cpp
	framework/async/MsgChannel.cpp
	framework/async/NetworkSystem.cpp
	framework/async/ServerScan.cpp
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
set(src_stub_sys		sys/stub/sys_stub.cpp)
set(src_stub_util		sys/stub/util_stub.cpp)

set(src_sys_dedicated	sys/linux/dedicated.cpp)

if (APPLE)
	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/sys_local.cpp
		sys/posix/posix_net.cpp
		sys/posix/posix_signal.cpp
		sys/posix/posix_main.cpp
	)

	set(src_sys_core
		sys/posix/posix_input.cpp
		sys/osx/PickMonitor.cpp
		sys/osx/PreferencesDialog.cpp
		sys/osx/macosx_guids.cpp
		sys/osx/DOOMController.mm
		sys/osx/macosx_event.mm
		sys/osx/macosx_glimp.mm
		sys/osx/macosx_misc.mm
#		sys/osx/macosx_utils.mm
#		sys/osx/macosx_sys.mm
	)
elseif (WIN32)
	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/sys_local.cpp
		sys/win32/win_cpu.cpp
#		sys/win32/win_gamma.cpp
		sys/win32/win_glimp.cpp
		sys/win32/win_input.cpp
		sys/win32/win_main.cpp
		sys/win32/win_net.cpp
		sys/win32/win_qgl.cpp
		sys/win32/win_shared.cpp
		sys/win32/win_syscon.cpp
		sys/win32/win_taskkeyhook.cpp
		sys/win32/win_wndproc.cpp
	)

	set(src_sys_core
		${src_stub_util}
	)
else()
	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/sys_local.cpp
		sys/posix/posix_net.cpp
		sys/posix/posix_signal.cpp
		sys/posix/posix_main.cpp
		sys/linux/main.cpp
	)

	set(src_sys_core
		${src_stub_util}
		sys/posix/posix_input.cpp
		sys/linux/glimp.cpp
		sys/linux/input.cpp
		sys/linux/libXNVCtrl/NVCtrl.c
	)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(idlib${cpu} STATIC ${src_idlib})
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT MINGW)
	set_target_properties(idlib${cpu} PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

if (CORE)
	add_executable(doom3.${cpu} WIN32
		${src_core}
		${src_sys_base}
		${src_sys_core}
	)

	set_target_properties(doom3.${cpu} PROPERTIES COMPILE_DEFINITIONS "__DOOM_DLL__")
	set_target_properties(doom3.${cpu} PROPERTIES LINK_FLAGS "${ldflags}")
	target_link_libraries(doom3.${cpu}
		idlib${cpu}
		${X11_X11_LIB} ${X11_Xext_LIB} ${Xxf86vm}
		${OPENGL_gl_LIBRARY}
		${OPENAL_LIBRARY}
		${OGG_LIBRARIES}
		${VORBISFILE_LIBRARIES}
		${VORBIS_LIBRARIES}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
		${SDL_LIBRARY}
		${sys_libs}
	)
endif()

if (DEDICATED)
	add_executable(doom3ded.${cpu} WIN32
		${src_core}
		${src_stub_openal}
		${src_stub_gl}
		${src_stub_util}
		${src_sys_base}
		${src_sys_dedicated}
	)

	set_target_properties(doom3ded.${cpu} PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED;__DOOM_DLL__")
	set_target_properties(doom3ded.${cpu} PROPERTIES LINK_FLAGS "${ldflags}")
	target_link_libraries(doom3ded.${cpu}
		idlib${cpu}
		${VORBISFILE_LIBRARIES}
		${VORBIS_LIBRARIES}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
		${SDL_LIBRARY}
		${sys_libs}
	)
endif()

if (GAME)
	add_library(game${cpu} SHARED ${src_game})
	set_target_properties(game${cpu} PROPERTIES PREFIX "")
	set_target_properties(game${cpu} PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
	set_target_properties(game${cpu} PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/game")
	set_target_properties(game${cpu} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(game${cpu} PROPERTIES INSTALL_NAME_DIR "@executable_path")
	target_link_libraries(game${cpu} idlib${cpu})
endif()

if (D3XP)
	add_library(game${cpu}-d3xp SHARED ${src_d3xp})
	set_target_properties(game${cpu}-d3xp PROPERTIES PREFIX "")
	set_target_properties(game${cpu}-d3xp PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
	set_target_properties(game${cpu}-d3xp PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/d3xp")
	set_target_properties(game${cpu}-d3xp PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(game${cpu}-d3xp PROPERTIES INSTALL_NAME_DIR "@executable_path")
	target_link_libraries(game${cpu}-d3xp idlib${cpu})
endif()
