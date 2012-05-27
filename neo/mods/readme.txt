Additional mods

The following mods have been adapted from the original SDK, to facilite using
them in all platforms. Some of them provide game libraries only for Windows,
and others for Windows/Linux/Mac but only for 32 bits.

Getting the data

To play these mods you also need the data, found in the respective websites.
Then you have to put it where the game library (game*.so) is, or vice-versa.

- Classic Doom 3
  Build subdirectory: classicdoom
  Website: http://www.moddb.com/mods/classic-doom-3

- Doom 3 Evolution (uses Sikkmod 1.1)
  Build subdirectory: skikkmod_v11 (copy as doom3evolution)
  Website: http://www.moddb.com/mods/doom-3-evolution

- Doom 3 HD Mod
  Build subdirectory: sikkmod (copy as doom3hd), sikkmodd3xp (copy as doom3roehd)
  Website: http://www.moddb.com/mods/cverdzislav
  Installing:
    (destination folder << source folder1 + folder2 + ...)
    doom3hd << d3hdm + d3hdmmtr
    doom3roehd << d3xphdm + d3hdmtr
  Running:
    Doom 3: dhewm3 +set fs_game doom3hd
    RoE: dhewm3 +set fs_game_base d3xp +set fs_game doom3roehd

- HardQore2
  Build subdirectory: hardqore2
  Website: http://www.moddb.com/mods/hardqore-2
  Notes: requires "release" build, as some assertions fail

- Project Synergy
  Build subdirectory: synergy
  Website: http://www.moddb.com/mods/project-synergy

- Ruiner
  Build subdirectory: ruiner
  Website: http://www.moddb.com/mods/ruiner
  Notes: requires "release" build, as some assertions fail

- Sikkmod 1.1 for Resurrection of Evil
  Build subdirectory: sikkmodd3xp
  Website: http://www.moddb.com/mods/sikkmod
  Run as: dhewm3 +set fs_game_base d3xp +set fs_game sikkmodxp

- Sikkmod 1.2
  Build subdirectory: skikkmod
  Website: http://www.moddb.com/mods/sikkmod

Compiling

As there are many identical source files, you may want to use ccache to speed
up the building process. After installing it, run CMake as follows:

$ env PATH="/usr/lib/ccache:$PATH" cmake -DMODS=1 /path/to/repository/neo

This assumes your distribution provides a /usr/lib/ccache directory with
appropiate symlinks. Otherwise you'll have to create one yourself.

Troubleshooting

If you experience screen artifacts when using Sikkmod and related mods (in
particular with "motion blur" and "soft shadows"), check if you have
image_downSample set to "1" in your ~/.dhewm3/moddir/dhewm.cfg. If that's the
case, try setting to "0".

This can be caused by video RAM autodetection problems. Another alternative is
to delete your ~/.dhewm3/base/dhewm.cfg and run (just once):
$ dhewm3 +set sys_videoram 512
Then when creating new configurations, this won't happen again. For example:
$ dhewm3 +set fs_game sikkmod
(assumming ~/.dhewm3/sikkmod/dhewm.cfg does not yet exist)

Other non-native mods

In case the source code is available, it can be easily adapted to work with
dhewm3. Otherwise you'll have to use the same Doom 3 version as the available
game library. Fortunately you have two choices:

- For a Linux 32-bit mod, build dhewm3 with the X86 option turned on.
  If you don't have Linux, but an OS with binary compatibility such as
  FreeBSD, you can still run it (but building is not immediate).
  
- For a Windows 32-bit mod, you can cross-compile it (see main readme) and run
  it with Wine. Alternatively you can download a binary. In any case, make
  sure it's for 32-bit.
