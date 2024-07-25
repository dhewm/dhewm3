# Configuration

This document explains some dhewm3-specific configuration options.

For general Doom3 configuration see for example [this list of CVars](https://modwiki.dhewm3.org/CVars_%28Doom_3%29)
and [this list of Console Commands](https://modwiki.dhewm3.org/Commands_%28Doom_3%29).

**CVars** are set by entering `cvarName value` in the console, for example `com_showFPS 1`.  
They can also be set as commandline arguments when starting dhewm3, for example `./dhewm3 +set r_fullscreen 0`.

Just entering a CVar's name (without a value) will show its current value, its default value
and a short description of what it does.

Starting dhewm3 with the commandline argument `-h` (for example `dhewm3.exe -h`) will show some
useful commandline arguments, for example how to tell dhewm3 where the game data can be found on your system.

## dhewm3 Settings Menu

When built with [Dear ImGui](https://github.com/ocornut/imgui) integration (which is the default
when using SDL2), dhewm3 has an advanced settings menu with many settings that the main menu doesn't
offer, including a *Control Bindings* menu that supports as many keys per command as you want and is
fully navigable with keyboard or gamepad (or mouse, of course).

Unless you have already bound your `F10` key to something else, it will open this menu.  
If you `F10` key is already in use, you can either unbind it (in the old Options -> Controls menu),
or open the dhewm3 settings menu by entering `dhewm3Settings` in the console - within that menu,
you can also bind opening it to any other key.

*By the way:* An especially nice feature of the dhewm3 Settings Menu is that you can **open it while
your're in the game**. The game is be paused (if you're playing Single Player) but is still on the
screen, so you **when you modify *Video Options* like *brightness* you can see the effect in realtime**!
Is this cool or what? B-)

Even though *using ImGui menus with a **gamepad*** is quite intuitive (basically use the DPad to navigate
and `A` to select and `B` to cancel; in the *Control Bindings* menu read the *Usage Help* section
for for things specific to that menu), the sheets on https://www.dearimgui.com/controls_sheets/
might be useful for advanced tricks (note however that not all features are available on PC, it seems like
there's no on-screen keyboard and I'm not sure whether controlling the mouse cursor with the PS4/5
touchpad works).

