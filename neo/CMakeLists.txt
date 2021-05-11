project(SteelStorm2)
cmake_minimum_required(VERSION 2.6)

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

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sys/cmake")
set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
# Uncomment this next line to force "Show all files" in msvc
# set_property( GLOBAL PROPERTY USE_FOLDERS ON)

include(CheckCXXCompilerFlag)
include(CheckFunctionExists)
include(SourceFileUtilities)

option(CORE			"Build the core" ON)
option(BASE			"Build the base game code" OFF)
option(D3XP			"Build the d3xp game code" ON)
option(TOOLS		"build the tools game code ( Windows only )" ON)
option(DEDICATED	"Build the dedicated server" ON)
option(X86			"Cross compile for x86 (only applicable on x86_64)" OFF)
option(ONATIVE		"Optimize for the host CPU" OFF)
option(ALLOWRESOURCES "Enable Resource compilation into files" ON)

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

if (MSVC AND CMAKE_CL_64)
	set(cpu "x86_64")
#	set(cpu "amd64")
endif()

# game library suffix
set(gamesuffix ${cpu})

# target os
if (APPLE)
# no suffix for .dylib
	set(gamesuffix "")
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

# build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# precompiled libraries from the SteelStorm2-libs repo
if (DHEWM3LIBS)
	if (CMAKE_CROSSCOMPILING)
		set(CMAKE_FIND_ROOT_PATH ${DHEWM3LIBS})
	else()
		set(ENV{CMAKE_PREFIX_PATH} ${DHEWM3LIBS})
	endif()

	# these are too stupid, give them a hint
	set(ENV{OPENALDIR} ${DHEWM3LIBS})
	set(ENV{SDLDIR} ${DHEWM3LIBS})
endif()

# libs
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

set(CMAKE_REQUIRED_INCLUDES ${JPEG_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARY})
CHECK_FUNCTION_EXISTS("jpeg_mem_src" HAVE_JPEG_MEM_SRC)

find_package(OGG REQUIRED)
include_directories(${OGG_INCLUDE_DIR})

find_package(Vorbis REQUIRED)
include_directories(${VORBIS_INCLUDE_DIR})

find_package(VorbisFile REQUIRED)
include_directories(${VORBISFILE_INCLUDE_DIR})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

#find_package(OpenGL REQUIRED)
#include_directories(${OPENGL_INCLUDE_DIR})

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(CURL QUIET)
if (CURL_FOUND)
	set(ID_ENABLE_CURL ON)
	include_directories(${CURL_INCLUDE_DIR})
else()
	message(STATUS "libcurl not found, server downloads won't be available")
	set(ID_ENABLE_CURL OFF)
endif()

# This is required for tools on windows.
find_package(MFC)

