cmake_minimum_required(VERSION 2.6...3.22 FATAL_ERROR)
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

option(LINUX_RELEASE_BINS "Set RPATH to \$ORIGIN/libs/ for Linux binary releases" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sys/cmake")

if(LINUX_RELEASE_BINS)
	message(STATUS "Setting RPATH to \$ORIGIN/libs/ so you can put dependencies in there")
	set(CMAKE_SKIP_RPATH OFF CACHE BOOL "Skip RPATH" FORCE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/libs")
	set(ldflags "${ldflags} -Wl,-z,origin") # not sure this is even needed, but shouldn't hurt
else()
	set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
endif()

set(DHEWM3BINARY "dhewm3")

include(CheckCXXCompilerFlag)
include(GNUInstallDirs OPTIONAL RESULT_VARIABLE GNUINSTALLDIRS)

option(CORE			"Build the core" ON)
option(BASE			"Build the base game code" ON)
option(D3XP			"Build the d3xp game code" ON)
if(MSVC)
	option(TOOLS		"Build the tools game code (Visual Studio+SDL2 only)" OFF)
endif()
option(DEDICATED	"Build the dedicated server" OFF)
option(ONATIVE		"Optimize for the host CPU" OFF)
option(SDL2			"Use SDL2 instead of SDL1.2" ON)
option(REPRODUCIBLE_BUILD "Replace __DATE__ and __TIME__ by hardcoded values for reproducible builds" OFF)

option(HARDLINK_GAME "Compile gamecode into executable (no game DLLs)" OFF)

if(NOT MSVC) # GCC/clang or compatible, hopefully
	option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored compiler warnings/errors (GCC/Clang only; esp. useful with ninja)." OFF)
	option(ASAN		"Enable GCC/Clang Adress Sanitizer (ASan)" OFF) # TODO: MSVC might also support this, somehow?
	option(UBSAN	"Enable GCC/Clang Undefined Behavior Sanitizer (UBSan), implies HARDLINK_GAME" OFF)

	if(UBSAN AND NOT HARDLINK_GAME)
		message(STATUS "UBSAN requires linking the gamecode into the executable, will enable HARDLINK_GAME")
		set(HARDLINK_GAME ON)
	endif()
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR)
	message(FATAL_ERROR "No target CPU architecture set")
endif()

if(NOT CMAKE_SYSTEM_NAME)
	message(FATAL_ERROR "No target OS set")
endif()

# target cpu
set(cpu ${CMAKE_SYSTEM_PROCESSOR})

# Originally, ${CMAKE_SYSTEM_PROCESSOR} was supposed to contain the *target* CPU, according to CMake's documentation.
# As far as I can tell this has always been broken (always returns host CPU) at least on Windows
# (see e.g. https://cmake.org/pipermail/cmake-developers/2014-September/011405.html) and wasn't reliable on
# other systems either, for example on Linux with 32bit userland but 64bit kernel it returned the kernel CPU type
# (e.g. x86_64 instead of i686). Instead of fixing this, CMake eventually updated their documentation in 3.20,
# now it's officially the same as CMAKE_HOST_SYSTEM_PROCESSOR except when cross-compiling (where it's explicitly set)
# So we gotta figure out the actual target CPU type ourselves.. (why am I sticking to this garbage buildsystem?)
if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR))
	# special case: cross-compiling, here CMAKE_SYSTEM_PROCESSOR should be correct, hopefully
	# (just leave cpu at ${CMAKE_SYSTEM_PROCESSOR})
elseif(MSVC)
	# because all this wasn't ugly enough, it turned out that, unlike standalone CMake, Visual Studio's
	# integrated CMake doesn't set CMAKE_GENERATOR_PLATFORM, so I gave up on guessing the CPU arch here
	# and moved the CPU detection to MSVC-specific code in neo/sys/platform.h
