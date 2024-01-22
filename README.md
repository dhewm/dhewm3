# ABOUT

_dhewm 3_ is a _Doom 3_ GPL source port, known to work on at least Windows, Linux, macOS and FreeBSD.

The goal of _dhewm 3_ is bring _DOOM 3_ with the help of SDL to all suitable
platforms.

Bugs present in the original _DOOM 3_ will be fixed (when identified) without
altering the original gameplay.

**The official homepage is:** https://dhewm3.org

**Mods supported by dhewm3:** https://dhewm3.org/mods.html

**Mod SDK for dhewm3:**  https://github.com/dhewm/dhewm3-sdk

**The project is hosted at:** https://github.com/dhewm

**Download the latest release:** https://github.com/dhewm/dhewm3/releases/latest

**Consult the FAQ at:** https://github.com/dhewm/dhewm3/wiki/FAQ

**Report bugs here:** https://github.com/dhewm/dhewm3/issues


# CHANGES

Compared to the original _DOOM 3_, the changes of _dhewm 3_ worth mentioning are:

- 64-bit port
- SDL for low-level OS support, OpenGL and input handling
- OpenAL for audio output, all OS-specific audio backends are gone
- OpenAL EFX for EAX reverb effects (read: EAX-like sound effects on all platforms/hardware)
- Gamepad support
- Better support for widescreen (and arbitrary display resolutions)
- A portable build system based on CMake
- (Cross-)compilation with MinGW-w64

See [Changelog.md](./Changelog.md) for a more complete changelog.


# GENERAL NOTES

## Game data and patching

This source release does not contain any game data, the game data is still
covered by the original EULA and must be obeyed as usual.

You must patch the game to the latest version (1.3.1). See the FAQ for details, including
how to get the game data from Steam on Linux or OSX.

Note that the original _Doom 3_ and _Doom 3: Resurrection of Evil_ (together with
_DOOM 3: BFG Edition_, which is *not* supported by dhewm3) are available from the Steam Store at

https://store.steampowered.com/app/208200/DOOM_3/

See https://dhewm3.org/#how-to-install for game data installation instructions.

## Configuration

See [Configuration.md](./Configuration.md) for dhewm3-specific configuration, especially for 
using gamepads.

## Compiling

The build system is based on CMake: http://cmake.org/

Required libraries are not part of the tree. These are:

- zlib
- OpenAL (OpenAL Soft required, Creative's and Apple's versions are made of fail)
- SDL v1.2 or 2.0 (2.0 recommended)
- libcurl (optional, required for server downloads)
- Optionally, on non-Windows: libbacktrace (usually linked statically)
  - sometimes (e.g. on debian-based distros like Ubuntu) it's part of libgcc (=> always available),
    sometimes (e.g. Arch Linux, openSUSE) it's in a separate package
  - If this is available, dhewm3 prints more useful backtraces if it crashes