More information for *using ImGui menus with **keyboard and/or mouse*** can be found
[here](https://github.com/ocornut/imgui/blob/v1.90.7/imgui.cpp#L126)

## The Console

Like most id Software games from Quake 1 on, Doom3 has a console that allows entering commands 
and setting Console Variables ("CVars"), often for advanced configuration or to aid development,
see also https://modwiki.dhewm3.org/Console.  

Unlike in original Doom3, in dhewm3 the console is always available (no need to set `com_allowconsole 1`
or similar), and **can be opened with the key combination `Shift + Esc`**.  
The classic "console key" (the one between `Esc`, `Tab` and `1`) should also still work with
most keyboard layouts. However you can disable that, so you can bind that key like any other key
(for example to select the chainsaw), by setting `in_ignoreConsoleKey 1`.

## Using Gamepads

Starting with 1.5.3, dhewm3 supports using gamepads, as long as they're supported by SDL2.  
This includes XBox Controllers (and compatible ones), Playstation 3-5 controllers,
Nintendo Switch Pro Controllers, many thirdparty controllers for those consoles, and lots of other
gamepads for PC.

Some notes:
* By default, no bindings for the gamepad exist, so you need to configure them once in the
  Settings -> Controls menu.
    - You need to bind *Turn Left*, *Turn Right*, *Look Up* and *Look Down* to the corresponding
      directions of one stick to use it to look around or aim.
    - Similarly, you need to bind *Forward*, *Backpedal*, *Move Left* and *Move Right* to the
      corresponding directions of a stick to use it for player movement.
    - However, see below for configs with default bindings that you can load.
* The `Start` button (`+` on Nintendo gamepads, `Options` on Playstation 4/5 controllers) acts
  like the Escape key, so it will **open/close the menu** and can not be bound.  
  The other buttons, axes and triggers can be bound to arbitrary actions in the Controls menu,
  except for the Home button, which can't be used by dhewm3 at all (because it opens Steam when that is running).
* In **menus**, either stick will move the cursor, and the button you assign to *attack* (fire) acts
  like the left mouse button, and so does the lower face button (`A` on XBox controllers, `B` on Nintendo
  controllers, `Cross` on PS controllers) and the upper face button (`Y` on XBox, `X` on Nintendo, `Triangle` on PS).
* The layout of the controller (XBox-like, Nintendo-like, Playstation-like) should be automatically
  detected and is used to display the button names according to the layout. If yours isn't detected
  correctly, you can overwrite it with the `joy_gamepadLayout` CVar.
* Requires SDL2, layout detection requires SDL 2.0.12 or newer.
* Only one gamepad is supported or, more specifically, if multiple are connected, they all behave the same
  and you can't bind their buttons/axes to different actions, and the auto-layout detection will use the
  last gamepad it found to determine the layout.
* You can disable gamepads by setting the `in_useGamepad` CVar to `0`.
* There are several CVars to tweak the behavior:
    - `joy_deadZone` Deadzone of the sticks, where `1.0` would be "stick moved fully in one direction".
       This means that values below this register as 0. If you move or look around ingame even though
       you're not moving a stick, try increasing the `joy_deadZone` value (default is `0.25`).
    - `joy_triggerThreshold` Basically the deadzone for triggers. If your trigger triggers without
       being touched, try increasing this value (default is `0.05`).
    - `joy_gamepadLayout` overwrite automatically detected layout (XBox, Nintendo, PS), see above.
    - `joy_pitchSpeed` How fast you look up/down (when the stick is at a maximum position)
    - `joy_yawSpeed` Same for turning left/right
    - `joy_invertLook` Inverts the controls for looking up/down (like in a flight simulator)
    - `joy_gammaLook` If set to `1`, use a log curve instead of a power curve for looking around,
       affects how fast you turn (or look up/down) when the stick is between center and maximum.
    - `joy_powerScale` If `joy_gammaLook` is `0`, this is the exponent used for the power curve.
    - `joy_dampenLook` if enabled (`1`), somehow reduced the speed of looking around, depending on
      `joy_deltaPerMSLook`.

I created gamepad configs for the base game and d3xp (Resurrection of Evil), based on the standard bindings
of Doom3 BFG, see gamepad.cfg and gamepad-d3xp.cfg in the [base/ directory](./base/).  
Put them in your base/ folder, open the console and enter `exec gamepad.cfg` for the base game,
or `exec gamepad-d3xp.cfg` for Resurrection of Evil (probably also works for Doom3: Lost Mission).

**Alternative gamepad configs** based on the layout of the Doom3 port for the original XBox
are available [**here**](https://github.com/dhewm/dhewm3/issues/536#issuecomment-1928710201).

<details><summary>Click to see what button/stick/trigger does what in gamepad.cfg</summary>

* `Left Stick`: Move (Forward, Backpedal, Move Left, Move Right)
  - `Press Left Stick`: Sprint
* `Right Stick`: Look around (Turn Left, Turn Right, Look Up, Look Down)
  - `Press Right Stick`: Crouch
* `A`: Jump (on Nintendo controllers this is `B`, on Playstation controllers it's `Cross`)
* `X`: Reload (Nintendo: `Y`, Playstation: `Square`)
* `Right Trigger`: Attack
* `Left Trigger`: Flashlight
* `Left Shoulder Button`: Previous Weapon
* `Right Shoulder Button`: Next Weapon
* `DPad Up`: Grenade
* `DPad Down`: BFG
* `DPad Left`: Soulcube
* `DPad Right`: Fists/Grabber
* `Back`: PDA (Nintendo: `-`, Playstation: `Select`/`Share`)
* `Start`: Open/Close Menu (just like the Escape key; Nintendo: `+`, Playstation 4/5: `Options`)

</details>

**_Note_** that in *configs* (or `bind` commands in the console), the following names are used for
gamepad buttons, sticks and triggers:

<details><summary>Click to see the list of gamepad button/stick/trigger names</summary>

* `"JOY_BTN_SOUTH"` - `A` button on XBox-style gamepads, `B` on Nintendo-style gamepads or `Cross` on Playstation-style gamepads 
* `"JOY_BTN_EAST"` - `B` (XBox), `A` (Nintendo), `Circle` (Playstation)
* `"JOY_BTN_WEST"` - `X` (XBox), `Y` (Nintendo), `Square` (Playstation)
* `"JOY_BTN_NORTH"` - `Y` (XBox), `X` (Nintendo), `Triangle` (Playstation)
* `"JOY_BTN_BACK"` - The `Back` button, aka `-` (Nintendo) or `Select`/`Share` (Playstation)
* `"JOY_BTN_LSTICK"` - Pressing the Left Stick down
* `"JOY_BTN_RSTICK"` - Pressing the Right Stick down
* `"JOY_BTN_LSHOULDER"` - Left Shoulder Button
* `"JOY_BTN_RSHOULDER"` - Right Shoulder button
* `"JOY_DPAD_UP"` - DPad Up
* `"JOY_DPAD_DOWN"` - DPad Down
* `"JOY_DPAD_LEFT"` - DPad Left
* `"JOY_DPAD_RIGHT"` - DPad Right
* `"JOY_BTN_MISC1"` - misc. additional button, like Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button
* `"JOY_BTN_RPADDLE1"` - Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1)
* `"JOY_BTN_LPADDLE1"` - Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3)
* `"JOY_BTN_RPADDLE2"` - Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2)
* `"JOY_BTN_LPADDLE2"` - Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4
* `"JOY_STICK1_UP"` - Moving Left Stick up
* `"JOY_STICK1_DOWN"` - Moving Left Stick down
* `"JOY_STICK1_LEFT"` - Moving Left Stick to the left
* `"JOY_STICK1_RIGHT"` - Moving Left Stick to the right
* `"JOY_STICK2_UP"` - Moving Right Stick up
* `"JOY_STICK2_DOWN"` - Moving Right Stick down
* `"JOY_STICK2_LEFT"` - Moving Right Stick to the left
* `"JOY_STICK2_RIGHT"` - Moving Right Stick to the right
* `"JOY_TRIGGER1"` - Pressing the Left Trigger
* `"JOY_TRIGGER2"` - Pressing the Right Trigger

