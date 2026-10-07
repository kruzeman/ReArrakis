# Build and controls

[README](../README.md) · [Русская инструкция](usage-ru.md)

## Requirements and build

Linux or macOS, Python 3.10+, C11 and C++17 compilers, pkg-config and SDL2 development files. SDL2_ttf and external fonts are not required. Use your own raw, unswapped 1 MiB USA ROM; the SHA-256 is in [the profile](../profiles/dune-us.json).

Ubuntu / Debian: `sudo apt install git python3 build-essential pkg-config libsdl2-dev`.

Arch Linux: `sudo pacman -S --needed git python base-devel pkgconf sdl2`.

```sh
git clone https://github.com/kruzeman/ReArrakis.git
cd ReArrakis
python3 tools/build_dune.py '/path/to/Dune - The Battle for Arrakis (U) [!].gen'
./run-dune.sh
```

Quote paths containing spaces. The builder verifies the hash before extracting sound-driver templates or translating code. It never downloads a ROM. Windows packaging is not implemented for this standalone builder yet.

Outputs in the ignored `build/` directory:

| File | Purpose |
| --- | --- |
| `dune` | Native executable, with ROM data embedded |
| `dune.c` | Generated C translation |
| `dune.json` | Analysis report and disassembly |
| `dune-z80*.bin` | Sound-driver templates for static analysis |

To generate C without compiling:

```sh
python3 tools/build_dune.py '/path/to/Dune.gen' --emit-only
```

Rebuild after updating source: `git pull` does not update an existing executable. Use the Dune builder rather than the generic CLI; it supplies game-specific discovery roots and sound-driver variants.

## macOS

Install Command Line Tools if Clang is unavailable, and use Homebrew for any
missing Python/SDL2 dependencies:

```sh
xcode-select --install
brew install python pkgconf sdl2-compat
```

An existing SDL2 installation also works. The same builder produces a native
executable for the current Mac; no separate compiler or Python packages are
needed. From the checkout directory:

```sh
python3 tools/build_dune.py '/path/to/Dune - The Battle for Arrakis (U) [!].gen'
./run-dune.command
```

After building, double-click **run-dune.command** in Finder to play. It reuses
the normal launcher and forwards runtime arguments, including `--audio mute`.
Sound, mouse controls, adaptive rendering and zoom use the same code as Linux.
Use **Fn + F11** if the keyboard assigns a system action to F11.

### Finder app without a Terminal window

After the normal SDL build, package it locally:

```sh
python3 tools/build_macos_app.py
open build/ReArrakis.app
```

Double-click `ReArrakis.app` to play without opening Terminal. You can move
it elsewhere on the same Mac. It contains the executable (including your ROM)
and uses the installed SDL libraries; it is not a standalone distribution or
a universal build. Do not upload the app or its game data.

The launcher keeps writable game files and `launch.log` in
`~/Library/Application Support/ReArrakis/`. If startup fails, inspect that log.
The app is ad-hoc signed locally, not notarized. To rebuild, move the old app
aside first, or select a new path with `--output '/path/to/ReArrakis.app'`.
`--binary '/path/to/dune'` packages a different existing native SDL build.

## Controls

| Input | Action |
| --- | --- |
| Left click | Select / confirm / place a building |
| Right click | Order a selected unit; otherwise original B |
| Middle click | Original B / cancel |
| Pointer near a window edge | Scroll; move inward to stop |
| Left click on minimap | Center the expanded overview |
| Mouse wheel | Map zoom, 50–100% in 10% steps |
| 0 | Reset zoom to 100% |
| F11 | Toggle desktop fullscreen |
| F3 | Toggle approximate original 68000 occupancy |
| Arrows | Original directional pad |
| Z / X / C | A / B / C |
| Enter | Start |
| Space | Pause the runtime |
| Tab held | Fast-forward |
| Esc / F10 | Pause menu |
| F1 | Controller settings |

Mouse navigation also works in supported native menus. Original code still applies pricing, construction, selection and movement rules.

The map adapts to the window; HUD elements remain separate. Title screens and menus retain their original proportions. Expanded rendering is bounded to 1024 × 768 source pixels, so extreme aspect ratios may limit effective zoom. Minimap navigation is a recent addition requiring manual gameplay verification.

## Gamepad settings

Press **F1** for controller settings, or **Esc / F10** for the pause menu
and choose **Controller settings**. Fn may be needed on Mac. An unassigned
controller Back/Select button also opens the pause menu. Escape returns one
level; choose
**Quit game** in the pause menu to exit (confirmation required).

- **Pad** selects one SDL-recognized controller by name and number. All others
  are ignored, including their axes and remapping events. Unplugging the selected
  pad stops controller input; choose a replacement explicitly. It never takes
  over automatically. There is no two-controller limit.
- **Input** disables all gameplay input from the pad, while keyboard and mouse
  remain available to recover a bad mapping.
- **Cursor** selects Left, Right or Off; **Axes / Deadzone** can assign custom X/Y
  axes. Analog speed depends on deflection. **Digital** optionally maps a stick
  to native directions; it defaults to Off and cannot use the cursor's axes at
  the same time. **Mouse** separately disables mouse gameplay input and edge
  scrolling; mouse clicks still work in settings.
- **Buttons / D-pad** assigns A, B, C, Start, Up, Down, Left and Right individually
  or with **Assign all eight**. Press Enter/click a row, then press the desired
  button, D-pad direction, trigger or axis direction. Release between presses.
  Escape cancels; Delete/Backspace clears a binding. Assigning an already-used
  control to a single action swaps the two bindings. Menu navigation stays
  independent of gameplay bindings, so it remains usable after remapping.