For **UNIX-like systems**, these libraries need to be installed (including the
developer files). It is recommended to use the software management tools of
your OS (apt, dnf, portage, BSD ports, [Homebrew for macOS](http://brew.sh), ...).

For **Windows**, there are three options:

- Use the provided binaries (recommended, see below)
- Compile these libraries yourself
- Use [vcpkg](https://vcpkg.io/) to install the dependencies
    - Remember to set `CMAKE_TOOLCHAIN_FILE` as described in their [Getting Started Guide](https://vcpkg.io/en/getting-started.html)

Create a distinct build folder outside of this source repository and issue
the cmake command there, pointing it at the neo/ folder from this repository:

`cmake /path/to/repository/neo`

**macOS** users need to point CMake at OpenAL Soft (better solutions welcome):

`cmake -DOPENAL_LIBRARY=/usr/local/opt/openal-soft/lib/libopenal.dylib -DOPENAL_INCLUDE_DIR=/usr/local/opt/openal-soft/include /path/to/repository/neo`

Newer versions of Homebrew install openal-soft to another directory, so use this instead:

`cmake -DOPENAL_LIBRARY="/opt/homebrew/opt/openal-soft/lib/libopenal.dylib" -DOPENAL_INCLUDE_DIR="/opt/homebrew/opt/openal-soft/include" /path/to/repo/neo`

### Compiling example using Ubuntu

Should be the same for Debian and other Debian-derivatives, but apart from the first step (installing
build dependecies) it should be the same on other Linux distros and even other UNIX-likes in general.

Open a terminal and follow these steps:

* Install build dependencies:  
  `sudo apt install git cmake build-essential libsdl2-dev libopenal-dev zlib1g-dev libcurl4-openssl-dev`  
    - The build-essential package on Debian/Ubuntu/... installs some basics for compiling code
      like GCC (incl. g++), GNU Make and the glibc development package
    - Instead of libcurl4-openssl-dev, other libcurl*-dev packages should also work - or none at all, curl is optional.
    - Not strictly necessary, but I recommend making libbacktrace available. On distributions not
      based on Debian, you may have to manually install some kind of libbacktrace development package.
* Use git to get the code from Github (alternatively you can also download the code as an archive and extract that):  
  `git clone https://github.com/dhewm/dhewm3.git`
* Change into the dhewm3 directory, create a directory to build in and change into the build directory:  
  `cd dhewm3` then `mkdir build` then `cd build`
* Create a Makefile with CMake: `cmake ../neo/`
    - You can set different options for CMake with arguments like `-DDEDICATED=ON` (to enable the dedicated server).
      You can show a list of supported options by running `cmake -LH ../neo/`. You can run CMake again
      with another `-DFOO=BAR` option to change that option (previously set options are remembered,
      unless you remove all contents of the build/ dir).
    - You could also install the **cmake-qt-gui** package and run `cmake-gui ../neo/`, which will let
      you configure the build in a GUI instead of using `-D` commandline-arguments.
* Compile dhewm3: `make -j8`
    - `-j8` specifies the number of compiler processes to run in parallel (8 in this example),
      it makes sense to use the number of CPU threads (or cores) in your system.

When all steps are done and no errors occurred, you should be able to run dhewm3 right there, like:  
`./dhewm3 +set fs_basepath /path/to/your/doom3/`  
*Replace `/path/to/your/doom3/` with the path to your Doom3 installation (that contains `base/` with
`pak000.pk4` to `pak008.pk4`)*

### Using the provided Windows binaries

Get a clone of the latest binaries here: https://github.com/dhewm/dhewm3-libs

There are two subfolders:

- 32-bit binaries are located in `i686-w64-mingw32`
- 64-bit binaries are located in `x86_64-w64-mingw32`

Issue the appropriate command from the build folder, for example (for VS2019 and 32bit):

`cmake -G "Visual Studio 16 2019" -A Win32 -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

For 64bit dhewm3 binaries, use `-A x64` and `/path/to/dhewm3-libs/x86_64-w64-mingw32` instead (note that the official dhewm3 binaries for Windows are 32bit).  
For Visual Studio 2022 it's `"Visual Studio 17 2022"`.

For 32bit MinGW use:
`cmake -G "MinGW Makefiles" -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

The binaries are compatible with MinGW-w64 and all MSVC versions.

### Cross-compiling

For cross-compiling, a CMake Toolchain file is required.

For the MinGW-w64 toolchain `i686-w64-mingw32` on Ubuntu 12.04, it looks like:

```
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Then point CMake at your toolchain file:
`cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/Toolchain.cmake -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

If you want to build for x86_64 aka AMD64 aka x64, replace all instances of `i686`
in the toolchain file with `x86_64`.

## Back End Rendering of Stencil Shadows

The Doom 3 GPL source code release **did** not include functionality enabling rendering
of stencil shadows via the "depth fail" method, a functionality commonly known as
"Carmack's Reverse".  
It has been restored in dhewm3 1.5.1 after Creative Labs' [patent](https://patents.google.com/patent/US6384822B1/en)
finally expired.

Note that this did not change the visual appearance of the game, and didn't seem to
make a noticeable performance difference (on halfway-recent hardware) either.

## MayaImport

The code for our Maya export plugin is included, if you are a Maya licensee
you can obtain the SDK from Autodesk.


# LICENSE

See COPYING.txt for the GNU GENERAL PUBLIC LICENSE

ADDITIONAL TERMS:  The Doom 3 GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU GPL which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

EXCLUDED CODE:  The code described below and contained in the Doom 3 GPL Source Code release is not part of the Program covered by the GPL and is expressly excluded from its terms.  You are solely responsible for obtaining from the copyright holder a license for such code and complying with the applicable license terms.

## PropTree

neo/tools/common/PropTree/*

Copyright (C) 1998-2001 Scott Ramsay

sramsay@gonavi.com

http://www.gonavi.com

This material is provided "as is", with absolutely no warranty expressed
or implied. Any use is at your own risk.

Permission to use or copy this software for any purpose is hereby granted
without fee, provided the above notices are retained on all copies.
Permission to modify the code and to distribute modified code is granted,
provided the above notices are retained, and a notice that the code was
modified is included with the above copyright notice.

If you use this code, drop me an email.  I'd like to know if you find the code
useful.

## Base64 implementation

neo/idlib/Base64.cpp

Copyright (c) 1996 Lars Wirzenius.  All rights reserved.

June 14 2003: TTimo <ttimo@idsoftware.com>

modified + endian bug fixes

http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=197039

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.


## IO on .zip files using minizip

src/framework/minizip/*

Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

Modifications of Unzip for Zip64
Copyright (C) 2007-2008 Even Rouault

Modifications for Zip64 support
Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

## MD4 Message-Digest Algorithm

neo/idlib/hashing/MD4.cpp

Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD4 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD4 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

## MD5 Message-Digest Algorithm

neo/idlib/hashing/MD5.cpp

This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

## CRC32 Checksum

neo/idlib/hashing/CRC32.cpp

Copyright (C) 1995-1998 Mark Adler

## stb_image and stb_vorbis

neo/renderer/stb_image.h
neo/sound/stb_vorbis.h

Used to decode JPEG and OGG Vorbis files.

from https://github.com/nothings/stb/

Copyright (c) 2017 Sean Barrett

Released under MIT License and Unlicense (Public Domain)

## Brandelf utility

neo/sys/linux/setup/brandelf.c

Copyright (c) 1996 Søren Schmidt
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer
   in this position and unchanged.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software withough specific prior written permission

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

`$FreeBSD: src/usr.bin/brandelf/brandelf.c,v 1.16 2000/07/02 03:34:08 imp Exp $`

## makeself - Make self-extractable archives on Unix

neo/sys/linux/setup/makeself/*, neo/sys/linux/setup/makeself/README
Copyright (c) Stéphane Peter
Licensing: GPL v2
