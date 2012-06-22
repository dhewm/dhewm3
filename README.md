# ABOUT

_dhewm 3_ is a _Doom 3_ GPL source modification.

The goal of _dhewm 3_ is bring _DOOM 3_ with the help of SDL to all suitable
plaforms.

Bugs present in the original _DOOM 3_ will be fixed (when identified) without
altering the original gameplay.

**The project is hosted at:** https://github.com/dhewm

**Consult the FAQ at:** https://github.com/dhewm/dhewm3/wiki/FAQ

**Report bugs here:** https://github.com/dhewm/dhewm3/issues


# CHANGES

Compared to the original _DOOM 3_, the changes of _dhewm 3_ worth mentioning are:

- 64bit port
- SDL for low level OS support, OpenGL and input handling
- OpenAL for audio output, all OS specific audio backends are gone
- OpenAL EFX for EAX reverb effects (read: EAX on all platforms)
- A portable build system based on CMake
- (Cross-)compilation with mingw-w64


# GENERAL NOTES

## Game data and patching

This source release does not contain any game data, the game data is still
covered by the original EULA and must be obeyed as usual.

You need the original PK4 files that come under `base/`, as well as the ones
containted in the latest patch. See below for installation details.

If you have the _Resurrection of Evil_ expansion, the PK4 files under `d3xp/`
can be used to play it. The `D3XP` option must also be enabled (see below for
configuration details).

Note that _Doom 3_ and _Doom 3: Resurrection of Evil_ are available from the Steam store at

http://store.steampowered.com/app/9050/

http://store.steampowered.com/app/9070/

## Dependencies

Required libraries are not part of the tree. These are:

- libjpeg (minimum v6, v8 recommended)
- libogg
- libvorbis
- libvorbisfile (may be part of libvorbis)
- OpenAL (OpenAL Soft required, Creative's and Apple's versions are made of fail)
- SDL v1.2
- libcurl (optional, required for server downloads)

For UNIX like system these libraries need to be installed (including the
developer files). It is recommended to use the software management tools of
your OS (apt-get, portage, rpm, BSD ports, MacPorts, ...).

For Windows there are two options:

- Use the provided binaries (recommended, see below)
- Compile these libraries yourself

## Compiling

The build system is based on CMake: http://cmake.org/

Create a distinct build folder outside of this source repository and issue
the cmake command there, pointing it at the neo/ folder from this repository:

`$ cmake /path/to/repository/neo`

OSX users need to point cmake at OpenAL Soft (better solutions welcome):

`$ cmake -DOPENAL_LIBRARY=/usr/local/lib/libopenal.dylib -DOPENAL_INCLUDE_DIR=/usr/local/include /path/to/repository/neo`

For UNIX like systems, run make (possibly with multiple jobs) after that. Like
`$ make -j4`

## Options

In CMake use this syntax to set variables:

`$ cmake -DVAR1=V1 -DVAR2=V2 ... /path/to/repository/neo`

For example:

`$ cmake -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=/usr -DD3XP=1 -DONATIVE=1 /path/to/repository/neo`

### Build type

The following values are valid for the `CMAKE_BUILD_TYPE` variable:

- `prerelease`: with optimizations, debugging symbols and consistency checks (default value)
- `release`: with optimizations, lacks consistency checks (via assertions)
- `minsizerel`: as before, but avoiding optimizations than increase binary size
- `debug`: for development purposes (includes debugging symbols and additional code)
- `profile`: generates profiling information for use with `gprof` in UNIX like systems

### Common

The following are disabled by default.

- `D3XP`: build game library for the _Resurrection of Evil_ expansion
- `MODS`: build game libraries for additional mods (see [this](dhewm3/blob/master/neo/mods/README.md))
- `DEDICATED`: build dedicated server
- `X86`: cross compile for x86 on an x86\_64 machine
- `ONATIVE`: optimize for the host CPU

### Using the provided Windows binaries

Get a clone of the latest binaries here: https://github.com/dhewm/dhewm3-libs

There are two subfolder:

- 32bit binaries are located in "i686-w64-mingw32"
- 64bit binaries are located in "x86\_64-w64-mingw32"

Issue the appropriate command from the build folder, for example:

`$ cmake -G "Visual Studio 10" -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

`$ cmake -G "MinGW Makefiles" -DDHEWM3LIBS=/path/to/dhewm3-libs/x86_64-w64-mingw32 /path/to/repository/neo`

The binaries are compatible with mingw-w64 and all MSVC versions.

Note that you'll need the *DLL* libraries in the `bin` subdirectory
to run the program. They can be copied to the executable directory
for convenience.

### Cross compiling

For cross compiling a CMake Toolchain file is required.

For the mingw-w64 toolchain "i686-w64-mingw32" on Ubuntu precise it looks like:

<pre>
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
</pre>

Then point CMake at your Toolchain file:
`$ cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/Toolchain.cmake -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

### Installation (UNIX)

- `CMAKE_INSTALL_PREFIX`: installation prefix, defaults to `/usr/local`

## Installing

Under UNIX there is an "install" target, so just run
`$ make install`

Then copy your `base/*.pk4` files to `${CMAKE_INSTALL_PREFIX}/share/dhewm3/base`,
and optionally the `d3xp/*.pk4` files to `${CMAKE_INSTALL_PREFIX}/share/dhewm3/d3xp`.

The program will also look for data in the executable directory and current
directory after trying the system installation path, so it can also be used
for user installations. For other cases you can use:

`$ dhewm3 +set fs_basepath /path/to/doom3`

# LEGAL

For license information see [this](dhewm3/blob/master/LEGAL.md).