else() # not MSVC and not cross-compiling, assume GCC or clang (-compatible), seems to work for MinGW as well
	execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpmachine"
	                RESULT_VARIABLE cc_dumpmachine_res
	                OUTPUT_VARIABLE cc_dumpmachine_out)
	if(cc_dumpmachine_res EQUAL 0)
		string(STRIP ${cc_dumpmachine_out} cc_dumpmachine_out) # get rid of trailing newline
		message(STATUS "`${CMAKE_C_COMPILER} -dumpmachine` says: \"${cc_dumpmachine_out}\"")
		# gcc -dumpmachine and clang -dumpmachine seem to print something like "x86_64-linux-gnu" (gcc)
		# or "x64_64-pc-linux-gnu" (clang) or "i686-w64-mingw32" (32bit mingw-w64) i.e. starting with the CPU,
		# then "-" and then OS or whatever - so use everything up to first "-"
		string(REGEX MATCH "^[^-]+" cpu ${cc_dumpmachine_out})
		message(STATUS "  => CPU architecture extracted from that: \"${cpu}\"")
	else()
		message(WARNING "${CMAKE_C_COMPILER} -dumpmachine failed with error (code) ${cc_dumpmachine_res}")
		message(WARNING "will use the (sometimes incorrect) CMAKE_SYSTEM_PROCESSOR (${cpu}) to determine D3_ARCH")
	endif()
endif()

if(cpu STREQUAL "powerpc")
	set(cpu "ppc")
elseif(cpu STREQUAL "aarch64")
	# "arm64" is more obvious, and some operating systems (like macOS) use it instead of "aarch64"
	set(cpu "arm64")
elseif(cpu MATCHES "i.86")
	set(cpu "x86")
elseif(cpu MATCHES "[aA][mM][dD]64" OR cpu MATCHES "[xX]64")
	set(cpu "x86_64")
elseif(cpu MATCHES "[aA][rR][mM].*") # some kind of arm..
	# On 32bit Raspbian gcc -dumpmachine returns sth starting with "arm-",
	# while clang -dumpmachine says "arm6k-..." - try to unify that to "arm"
	if(CMAKE_SIZEOF_VOID_P EQUAL 8) # sizeof(void*) == 8 => must be arm64
		set(cpu "arm64")
	else() # should be 32bit arm then (probably "armv7l" "armv6k" or sth like that)
		set(cpu "arm")
	endif()
endif()

# target os
if(APPLE)
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

add_definitions(-DD3_OSTYPE="${os}" -DD3_SIZEOFPTR=${CMAKE_SIZEOF_VOID_P})

if(MSVC)
	# for MSVC D3_ARCH is set in code (in neo/sys/platform.h)
	message(STATUS "Setting -DD3_SIZEOFPTR=${CMAKE_SIZEOF_VOID_P} -DD3_OSTYPE=\"${os}\" - NOT setting D3_ARCH, because we're targeting MSVC (VisualC++)")
	# make sure ${cpu} isn't (cant't be) used by CMake when building with MSVC
	unset(cpu)
else()
	add_definitions(-DD3_ARCH="${cpu}")
	message(STATUS "Setting -DD3_ARCH=\"${cpu}\" -DD3_SIZEOFPTR=${CMAKE_SIZEOF_VOID_P} -DD3_OSTYPE=\"${os}\" ")

	if(cpu MATCHES ".*64.*" AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
		# tough luck if some CPU architecture has "64" in its name but uses 32bit pointers
		message(SEND_ERROR "CMake thinks sizeof(void*) == 4, but the target CPU ${cpu} looks like a 64bit CPU!")
		message(FATAL_ERROR "If you're building in a 32bit chroot on a 64bit host, switch to it with 'linux32 chroot' or at least call cmake with linux32 (or your OSs equivalent)!")
	endif()
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

if(CMAKE_MAJOR_VERSION LESS 3 OR ( CMAKE_MAJOR_VERSION EQUAL 3 AND CMAKE_MINOR_VERSION LESS 8 ))
	# cmake < 3.8 doesn't support source_group(TREE ...) so replace it with a dummy
	# (it's only cosmetical anyway, to make source files show up properly in Visual Studio)
	function(source_group)
	endfunction()
	message(STATUS "Using CMake < 3.8, doesn't support source_group(TREE ...), replacing it with a dummy")
	message(STATUS "  (this is only relevants for IDEs, doesn't matter for just compiling dhewm3)")
#else()
#	message(STATUS "Using CMake >= 3.8, supports source_group(TREE ...)")
endif()

# libs
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

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

if(REPRODUCIBLE_BUILD)
	# don't use __DATE__ and __TIME__ macros so builds are reproducible
	add_definitions(-DID_REPRODUCIBLE_BUILD)
endif()


find_package(CURL QUIET)
if(CURL_FOUND)
	set(ID_ENABLE_CURL ON)
	include_directories(${CURL_INCLUDE_DIR})
	message(STATUS "libcurl found and enabled")
else()
	message(WARNING "libcurl not found, server downloads won't be available (apart from that dhewm3 will work)")
	set(ID_ENABLE_CURL OFF)
	set(CURL_LIBRARY "")
endif()

set(D3_COMPILER_IS_CLANG FALSE)
set(D3_COMPILER_IS_GCC_OR_CLANG FALSE)

if(MSVC)
	# This is required for tools on windows.
	find_package(MFC)
	
	if(TOOLS AND NOT MFC_FOUND)
		message(SEND_ERROR "MFC ('Microsoft Foundation Classes for C++') couldn't be found, but is needed for TOOLS!")
		message(FATAL_ERROR "If you're using VS2013, you'll also need the 'Multibyte MFC Library for Visual Studio 2013': https://www.microsoft.com/en-us/download/details.aspx?id=40770 (VS2015 and 2017 include that in the default MFC package)")
	endif()

else() # not MSVC

if(NOT WIN32)
	# libbacktrace support - TODO: might work with MinGW? we don't have a crash handler for win32 though..
	include(CheckCSourceCompiles)
	set(CMAKE_REQUIRED_LIBRARIES backtrace)
	check_c_source_compiles( "#include <backtrace.h>
	int main() { backtrace_create_state(NULL, 0, NULL, NULL); return 0; }" HAVE_LIBBACKTRACE )
	unset(CMAKE_REQUIRED_LIBRARIES)

	if(HAVE_LIBBACKTRACE)
		set(sys_libs ${sys_libs} backtrace)
		add_definitions(-DD3_HAVE_LIBBACKTRACE)
		message(STATUS "Using libbacktrace")
	else()
		message(WARNING "libbacktrace wasn't found. It's not required but recommended, because it provides useful backtraces if dhewm3 crashes")
	endif()
endif() # NOT WIN32

# check if this is some kind of clang (Clang, AppleClang, whatever)
# (convert compiler ID to lowercase so we match Clang, clang, AppleClang etc, regardless of case)
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} compiler_id_lower)
if(compiler_id_lower MATCHES ".*clang.*")
	message(STATUS "Compiler \"${CMAKE_CXX_COMPILER_ID}\" detected as some kind of clang")
	set(D3_COMPILER_IS_CLANG TRUE)
	set(D3_COMPILER_IS_GCC_OR_CLANG TRUE)
