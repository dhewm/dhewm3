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
- `r_screenshotJpgQuality` Quality when using JPG screenshots (`0` - `100`)
- `r_screenshotPngCompression` Compression level when using PNG screenshots (`0` - `9`)

## Other CVars added in dhewm3

- `g_hitEffect` if set to `1` (the default), mess up player camera when taking damage.
   Set to `0` if you don't like that effect.

- `in_nograb` if set to `1`, the mouse isn't grabbed when ingame. Not overly useful for normal playing
  (unless maybe you play with a gamepad), but very useful for debugging. Default is `0`.
- `in_grabKeyboard` if enabled (`1`), grabs all keyboard input if mouse is grabbed, so keyboard shortcuts
  from the OS like Alt-Tab or Windows Key won't work and thus not accidentally interrupt your playing.
  Defaults to `0`.

- `in_kbd` allows you to set your keyboard layout so the console key works better. Mostly useful with SDL1.2
- `in_tty` tab completion and history for input from the **terminal** (on Unix-likes, like Linux, macOS, BSD, ...)

- `r_fullscreenDesktop` `0`: "real"/"exclusive" fullscreen mode, might switch screen resolution  
  `1`: "desktop" fullscreen mode, which keeps desktop resolution and is more like a borderless fullscreen window
- `r_fillWindowAlphaChan` Make sure alpha channel of windows default framebuffer is completely opaque
  at the end of each frame. Needed at least when using Wayland.  
  `1`: do this, `0`: don't do it, `-1`: let dhewm3 decide (default)

- `r_useCarmacksReverse` Use Z-Fail ("Carmack's Reverse") when rendering shadows (default `1`)
- `r_useStencilOpSeparate` Use glStencilOpSeparate() (if available) when rendering shadow (default `1`)

- `s_alReverbGain` reduce strength of OpenAL (EAX-like) EFX reverb effects, `0.0` - `1.0` (default `0.5`)
