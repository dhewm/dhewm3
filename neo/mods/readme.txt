Additional mods

The following mods have been adapted from the original SDK, to facilite using
them in all platforms. Some of them provide game libraries only for Windows,
and others for Windows/Linux/Mac but only for 32 bits.

Getting the data

To play these mods you also need the data, found in the respective websites.
Then you have to put it where the game library (game*.so) is, or vice-versa.

- Sikkmod 1.2
  Build subdirectory: skikkmod
  Website: http://www.moddb.com/mods/sikkmod

- Doom 3 Evolution (uses Sikkmod 1.1)
  Build subdirectory: skikkmod_v11
  Website: http://www.moddb.com/mods/doom-3-evolution

- Sikkmod 1.1 for Resurrection of Evil
  Build subdirectory: sikkmodd3xp
  Website: http://www.moddb.com/mods/sikkmod
  TODO: add notes about running (+set fs_base_game), and test

- Classic Doom 3
  Build subdirectory: classicdoom
  Website: http://www.moddb.com/mods/classic-doom-3

- Project Synergy
  Build subdirectory: synergy
  Website: http://www.moddb.com/mods/project-synergy

TODO: add Doom 3 HD, as well as running instructions
  
Compiling

As there are many identical source files, you may want to use ccache to speed
up the building process. After installing it, run CMake as follows:

$ env PATH="/usr/lib/ccache:$PATH" cmake -DMODS=1 /path/to/repository/neo

This assumes your distribution provides a /usr/lib/ccache directory with
appropiate symlinks. Otherwise you'll have to create one yourself.