elseif(CMAKE_COMPILER_IS_GNUCC)
	set(D3_COMPILER_IS_GCC_OR_CLANG TRUE)
endif()
unset(compiler_id_lower)

endif() # not MSVC

# compiler specific flags
if(D3_COMPILER_IS_GCC_OR_CLANG)
	add_compile_options(-pipe)
	add_compile_options(-Wall)

	if(NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_compile_options(-march=native)
	elseif(NOT APPLE AND cpu STREQUAL "x86")
		add_compile_options(-march=pentium3)
	endif()

	if(FORCE_COLORED_OUTPUT)
		if(CMAKE_COMPILER_IS_GNUCC)
		   add_compile_options (-fdiagnostics-color=always)
		elseif (D3_COMPILER_IS_CLANG)
		   add_compile_options (-fcolor-diagnostics)
		endif ()
	endif ()

	set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -D_DEBUG -O0")
	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_PROFILE "-g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -fno-math-errno -fno-trapping-math -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -ggdb -O2 -fno-math-errno -fno-trapping-math -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -fno-math-errno -fno-trapping-math -fomit-frame-pointer")

	set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})
	set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE})

	add_compile_options(-fno-strict-aliasing)
	# dear idiot compilers, don't fuck up math code with useless FMA "optimizations"
	# (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100839)
	add_compile_options(-ffp-contract=off)
	
	if(ASAN)
		# if this doesn't work, ASan might not be available on your platform, don't set ASAN then..
		add_compile_options(-fsanitize=address)
		set(sys_libs ${sys_libs} -fsanitize=address)
	endif()
	if(UBSAN)
		# if this doesn't work, UBSan might not be available on your platform, don't set UBSAN then..
		add_compile_options(-fsanitize=undefined)
		set(sys_libs ${sys_libs} -fsanitize=undefined)
	endif()

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

	CHECK_CXX_COMPILER_FLAG("-Woverloaded-virtual" cxx_has_Woverload_virtual)
	if(cxx_has_Woverload_virtual)
		add_compile_options(-Woverloaded-virtual)
	endif()
	
	# shut up about using memcpy() on classes, in the cases doom3 uses it it seems to be fine
	CHECK_CXX_COMPILER_FLAG("-Wno-class-memaccess" cxx_has_Wno-class-memaccess)
	if(cxx_has_Wno-class-memaccess)
		add_compile_options(-Wno-class-memaccess)
	endif()

	if(AROS)
		set(CMAKE_SHARED_LIBRARY_SUFFIX ".aros-${cpu}")
		add_definitions(-DIOAPI_NO_64)
	elseif(APPLE)
		add_definitions(-DMACOS_X=1)

		if(cpu STREQUAL "x86_64")
			add_compile_options(-arch x86_64 -mmacosx-version-min=10.9)
			set(ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.9")
		elseif(cpu STREQUAL "arm64")
			add_compile_options(-arch arm64 -mmacosx-version-min=11.0)
			set(ldflags "${ldflags} -arch arm64 -mmacosx-version-min=11.0")
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
				add_compile_options(-arch ppc -mone-byte-bool)
				set(ldflags "${ldflags} -arch ppc -mone-byte-bool")
			endif()

			add_compile_options(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		else()
			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
		endif()

		set(sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework IOKit")
	elseif(WIN32)
		# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
		set(ldflags "${ldflags} -static-libgcc -static-libstdc++ -Wl,--stack,8388608")
	elseif(os STREQUAL "linux")
		set(sys_libs ${sys_libs} dl)
	elseif(os STREQUAL "freebsd")
		find_library(EXECINFO_LIBRARIES execinfo /usr/lib /usr/local/lib)
		set(sys_libs ${sys_libs} ${EXECINFO_LIBRARIES})
	endif()
elseif(MSVC)
	add_compile_options(/MP) # parallel build (use all cores, or as many as configured in VS)
	
	add_compile_options(/W3) # TODO: was /W4, caused trouble with VS2019 (and/or its integrated CMake? or only HarrieVG's setup?)
	add_compile_options(/we4840) # treat as error when passing a class to a vararg-function (probably printf-like)
	# treat several kinds of truncating int<->pointer conversions as errors (for more 64bit-safety)
	add_compile_options(/we4306 /we4311 /we4312 /we4302)
	# ignore the following warnings:
	add_compile_options(/wd4100) # unreferenced formal parameter
	add_compile_options(/wd4127) # conditional expression is constant
	add_compile_options(/wd4244) # possible loss of data
	add_compile_options(/wd4245) # signed/unsigned mismatch
	add_compile_options(/wd4267) # possible loss of data
	add_compile_options(/wd4714) # 'function' marked as __forceinline not inlined
	add_compile_options(/wd4996) # 'function': was declared deprecated
	add_compile_options(/wd4068) # unknown pragma
	add_compile_options(/wd4458) # declaration of 'variable' hides class member
	add_definitions(-D_ALLOW_KEYWORD_MACROS) # because of the "#define private public" and "#define protected public" in TypeInfo.cpp
	set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MDd")
	set(CMAKE_C_FLAGS_RELEASE "/Ox /Oy /MD")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /Oy /Zi /MD")
	set(CMAKE_C_FLAGS_MINSIZEREL "/Ox /Oy /Os /MD")
	# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
	set(ldflags "${ldflags} /STACK:8388608")
else()
	message(FATAL_ERROR "Unsupported compiler")
endif()

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
# disable assertions in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG ${CMAKE_C_FLAGS_MINSIZEREL}")

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

if(NOT MSVC)
message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${cpu}")
endif()

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

# I'm a bit sloppy with headers and just glob them in..
# they're only handled in CMake at all so they turn up in Visual Studio solutions..

# globs all the headers from ${PATHPREFIX}/ and adds them to ${SRCLIST}
function(add_globbed_headers SRCLIST PATHPREFIX)
	file(GLOB_RECURSE tmp_hdrs RELATIVE "${CMAKE_SOURCE_DIR}" "${PATHPREFIX}/*.h")
	set(${SRCLIST} ${tmp_hdrs} ${${SRCLIST}} PARENT_SCOPE)
endfunction()

add_globbed_headers(src_renderer "renderer")

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

add_globbed_headers(src_framework "framework")

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

add_globbed_headers(src_cm "cm")

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

add_globbed_headers(src_dmap "tools/compilers/dmap")

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

add_globbed_headers(src_aas "tools/compilers/aas")

set(src_roq
	tools/compilers/roqvq/NSBitmapImageRep.cpp
	tools/compilers/roqvq/codec.cpp
	tools/compilers/roqvq/roq.cpp
	tools/compilers/roqvq/roqParam.cpp
)

add_globbed_headers(src_roq "tools/compilers/roqvq")

set(src_renderbump
	tools/compilers/renderbump/renderbump.cpp
)

add_globbed_headers(src_renderbump "tools/compilers/renderbump")

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

add_globbed_headers(src_snd "sound")

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

add_globbed_headers(src_ui "ui")

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

add_globbed_headers(src_idlib "idlib")

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

add_globbed_headers(src_game "game")

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

add_globbed_headers(src_d3xp "d3xp")

set(src_debuggerServer
	tools/debugger/DebuggerBreakpoint.h
	tools/debugger/DebuggerBreakpoint.cpp
	tools/debugger/DebuggerServer.h
	tools/debugger/DebuggerServer.cpp
	tools/debugger/DebuggerScript.h
	tools/debugger/DebuggerScript.cpp
	tools/debugger/DebuggerMessages.h
	tools/debugger/debugger.cpp
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


set(src_tools
	tools/guied/GEWindowWrapper_stub.cpp
)

# TODO: add_globbed_headers(src_tools "tools/guied")

# Begin normal tools code
if (TOOLS AND MFC_FOUND AND MSVC)
	set(ALLOWRESOURCES ON)
	# Common files.
	file(GLOB src_comafx "tools/comafx/*.cpp")
	add_globbed_headers(src_comafx "tools/comafx")
	file(GLOB_RECURSE src_tools_common "tools/common/*.cpp")
	add_globbed_headers(src_tools_common "tools/common")

	#Compilers.
	# DG: this is always added, why add it again? file(GLOB src_compiler_renderbump "neo/tools/compilers/renderbump/*.cpp")
	file(GLOB src_compiler_roqvq	  "tools/compilers/roqvq/*.cpp") # FIXME: some of those were added before
	add_globbed_headers(src_compiler_roqvq "tools/compilers/roqvq")
	#tools
	# Articulated Frame editor.
	file(GLOB src_afeditor "tools/af/*.cpp")
	add_globbed_headers(src_afeditor "tools/af")
	# Declaration editor
	file(GLOB src_decleditor "tools/decl/*.cpp")
	add_globbed_headers(src_decleditor "tools/decl")
	# GUI Editor.
	file(GLOB src_gui_editor "tools/guied/*.cpp")
	add_globbed_headers(src_gui_editor "tools/guied")
	# Material editor
	file(GLOB src_materialeditor "tools/materialeditor/*.cpp")
	add_globbed_headers(src_materialeditor "tools/materialeditor")
	# Particle Editor
	file(GLOB src_particleeditor "tools/particle/*.cpp")
	add_globbed_headers(src_particleeditor "tools/particle")
	# PDA editor
	file(GLOB src_pdaeditor "tools/pda/*.cpp")
	add_globbed_headers(src_pdaeditor "tools/pda")
	# Radiant ( Map editor )
	file(GLOB src_map_editor "tools/radiant/*.cpp")
	add_globbed_headers(src_map_editor "tools/radiant")
	# Script editor
	file(GLOB src_script_editor "tools/script/*.cpp")
	add_globbed_headers(src_script_editor "tools/script")
	# Script Debugger
	set(src_debuggerClient
		tools/debugger/DebuggerClient.h
		tools/debugger/DebuggerClient.cpp
		tools/debugger/DebuggerApp.h
		tools/debugger/DebuggerApp.cpp
		tools/debugger/DebuggerQuickWatchDlg.h
		tools/debugger/DebuggerQuickWatchDlg.cpp
		tools/debugger/DebuggerWindow.h
		tools/debugger/DebuggerWindow.cpp
		tools/debugger/DebuggerFindDlg.h
		tools/debugger/DebuggerFindDlg.cpp
	)
	set(src_script_debugger
		${src_debuggerServer}
		${src_debuggerClient}
	)
	# sound editor?
	file(GLOB src_sound_editor "tools/sound/*.cpp")
	add_globbed_headers(src_sound_editor "tools/sound")
	

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
		${src_script_debugger}
		"tools/edit_public.h"
		"tools/edit_gui_common.h"
		)
	SET(CMAKE_MFC_FLAG 2)
	set(TOOLS_DEFINES "ID_ALLOW_TOOLS;_AFXDLL")
else()
	set(src_editor_tools "tools/edit_stub.cpp" "tools/edit_public.h" "tools/debugger/debugger_common.h")
	list(APPEND src_editor_tools
		${src_debuggerServer}
	)
endif()


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

	# adding the few relevant headers in sys/ manually..
	set(src_sys_base ${src_sys_base}
		sys/platform.h
		sys/sys_local.h
		sys/sys_public.h
		sys/win32/win_local.h
	)

	set(src_sys_core
		sys/glimp.cpp
	)

	if(TOOLS)
		set(src_sys_core ${src_sys_core} "sys/win32/rc/doom.rc")
	else()
		# adding both .rc files breaks the build, but I think it only contains the icon
		# and doom.rc also contains that (+icons for editor etc, AFAIK), so this is fine
		set(src_sys_core ${src_sys_core} "${CMAKE_SOURCE_DIR}/sys/win32/rc/dhewm3.rc" )
	endif()

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
	if(D3_COMPILER_IS_GCC_OR_CLANG AND NOT MINGW)
		set_target_properties(idlib PROPERTIES COMPILE_FLAGS "-fPIC")
	endif()
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_idlib})

if(HARDLINK_GAME)
	if(BASE)
		set(src_core ${src_core} ${src_game})
		set(game_includes "${CMAKE_SOURCE_DIR}/game")
		if(D3XP)
			message(WARNING "hardlinking the base game, d3xp will not be supported (disable BASE for D3XP support with HARDLINK_GAME)")
		endif()
		
	elseif(D3XP)
		set(src_core ${src_core} ${src_d3xp})
		set(game_includes "${CMAKE_SOURCE_DIR}/d3xp")
	endif()
endif()# HARDLINK_GAME

if(CORE)
	add_executable(${DHEWM3BINARY} WIN32 MACOSX_BUNDLE
		${src_core}
		${src_sys_base}
		${src_sys_core}
		${src_editor_tools}
	)
	
	if(MSVC AND CMAKE_MAJOR_VERSION GREATER 3 OR ( CMAKE_MAJOR_VERSION EQUAL 3 AND CMAKE_MINOR_VERSION GREATER_EQUAL 6 ))
		# CMake >= 3.6 supports setting the default project started for debugging (instead of trying to launch ALL_BUILD ...)
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${DHEWM3BINARY})
	endif()

	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_core} ${src_sys_base} ${src_sys_core} ${src_editor_tools})

