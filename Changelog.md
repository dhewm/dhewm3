dhewm3 Changelog
=================

Note: Numbers starting with a "#" like #330 refer to the bugreport with that number
      at https://github.com/dhewm/dhewm3/issues/

1.5.2 (WIP)
------------------------------------------------------------------------

* Gamma and Brightness are now applied in the shaders instead of by setting hardware gamma.  
  Can be disabled (so hardware gamma is used again) with `r_gammaInShaders 0`
* Improvements for (Windows-only) MFC-based tools:
    - Added the script debugger! (thanks *HarrievG*!)  
      Original Doom3 didn't have it (Quake4 did), but the Doom3 GPL source contained
      most of it. *HarrievG* implemented the missing parts and we added some new
      features. It can even be used over the network and while the client part
      (the debugger GUI) is Windows-only, the server can even run on all supported
      platforms, so you can even debug a game running on Linux or macOS, for example.  
      Relevant CVars for network debugging are:
      `com_enableDebuggerServer` and `com_dbgClientAdr` and `com_dbgServerAdr`.  
      To debug the running game on the same PC, just enter `debugger` in the console.
    - All tools can now be built in 64bit (thanks *raynorpat*!)
    - HighDPI support (thanks *HarrievG*!)
    - PDAEditor works now
    - Additional bugfixes