</details>

## Screenshot configuration

Doom3 always supported taking screenshots, but dhewm3 (from 1.5.3 on) supports using different
formats than TGA.  
This can be configured with the following CVars:

- `r_screenshotFormat` What format screenshots should be in:
  `0` = TGA (default), `1` = BMP, `2` = PNG, `3` = JPG
- `r_screenshotJpgQuality` Quality when using JPG screenshots (`1` - `100`). Lower value means smaller
   image file but worse quality (default is `75`)
- `r_screenshotPngCompression` Compression level when using PNG screenshots (`0` - `9`). Higher levels
  generate smaller files, but take noticeably longer; `3` (the default) seems to be a good compromise.

## Other CVars added in dhewm3

- `g_hitEffect` if set to `1` (the default), mess up player camera when taking damage.
   Set to `0` if you don't like that effect.

- `m_invertLook` allows inverting mouse look: `0`: don't invert (this is the default),
  `1`: invert up/down (flight controls), `2`: invert left/right, `3`: invert both directions

- `in_allowAlwaysRunInSP` Allow always run (`in_alwaysRun`) and toggle run (`in_toggleRun`) in
  Single Player as well - keep in mind you may run out of stamina!

- `in_nograb` if set to `1`, the mouse isn't grabbed when ingame. Not overly useful for normal playing
  (unless maybe you play with a gamepad), but very useful for debugging. Default is `0`.
