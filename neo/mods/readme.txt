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

- Doom 3 HD
  Build subdirectory: sikkmod (copy as doom3hd), sikkmodd3xp (copy as doom3roehd)
  Website: http://www.moddb.com/mods/cverdzislav
  Notes: run as
    "dhewm3 +set fs_game doom3hd" for Doom 3, and as
    "dhewm3 +set fs_game_base d3xp +set fs_game doom3roehd" for RoE

- Project Synergy
  Build subdirectory: synergy
  Website: http://www.moddb.com/mods/project-synergy

- Sikkmod 1.1 for Resurrection of Evil
  Build subdirectory: sikkmodd3xp
  Website: http://www.moddb.com/mods/sikkmod
  Notes: run as "dhewm3 +set fs_game_base d3xp +set fs_game sikkmodxp"

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
to delete your ~/.dhewm3/base/dhewm3.cfg and run (just once):
$ ./dhewm3 +set sys_videoram 512
Then when creating new configurations, this won't happen again.