* Cycle through multiple Quicksave slots instead of immediately overwriting the last
  Quicksave. The `com_numQuicksaves` CVar allows setting the number of QuickSaves (#392)
* Make r_locksurfaces work (#357)  
  It doesn't do exactly what its description and name suggests: it renders
  everything that is *currently* visible from the position/view the player had
  when setting `r_locksurfaces 1`. Originally it was supposed to render exactly
  the surfaces that *were* visible then, but I couldn't get that to work.  
  This is pretty similar, but there may be differences with opened doors and such.
* Keyboard input improvements (mostly SDL2-only):
    - Support (hopefully) all keyboard keys on all kinds of keyboard layouts
      by using scancodes for otherwise unknown keys
    - Support typing in non-ASCII characters, if supported by Doom3 (it supports ISO-8859-1)
    - Support the clipboard also on non-Windows platforms  
      You can paste code from the clipboard into the console or other edit fields
      with `Shift+Insert`
    - Explicit support for Right Ctrl, Alt and Shift keys  
      (can be bound to different actions than their left counterparts)
    - Added `in_grabKeyboard` CVar to make sure dhewm3 gets *all* keyboard input  
      Prevents the Windows-key or Alt-Tab or whatever from taking focus from the game
    - Added `in_ignoreConsoleKey` - if set to `1`, the console is only opened with
      Shift+Esc, and the "console key" (that key between Esc, 1 and Tab) can be freely
      bound to an action (and its char can be typed in the console without closing it).
    - Added (SDL2-only) "auto" option for `in_kbd`: When not disabling the console key,
      dhewm3 will try to automatically detect it if `in_kbd` is set to "auto" (now default)
* `s_alReverbGain` CVar to reduce EFX reverb effect intensity (#365)
* Pause (looped) sounds when entering menu (#330)
* Fixes for looped sounds (#390)
* (Optionally) use libbacktrace on non-Windows platforms for more useful
  backtraces in case of crashes (usually linked statically)
* Replace libjpeg with stb_image and libogg/libvorbis(file) with stb_vorbis
    - Now the only required external dependencies should be OpenAL, SDL, zlib
      and optionally libCURL (and of course the C and C++ runtimes)
* Fixed a deadlock (freeze) on Windows when printing messages from another thread
* Fixed some warnings and uninitialized variables (thanks *turol*!)
* Work around dmap bug caused by GCC using FMA "optimizations" (#147)


1.5.1 (2021-03-14)
------------------------------------------------------------------------

* The (Windows-only) integrated **editing tools** of Doom3 are back!
    - They can only be built with non-Express versions of Visual Studio (tested
      Community Editions of VS2013 and VS2017) and can be disabled via CMake
    - Official dhewm3 Windows binaries are built with tools enabled, of course.
    - Only supports 32bit builds, because in contrast to the rest of dhewm3's code,
      the tool code is not 64bit compatible at all.
    - Based on Code from the dhewm3 branch of SteelStorm2, thanks to *Motorsep* for donating that code!
    - Has some bugfixes over the state in Doom3 1.3.1, like selecting a material
      in the Particle Editor doesn't break the viewport of the game any more.
    - Thanks to *Tommy Hanusa* for testing and reporting some issues (that were subsequently fixed)!
* Update savegame format (see #303 and #344)
    - old savegames should still work, but new savegames can't be loaded with older versions of dhewm3!
* Uploaded updated builds of Mod DLLs (esp. Dentonmod should run a lot more stable now).  
  Added Mod DLLs of [LibreCoop](https://www.moddb.com/mods/librecoop-dhewm3-coop)
  and [The Lost Mission](https://www.moddb.com/mods/the-lost-mission).  
  See https://dhewm3.org/mods.html for more details.
* dhewm3 now supports the **Doom3 Demo** gamedata
    - See [below](#using-the-doom3-demo-gamedata) for installation instructions
    - This is based on *Gabriel Cuvillier's* code for [D3Wasm](http://www.continuation-labs.com/projects/d3wasm/),
      which ports dhewm3 to web browsers, thanks!
* Create the game window on the display the cursor is currently on (when using more than one display)
* Added `r_fullscreenDesktop` CVar to set if fullscreen mode should be "classic"
  or "Desktop" which means a borderless window at current desktop resolution
* Fullscreen modes that are not at the current desktop resolution should work better now
    - including nvidia DSR / AMD VSR; for that you might have to use `dhewm3_notools.exe`,
      as DSR/VSR seem to be incompatible with applications that use MFC
      (the GUI framework used for the Doom3 tools like the D3Radiant)
* Several sound-related bugfixes:
    - Lags in starting to play a sound which for example caused the machinegun or
      plasmagun sounds to stutter have been eliminated (#141)
    - Trying to reset disconnected OpenAL devices, this esp. helps with display audio
      on Intel GPUs on Windows, when switching to fullscreen (#209)
    - Looping .wav sounds with leadin now work (#291)
    - The game still works if no sound devices are available at all (#292)
    - Make "idSoundCache: error unloading data from OpenAL hardware buffer" a Warning
      instead of an Error so it doesn't terminate game (by *Corey O'Connor*, #235)
* Restore "Carmack's Reverse" Z-Fail stencil shadows; use `glStencilOpSeparate()` if available
    - That bloody patent finally expired last October: https://patents.google.com/patent/US6384822B1/en
    - This neither seems to make a visual nor performance difference on any hardware
      I tried (including Raspberry Pi 4), so this is mostly out of principle
    - Based on Code by [*Leith Bade*](https://github.com/ljbade/doom3.gpl/commit/d4de024341e79e0ac1dfb54fb528859f8ccea605)
      and [*Pat Raynor*](https://github.com/raynorpat/Doom3/blob/2933cb554587aea546c2df1fdf086204d4ca363d/neo/renderer/draw_stencilshadow.cpp#L147-L182).
    - The `r_useCarmacksReverse` and `r_useStencilOpSeparate` CVars allow switching both things
      on/off for comparison
* New CVar `g_hitEffect`: If set to `0`, the player camera damage effects (like double-vision and extreme tilt)
  when being hit are disabled (by *dobosken*, #279).
* (On Windows) stdout.txt and stderr.txt are not saved next to the binary anymore,
  but in `My Documents/My Games/dhewm3/`, like save games, because the binary dir
  might not be writable and dhewm3 wouldn't start properly then
* Fix lingering messages in HUD after loading savegame
    - Sometimes the "Game saved..." message didn't go away after loading a savegame
      (when having saved while it still was showing from last save)
* Fixed clipping bug in delta1 which sometimes occured and made climbing some
  ladders impossible (#328)
* Improve compatibility with some custom scripts
  ("t->c->value.argSize == func->parmTotal" Assertion; see #303)
* Registering multiplayer servers at id's master-server fixed, so they can be
  found in the multiplayer menu (by *Stradex*, #293)
* Support for reproducible builds by setting the CMake option `REPRODUCIBLE_BUILD`.
* Should build on recent versions of macOS, also on Apple Silicon (thanks *Dave Nicolson* and *Petter Uvesten*).
* Proper handling of paths with dots in directory names (#299, #301)
    - Some string functions that are intended to find/cut off/replace/... file extensions
      did cut off the whole path at dots..
    - Especially fixes loading and saving maps from such paths in the builtin D3Radiant level editor
* `idFileSystemLocal::ListMods()` doesn't search `/` or `C:\` anymore
  (it did so if one of the paths, like `fs_cdpath`, was empty)
* Don't use translation in Autosave filenames (#305)
    - In the Spanish translation all the Alpha Lab autosaves got the same name,
      now the autosave name is based on the mapename instead which is distinct


1.5.0 (2018-12-15)
------------------------------------------------------------------------

* Support for some Mods via [custom SDK](https://github.com/dhewm/dhewm3-sdk):
  Classic Doom3, Fitz Packerton, HardQore2, Denton's Enhanced Doom3 and Rivensin.
    - See https://dhewm3.org/mods.html for more information.
    - This has also broken backwards compatibility with 1.4.x game DLLs,
      that's why this version will be 1.5.0 and not 1.4.2.
* Supports High DPI displays on Windows now
* Scale menus, fullscreen videos and the PDA to 4:3 (with black bars left/right) on  
  widescreen displays so they don't look stretched/distorted.
  Can be disabled with `r_scaleMenusTo43 0`.  
  No, this unfortunately can't be done for the HUD (except for the crosshair),
  because it also handles fullscreen effects (for example when receiving damage),
  and those would look bad with black/empty bars on left/right.
* Commandline option to display some help on supported commandline arguments:
  `-h` or `--help` or `-help` or `/?`
* ~~(Experimental) uncapped framerate, enable by entering `com_fixedTic -1`~~
  ~~in the console (can be set back with `com_fixedTic 0`).~~  
  (this turned out to be broken, see #261)
* Support for the AROS and OpenBSD operating systems
* Several bugfixes


1.4.1 (2016-06-19)
------------------------------------------------------------------------

* Fixed some (kinda rare) crashes due to assertion errors, especially observed in the last
  boss fights of both doom3 and the Resurrection of Evil Addon.
* Improved compatibility with AZERTY keyboards (the row of keys with 1...9, 0 is now usable)
* Fixed a crash (at least on FreeBSD) when loading Resurrection of Evil's last level
* Compatibility with Microsoft Visual Studio 2015
* Video resolutions in menu now sorted, added 2880x1800
* Support for up to 8 mouse buttons (on Linux this needs SDL2 2.0.4 or newer to work)


1.4.0 (2015-10-09)
------------------------------------------------------------------------

The first dhewm3 release. Changes compared to Doom3 1.3.1 as open sourced
on 2011-11-22 (most work done by *dhewg*):

* Use CMake as build system instead of Visual Studio and XCode solutions and SCons etc
* Replaced lots of platform-specific code with libSDL
* Use OpenAL as only soundbackend on all platforms (instead of only on Windows)  
  Ported EAX for sound effects to the cross-platform OpenAL EFX extension
* Made code 64bit compatible (except for Windows-only MFC-based tools, which were disabled
  because no free or at least no-cost compiler with MFC support was available at the time)
* Also made it compatible with the MinGW compiler
* Write savegames, configs, screenshots etc in user-specific directories
  instead of installation directory on all platforms
* Fixed lots of bugs and compiler warnings
* Removed support for binary .pk4's, only support loading .dll/.so/.dylib for
  game-code (mods) directly
* Support (and automatically detect) arbitrary aspect ratios
* Support more resolutions, inject them into the settings menu
* Open ingame-console with Shift+Esc (=> works with all keyboard layouts)
* Most probably much more I forgot...