if(HARDLINK_GAME)
	set_target_properties(${DHEWM3BINARY} PROPERTIES COMPILE_DEFINITIONS "${TOOLS_DEFINES}")
	target_include_directories(${DHEWM3BINARY} PRIVATE ${game_includes})
else()
	set_target_properties(${DHEWM3BINARY} PROPERTIES COMPILE_DEFINITIONS "__DOOM_DLL__;${TOOLS_DEFINES}")
endif()
	set_target_properties(${DHEWM3BINARY} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(${DHEWM3BINARY} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/sys/osx/Info.plist)

	target_link_libraries(${DHEWM3BINARY}
		idlib
		${OPENAL_LIBRARY}
		${CURL_LIBRARY}
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
		${src_debuggerServer}
	)
	
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_core} ${src_sys_base} ${src_stub_openal} ${src_stub_gl} ${src_debuggerServer})

if(HARDLINK_GAME)
	set_target_properties(${DHEWM3BINARY}ded PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED")
	target_include_directories(${DHEWM3BINARY}ded PRIVATE ${game_includes})
else()
	set_target_properties(${DHEWM3BINARY}ded PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED;__DOOM_DLL__")
endif()
	set_target_properties(${DHEWM3BINARY}ded PROPERTIES LINK_FLAGS "${ldflags}")
	target_link_libraries(${DHEWM3BINARY}ded
		idlib
		${CURL_LIBRARY}
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

if(BASE AND NOT HARDLINK_GAME)
	if (AROS)
		add_executable(base sys/aros/dll/dllglue.c ${src_game})
		set_target_properties(base PROPERTIES OUTPUT_NAME "base.aros-${AROS_ARCH}")
	else()
		add_library(base SHARED ${src_game})
	endif()
	
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_game})
	
	set_target_properties(base PROPERTIES PREFIX "")
	set_target_properties(base PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
	target_include_directories(base PRIVATE "${CMAKE_SOURCE_DIR}/game")
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

if(D3XP AND NOT HARDLINK_GAME)
	if (AROS)
		add_executable(d3xp sys/aros/dll/dllglue.c ${src_d3xp})
		set_target_properties(d3xp PROPERTIES OUTPUT_NAME "d3xp.aros-${AROS_ARCH}")
	else()
		add_library(d3xp SHARED ${src_d3xp})
	endif()
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_d3xp})
	set_target_properties(d3xp PROPERTIES PREFIX "")
	set_target_properties(d3xp PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
	target_include_directories(d3xp PRIVATE "${CMAKE_SOURCE_DIR}/d3xp")
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