- **Axes / Deadzone** sets X/Y axes, inversion, deadzone (5–60%) and speed
  (25–300%). Release sticks/triggers and click **Calibrate** to save their current
  centers. RAW and CENTER values show what the selected pad reports. Reset axes
  restores zero centers, 30% deadzone and 100% speed. Sticks must return to their
  calibrated deadzone before newly selected controller input is armed.

D-pad and face buttons navigate settings; analog sticks do not, so a drifting
stick cannot change settings. Keyboard arrows, Enter and mouse are always usable.
Back/Select opens settings only when it has no gameplay binding. OS-reserved
buttons such as Guide may also activate system features.

Default Genesis A/B/C/Start use SDL X/A/B/Start. Configuration is saved after
changes to `gamepad.cfg` (selected device) and `gamepad-<SDL GUID>.cfg` (model
profile). The macOS app stores them in `~/Library/Application Support/ReArrakis/`;
command-line launches use the working directory. Older config versions migrate
on the next save. Identical models with the same GUID share a saved profile;
live input is still isolated by instance ID. Optional `gamecontrollerdb.txt`
mappings are loaded from that directory for devices SDL does not recognize.

## Sound

The builder includes ymfm; the launcher uses `--audio on`.

```sh
DUNE_VOLUME=50 ./run-dune.sh
./run-dune.sh --audio mute
./build/dune --headless --audio on --limit 2000000 --dump-audio build/dune.wav
```

`DUNE_VOLUME` accepts 0–200; 100 is the default, 50 halves that level, and 0 mutes it. The default applies 4× gain to the original signal with limiting. Music, effects and voices do not have independent mix controls.

`mute` keeps Z80 running without synthesis. `stub` disables Z80 execution and is for diagnostics, not normal play.

## Checks and troubleshooting

```sh
make test
make demo
./build/dune --headless --audio mute --limit 2000000 --dump-frame build/frame.ppm
```

Tests and the demo use synthetic data without a game ROM. SDL checks skip if development files are unavailable. After generating `build/dune.c`, run the game smoke fixture:

```sh
cc -std=c11 -O0 tools/smoke_dune.c -o build/smoke-dune
./build/smoke-dune 7200 mute
```

Other fixtures in `tools/` cover intro, combat, construction, rendering, menus and driver patches. Their arguments and earlier results are in the [development journal](dune-status.md). A smoke check does not verify the whole campaign.

`status=budget` means the requested instruction limit was reached. For a translation fault, report the full `execution stopped at ...` / `fault at ...` line, ROM hash, house, mission and triggering action.

Missing SDL2: install its development package. Sound not compiled in: rebuild with `tools/build_dune.py`. A private repository requires GitHub access to clone.

Do not commit or attach ROMs, extracted assets, generated game code, executables or saves.

## CPU occupancy overlay

F3 toggles a small `68K: …%` panel in the top-left corner. The percentage is
virtual 68000 cycles outside Dune's original VBlank polling loop ($0FDA/$0FDE),
averaged over 15 console frames. Interrupt handling counts as work; a halted
CPU counts as waiting. It is an estimate: other busy-wait loops count as work.
This is not host CPU usage, GPU usage or an FPS counter. No console clock,
instruction budget or gameplay speed is changed. The overlay starts disabled.

The F3 debug overlay also shows `SPD: …%`: virtual console time versus
wall-clock time, sampled over at least half a second. Around 100% means
real-time console speed; 50% means half speed. It measures console timing,
not the frequency of game logic updates or unique rendered frames. Original
game-logic slowdowns can still happen at 100%. Paused/stopped displays `--%`.

Debug branch CPU experiment: the executable starts with 2x 68000 throughput.
F4 switches between 1x and 2x; F3 shows the current multiplier (68K: 1/2).
CPU cycles advance peripheral master time by half as much at 2x; VDP, Z80
and audio keep their stock clocks. This changes original CPU timing and can
affect CPU-bound gameplay. SPD remains console wall-clock speed, not CPU
multiplier. This experiment does not belong to main's faithful defaults.

Mouse support now covers native options and the password keyboard: hover
selects a row/cell; left click activates it. On option values, clicking the
left half cycles backwards and the right half forwards. Click keyboard
letters, `<`/`>` and `!` using their original behavior. Right click closes
options/password entry; in the options confirmation dialog, left click
accepts and right click declines. Settings are changed by native handlers.


### Pause menu and save states

**Esc / F10** opens the pause menu; **Resume game** continues immediately.
Escape/B backs out one level, then returns to the previous pause state.
Use arrows or the selected gamepad's D-pad to navigate, Enter/A to select,
or click a row. Left/right adjusts the selected slot, volume or CPU speed;
left/right never confirms loading, overwriting or quitting.

Choose one of **10 state slots**, then **Save state** or **Load state**.
The menu displays whether a slot exists and its last modification time.
Overwrite, load and quit prompts select **Cancel** by default. Loading an empty
slot does nothing. A successful load stays paused until you resume.
The menu also exposes fullscreen, playback volume and the debug CPU speed
(1x original or 2x). Controller remapping remains in its own submenu.

Files are `state-01.grs` through `state-10.grs` in the working directory.
The macOS app stores them in `~/Library/Application Support/ReArrakis/`.
They survive restarting the app and include 68000/Z80, RAM, video, PSG and FM
state. Host input and queued playback audio are cleared when loading.

States are local runtime/ABI snapshots, **not portable emulator saves**:
the loader requires the same ROM, runtime fingerprint, build features and ABI.
A changed runtime build can reject older states; keep the matching app if you
need to keep using them. Checksums detect accidental damage. A rejected load
leaves the running machine untouched, and a failed save does not replace an
existing slot. States cannot be saved/loaded while recording a WAV file.
Volume, fullscreen and the selected slot are session settings.