- `in_grabKeyboard` if enabled (`1`), grabs all keyboard input if mouse is grabbed, so keyboard shortcuts
  from the OS like Alt-Tab or Windows Key won't work and thus not accidentally interrupt your playing.
  Defaults to `0`.

- `in_namePressed` if set to `1`, the currently pressed key/button (on keyboard/mouse/gamepad)
  is printed to the console - useful when setting key-bindings in the console or a config. Default is `0`.

- `in_kbd` allows you to set your keyboard layout so the console key works better. Mostly useful with SDL1.2
- `in_tty` tab completion and history for input from the **terminal** (on Unix-likes, like Linux, macOS, BSD, ...)

- `r_fullscreenDesktop` configures fullscreen windows (when `r_fullscreen` is `1`).  
  `0`: "real"/"exclusive" fullscreen mode, might switch screen resolution  
  `1`: "desktop" fullscreen mode, which keeps desktop resolution and is more like a borderless fullscreen window
- `r_windowResizable` if set to `1` (the default), the dhewm3 window (when in windowed mode..)
   can be freely resized. Needs SDL2; with 2.0.5 and newer it's applied immediately, otherwise when
   creating the window (startup or `vid_restart`).
- `r_fillWindowAlphaChan` Make sure alpha channel of windows default framebuffer is completely opaque
  at the end of each frame. Needed at least when using Wayland.  
  `1`: do this, `0`: don't do it, `-1`: let dhewm3 decide (default)

- `r_useSoftParticles` Soften particle transitions when player walks through them or they cross solid geometry.
   Needs r_enableDepthCapture. Can slow down rendering! `1`: enable (default), `0`: disable
- `r_enableDepthCapture` Enable capturing depth buffer to texture. `0`: disable, `1`: enable,  
  `-1`: enable automatically (if soft particles are enabled; the default).   
   This can be used in custom materials with the "_currentDepth" texture.
- `r_useCarmacksReverse` Use Z-Fail ("Carmack's Reverse") when rendering shadows (default `1`)
- `r_useStencilOpSeparate` Use glStencilOpSeparate() (if available) when rendering shadow (default `1`)
- `r_scaleMenusTo43` Render full-screen menus in 4:3 by adding black bars on the left/right if necessary (default `1`)
- `r_glDebugContext` Enable OpenGL debug context and printing warnings/errors from the graphics driver.  
  Changing that CVar requires a `vid_restart` (or set it as startup argument)

- `s_alReverbGain` reduce strength of OpenAL (EAX-like) EFX reverb effects, `0.0` - `1.0` (default `0.5`)
- `s_alHRTF` Enable [HRTF](https://en.wikipedia.org/w/index.php?title=Head-related_transfer_function)
   for better surround sound with stereo **headphones**. `0`: Disable, `1`: Enable, `-1`: Let OpenAL decide (default).  
   *Note* that OpenAL may automatically enable HRTF when it detects headphones, and it can happen that
   it detects regular stereo speakers as headphones (when they're plugged into a jack that's somehow
   labeled as headphone jack) - in that case you'll want to explicitly disable it.
   The *Audio Options* tab of the [dhewm3 Settings Menu](#dhewm3-settings-menu) shows OpenAL Info,
   including the current HRTF state (if supported by your OpenAL version).
- `s_alOutputLimiter` Configure OpenAL's output-limiter which temporarily reduces the overall volume
  when too many too loud sounds play at once, to avoid issues like clipping. `0`: Disable, `1`: Enable, `-1`: Let OpenAL decide (default)
- `s_scaleDownAndClamp` Clamp and reduce volume of all sounds to prevent clipping or temporary downscaling by OpenAL's output limiter (default `1`)

- `imgui_scale` Factor to scale ImGui menus by (especially relevant for HighDPI displays).
  Should be a positive factor like `1.5` or `2`; or `-1` (the default) to let dhewm3 automatically
  detect an appropriate factor.
