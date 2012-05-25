Additional mods

The following mods have been adapted from the original SDK, to facilite using
them in all platforms. Some of them provide game libraries only for Windows,
and others for Windows/Linux/Mac but only for 32 bits.

- Sikkmod 1.2
- Sikkmod 1.1 (used in Doom 3 Evolution)
- Sikkmod 1.1 for Resurrection of Evil
- Classic Doom
- Project Synergy

As there are many identical source files, you may want to use ccache to speed
up the building process. After installing it, run CMake as follows:

$ env PATH="/usr/lib/ccache:$PATH" cmake -DMODS=1 /path/to/repository/neo

This assumes your distribution provides a /usr/lib/ccache directory with
appropiate symlinks. Otherwise you'll have to create one yourself.