# compiler specific flags
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_definitions(-pipe)
	add_definitions(-Wall)

	if (NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_definitions(-march=native)
	elseif (NOT APPLE AND cpu STREQUAL "x86")
		add_definitions(-march=pentium3)
	elseif(cpu STREQUAL "x86_64")
		# These two defines will fix issues involving Collision bugs with floating point round off error.
		add_definitions(-mfpmath=387) # sets x87 fpu as generic fpu by default.
		add_definitions(-msse2) # Enable SSE2 extensions... This should work by default.
	endif()

	set(CMAKE_C_FLAGS_DEBUG "-g -D_DEBUG -O1")
	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_PROFILE "-g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer")

	set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})
	set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE})

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
			set(ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.6")
		elseif (cpu STREQUAL "x86")
			CHECK_CXX_COMPILER_FLAG("-arch i386" cxx_has_arch_i386)
			if (cxx_has_arch_i386)
				add_definitions(-arch i386)
				set(ldflags "${ldflags} -arch i386")
			endif()

			add_definitions(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		elseif (cpu STREQUAL "ppc")
			CHECK_CXX_COMPILER_FLAG("-arch ppc" cxx_has_arch_ppc)
			if (cxx_has_arch_ppc)
				add_definitions(-arch ppc)
				set(ldflags "${ldflags} -arch ppc")
			endif()

			add_definitions(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		else()
			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
		endif()

		#set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework OpenGL -framework IOKit")
		set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework IOKit")
	else()
		if (cpu STREQUAL "x86" AND X86)
			add_definitions(-m32)
			set(ldflags "${ldflags} -m32")
		endif()

		if (os STREQUAL "linux")
			set(sys_libs ${sys_libs} dl)
		endif()
	endif()
elseif (MSVC)
	add_definitions(/W4)
	add_definitions(/wd4100) # unreferenced formal parameter
	add_definitions(/wd4127) # conditional expression is constant
	add_definitions(/wd4244) # possible loss of data
	add_definitions(/wd4245) # signed/unsigned mismatch
	add_definitions(/wd4267) # possible loss of data
	add_definitions(/wd4714) # 'function' marked as __forceinline not inlined
	add_definitions(/wd4996) # 'function': was declared deprecated
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	add_definitions(/Oi) # enable intrinsics.
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
		winmm
		iphlpapi
		wsock32
	)
endif()

if (APPLE)
	SET(bindir		"${CMAKE_INSTALL_PREFIX}")
	SET(libdir		"${CMAKE_INSTALL_PREFIX}")
	SET(datarootdir	"${CMAKE_INSTALL_PREFIX}")
	SET(datadir		"${datarootdir}")
elseif (WIN32)
	SET(bindir		"${CMAKE_INSTALL_PREFIX}")
	SET(libdir		"${CMAKE_INSTALL_PREFIX}/lib")
	SET(datarootdir	"${CMAKE_INSTALL_PREFIX}")
	SET(datadir		"${datarootdir}")
else()
	SET(bindir		"${CMAKE_INSTALL_PREFIX}")
	SET(libdir		"${CMAKE_INSTALL_PREFIX}/lib")
	SET(datarootdir	"${CMAKE_INSTALL_PREFIX}")
	SET(datadir		"${datarootdir}")
endif()

configure_file(
    "${CMAKE_SOURCE_DIR}/config.h.in"
    "${CMAKE_BINARY_DIR}/config.h"
)

message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${cpu}")

# Renderer.
file(GLOB src_renderer "renderer/*.cpp")

# Framework
file(GLOB_RECURSE src_framework "framework/*.cpp")

#Collision Model.
file(GLOB src_cm "cm/*.cpp")	

# Dmap
file(GLOB src_dmap "tools/compilers/dmap/*.cpp")	

# AAS builder.
file(GLOB src_aas "tools/compilers/aas/*.cpp")	

# ROQVQ compiler.
file(GLOB src_roq "tools/compilers/roqvq/*.cpp")	

# renderbump
file(GLOB src_renderbump "tools/compilers/renderbump/*.cpp")	

# Sound
file(GLOB src_snd "sound/*.cpp")	

# UI
file(GLOB src_ui "ui/*.cpp")	

# Gui editor stub.
file(GLOB src_tools "tools/guied/*_stub.cpp")	

# ID Library.
file(GLOB_RECURSE src_idlib "idlib/*.cpp")	
GENERATE_SOURCE_GROUPS(${src_idlib})

# Normal game code ( Base)
file(GLOB_RECURSE src_game "game/*.cpp")
GENERATE_SOURCE_GROUPS(${src_game})

# Expansion pack code ( d3xp)
file(GLOB_RECURSE src_d3xp "d3xp/*.cpp")
GENERATE_SOURCE_GROUPS(${src_d3xp})

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
GENERATE_SOURCE_GROUPS(${src_core})

set(src_stub_openal		sys/stub/openal_stub.cpp)
set(src_stub_gl			sys/stub/stub_gl.cpp)
set(src_stub_util		sys/stub/util_stub.cpp)

# Begin normal tools code
if (TOOLS AND MFC_FOUND AND MSVC)
	set(ALLOWRESOURCES ON)
	# Common files.
	file(GLOB src_comafx "tools/comafx/*.cpp")
	file(GLOB_RECURSE src_tools_common "tools/common/*.cpp")

	#Compilers.
	file(GLOB src_compiler_renderbump "neo/tools/compilers/renderbump/*.cpp")
	file(GLOB src_compiler_roqvq	  "neo/tools/compilers/roqvq/*.cpp")
	#tools
	# Articulated Frame editor.
	file(GLOB src_afeditor "tools/af/*.cpp")
	# Declaration editor
	file(GLOB src_decleditor "tools/decl/*.cpp")
	# GUI Editor.
	file(GLOB src_gui_editor "tools/guied/*.cpp")
	# Material editor
	file(GLOB src_materialeditor "tools/materialeditor/*.cpp")
	# Particle Editor
	file(GLOB src_particleeditor "tools/particle/*.cpp")
	# PDA editor
	file(GLOB src_pdaeditor "tools/pda/*.cpp")
	# Radiant ( Map editor )
	file(GLOB src_map_editor "tools/radiant/*.cpp")
	# Script editor
	file(GLOB src_script_editor "tools/script/*.cpp")
	# sound editor?
	file(GLOB src_sound_editor "tools/sound/*.cpp")
	

	# The numerous tools in a nice list.
	list(APPEND src_editor_tools
		${src_comafx}
		${src_tools_common}
		${src_compiler_renderbump}
		${src_afeditor}
		${src_decleditor}
		${src_gui_editor}
		${src_materialeditor}
		${src_particleeditor}
		${src_pdaeditor}
		${src_map_editor}
		${src_script_editor}
		${src_sound_editor}
		)
	SET(CMAKE_MFC_FLAG 2)
	set(TOOLS_DEFINES "ID_ALLOW_TOOLS;__AFXDLL")
else()
	# TODO: Add more platforms for tools
	set(src_editor_tools "tools/edit_stub.cpp")
endif()
GENERATE_SOURCE_GROUPS(${src_editor_tools})

if (APPLE)
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
elseif (WIN32)
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

	# Misc resource files.
	if(ALLOWRESOURCES OR TOOLS)
		file(GLOB src_windows_rc "sys/win32/rc/doom.rc" )
	endif()

	set(src_sys_core
		${src_stub_util}
		sys/glimp.cpp
		${src_windows_rc}
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
		${src_stub_util}
		sys/glimp.cpp
	)
endif()

# Make sure the numerous sys sources get sorted out.
GENERATE_SOURCE_GROUPS(
		${src_sys_core}
		${src_sys_base}
		${src_stub_openal}
		${src_stub_gl}
		${src_stub_util}
		)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_library(idlib STATIC ${src_idlib})
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT MINGW)
	set_target_properties(idlib PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

if (CORE)

	add_executable(SteelStorm2 WIN32 MACOSX_BUNDLE
			${src_core}
			${src_sys_base}
			${src_sys_core}
			${src_editor_tools}
		)
	set_target_properties(SteelStorm2 PROPERTIES COMPILE_DEFINITIONS "__DOOM_DLL__;${TOOLS_DEFINES}")
	set_target_properties(SteelStorm2 PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(SteelStorm2 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/sys/osx/Info.plist)

	if (cpu STREQUAL "x86_64")
		set_target_properties(SteelStorm2 PROPERTIES SUFFIX "_64")
	endif()
	
	target_link_libraries(SteelStorm2
		idlib
		#${OPENGL_gl_LIBRARY}
		${OPENAL_LIBRARY}
		${OGG_LIBRARIES}
		${VORBISFILE_LIBRARIES}
		${VORBIS_LIBRARIES}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
		${ZLIB_LIBRARY}
		${SDL_LIBRARY}
		${sys_libs}
	)

	if (NOT APPLE AND NOT WIN32)
		install(TARGETS SteelStorm2
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif ()
endif()

if (DEDICATED)
	add_executable(SteelStorm2ded WIN32 MACOSX_BUNDLE
		${src_core}
		${src_stub_openal}
		${src_stub_gl}
		${src_stub_util}
		${src_sys_base}
	)

	set_target_properties(SteelStorm2ded PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED;__DOOM_DLL__")
	set_target_properties(SteelStorm2ded PROPERTIES LINK_FLAGS "${ldflags}")
	if (MSVC_IDE)
		# set this property to avoid Relese and debug directories.
		# Only applies to executables.
		# set_target_properties (SteelStorm2ded${gamesuffix} PROPERTIES PREFIX "../../../")
	endif()
	
	if (cpu STREQUAL "x86_64")
		set_target_properties(SteelStorm2ded PROPERTIES SUFFIX "_64")
	endif()
	target_link_libraries(SteelStorm2ded
		idlib
		${VORBISFILE_LIBRARIES}
		${VORBIS_LIBRARIES}
		${CURL_LIBRARY}
		${JPEG_LIBRARY}
		${ZLIB_LIBRARY}
		${SDL_LIBRARY}
		${sys_libs}
	)

	if (NOT APPLE AND NOT WIN32)
		install(TARGETS SteelStorm2ded
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif ()
endif()

#if (BASE)
#add_library(base${gamesuffix} SHARED ${src_game})
#set_target_properties(base${gamesuffix} PROPERTIES PREFIX "")
#set_target_properties(base${gamesuffix} PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
##set_target_properties(base${gamesuffix} PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/game")
#set_target_properties(base${gamesuffix} PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR};${CMAKE_SOURCE_DIR};${CMAKE_SOURCE_DIR}/game" )
#set_target_properties(base${gamesuffix} PROPERTIES LINK_FLAGS "${ldflags}")
#set_target_properties(base${gamesuffix} PROPERTIES INSTALL_NAME_DIR "@executable_path")
#target_link_libraries(base${gamesuffix} idlib)
#
#	add_library(base SHARED ${src_game})
#	set_target_properties(base PROPERTIES PREFIX "")
#	set_target_properties(base PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
#	set_target_properties(base PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/game")
#	set_target_properties(base PROPERTIES LINK_FLAGS "${ldflags}")
#	set_target_properties(base PROPERTIES INSTALL_NAME_DIR "@executable_path")
#	target_link_libraries(base idlib)
#
#	if (NOT APPLE AND NOT WIN32)
#		install(TARGETS base
#				RUNTIME DESTINATION "${bindir}"
#				LIBRARY DESTINATION "${libdir}"
#				ARCHIVE DESTINATION "${libdir}"
#		)
#	endif ()
#endif()

if (D3XP)
	add_library(base${gamesuffix} SHARED ${src_d3xp})
	set_target_properties(base${gamesuffix} PROPERTIES PREFIX "")
	set_target_properties(base${gamesuffix} PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
	#set_target_properties(base${gamesuffix} PROPERTIES COMPILE_FLAGS -I\"${CMAKE_SOURCE_DIR}/d3xp\")
	set_target_properties(base${gamesuffix} PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR};${CMAKE_SOURCE_DIR};${CMAKE_SOURCE_DIR}/d3xp" )
	set_target_properties(base${gamesuffix} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(base${gamesuffix} PROPERTIES INSTALL_NAME_DIR "@executable_path")
	target_link_libraries(base${gamesuffix} idlib)
	#add_library(d3xp${gamesuffix} SHARED ${src_d3xp})
	#set_target_properties(d3xp${gamesuffix} PROPERTIES PREFIX "")
	#set_target_properties(d3xp${gamesuffix} PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
	#set_target_properties(d3xp${gamesuffix} PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/d3xp")
	#set_target_properties(d3xp${gamesuffix} PROPERTIES LINK_FLAGS "${ldflags}")
	#set_target_properties(d3xp${gamesuffix} PROPERTIES INSTALL_NAME_DIR "@executable_path")
	#target_link_libraries(d3xp${gamesuffix} idlib)

#	add_library(d3xp SHARED ${src_d3xp})
#	set_target_properties(d3xp PROPERTIES PREFIX "")
#	set_target_properties(d3xp PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
#	set_target_properties(d3xp PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/d3xp")
#	set_target_properties(d3xp PROPERTIES LINK_FLAGS "${ldflags}")
#	set_target_properties(d3xp PROPERTIES INSTALL_NAME_DIR "@executable_path")
#	target_link_libraries(d3xp idlib)

#	if (NOT APPLE AND NOT WIN32)
#		install(TARGETS d3xp
#				RUNTIME DESTINATION "${bindir}"
#				LIBRARY DESTINATION "${libdir}"
#				ARCHIVE DESTINATION "${libdir}"
#		)
#	endif ()
endif()
