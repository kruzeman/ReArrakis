# Project status

ReArrakis targets the verified USA revision of **Dune — The Battle for Arrakis**. It is an experimental native Linux/macOS build, not a finished port or a general-purpose Genesis recompiler.

## Scope recorded in the source project

- SDL2 startup, title/intro, house selection and the first mission.
- A 7,200-frame cold-boot run with synthesized audio and no translation fault.
- The first Atreides mission through victory, including resource collection.
- Mouse selection, orders, construction, production menus and supported dialogs.
- Adaptive map rendering, 50–100% zoom, separate HUD and edge scrolling.
- YM2612/PSG synthesis with the translated Z80 driver and voice-path variants.

The preserved [Russian journal](dune-status.md) records source-project checks and fixture arguments. These observations do not verify the whole campaign.

## Standalone migration verification — 2026-10-07

The cleaned ReArrakis tree builds the supported USA ROM with 40,348 translated
68000 instructions and 3,095 guarded Z80 variants. All 286 retained engine
checks and four ROM-free Dune adapter checks passed. The synthetic demo and an
installed wheel build/run also passed; the wheel includes the required runtime
headers and ymfm license.

Native game fixtures passed for Z80 RET/NOP patches, projectile dispatch and
creation, encoded references, mouse-only front menus and house choices, native
building menus, and adaptive rendering/input. The view fixture covers 30
size/zoom combinations, cursor/HUD separation, zoomed orders and placement.
A 7,200-frame smoke run ended at PC $00118C after 83,059,078 instructions with
no fault. A separate 20-million-instruction synthesized-audio run completed
with 229,726 YM writes and 1,347,522 stereo frames, without a fault.

The new minimap click path has a synthetic SDL event regression check. Actual
minimap gameplay and hardware-dependent fullscreen/audio remain manual checks.
No full campaign completion is claimed. Source ROM data, generated C, sound
templates, executables and test captures remain local in ignored build/.

## macOS verification — 2026-10-07

On Apple Silicon (macOS 27.0, Python 3.14.8, Apple Clang and SDL2 2.32.74),
the unmodified engine passed all 290 retained tests, with the Linux-specific
`/dev/full` check skipped. The synthetic demo builds and runs as a native
Mach-O arm64 executable. Finder-launcher argument forwarding and working
directory handling were checked with a synthetic executable.

The verified USA ROM builds with the existing builder as a native arm64
executable. Local real-ROM checks passed for mouse-only intro/menu navigation,
all three house selections and first-mission construction menus, Z80 RET/NOP
patches, projectile creation and encoded references. The SDL software-renderer
fixture passed all 30 size/zoom combinations, pixel-exact original rendering,
cursor/HUD isolation, zoomed unit orders and building placement.

The existing construction and combat fixtures also passed: windtrap purchase
and placement, Harkonnen combat through enemy destruction, and an Atreides
refinery/harvester run through the original mission-completion transition
at frame 27,360. No credits or victory flags were edited.

A 20-million-instruction headless run with ymfm produced 1,347,522 stereo
frames and 229,726 YM writes without a translation fault or dropped samples.
The budget exit (status 2) is expected. These checks use software rendering
and synthesized audio; accelerated rendering, fullscreen and physical audio
output still require a manual desktop check.

## Passwords and 68000 occupancy — 2026-10-07

The password action table at `$2197C` now seeds all 11 native branches,
fixing the missing translation at `$21A0C` when entering PLAYTESTER.
The USA build contains 40,382 M68K and 3,095 Z80 instructions.
A bounded native fixture entered all 29 passwords through the original
controller-driven password keyboard and verified their handler effects.
It also checked both toggle codes a second time and rejected three invalid
inputs. The fixture invokes the password screen from an initialized first
mission snapshot; this does not verify playing every unlocked mission.

F3 toggles an approximate virtual 68000 occupancy overlay, sampled every
15 frames. It excludes cycles in the original VBlank wait loop at
`$0FDA`/`$0FDE` and halted CPU cycles. Interrupt work counts as occupied;
other unidentified busy waits also count as occupied. This is not host CPU
usage and does not change the console clock or game speed.
The 17 adapter/timing tests passed and the optimized Linux build completed.
The reported X11 Compose warning is separate and has not been fixed here.

## Local macOS app packaging — 2026-10-07

`tools/build_macos_app.py` wraps an existing SDL executable in an ad-hoc
signed app with a Finder launcher that does not open Terminal. It uses
installed SDL libraries and keeps writable files in Application Support.
A synthetic executable checks bundle structure, signing, paths with spaces
and refusal to overwrite an existing app; no ROM is needed for that test.

## Limitations

- Other ROM revisions are rejected.
- Full campaign completion and every player command combination remain unverified.
- Minimap navigation was added in the last source commit and needs manual gameplay verification.
- Bounded rendering and extreme window proportions can limit effective zoom.
- Fullscreen, graphics drivers and audio hardware need testing on users' systems.
- Volume adjustment does not independently rebalance music, effects and voices.
- The standalone builder has no verified Windows packaging yet.
- RROP's host settings, external fonts and full-state save slots are not Dune features.

Report [issues](https://github.com/kruzeman/ReArrakis/issues) with OS, source revision, ROM hash, house/mission and the exact action or diagnostic line. Do not upload game data.

## Debug branch speed telemetry — 2026-10-07

The `debug` branch adds SPD to the F3 overlay: virtual master-clock progress
versus wall time, sampled over at least 0.5 seconds. 100% means real-time
console speed, not a guarantee of smooth original game logic. Pause, stop
and clock rewind reset the sample. This change is published separately
from main at the user's request.

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
## Cursor edge scrolling — 2026-10-07

Edge scrolling follows the visible game cursor bounds instead of a fixed
12-point band around the OS pointer. The free square and snapped grid cursor
use the same world scale as rendering, including drawable-pixel scaling.
The ROM-free SDL regression covers all four edges at 50%, 75% and 100% zoom,
stopping in the interior and cancelling on window leave.

## SDL gamepad settings (debug-derived branch)

Esc/F10 open the pause menu; F1 opens controller settings; keyboard and mouse remain available
regardless of controller bindings. One selected instance supplies game input.
Other devices and their remapping events are ignored. Disconnecting it stops
pad input until the user chooses another device, including with identical models.

Buttons and D-pad directions can be assigned individually or in an eight-step
wizard, including signed axes and triggers. Separate model profiles store
bindings, cursor/digital modes, mouse enable, custom axes, inversion, deadzone,
sensitivity and calibrated centers. Live axis values make drift diagnosable.
Analog input must return to its calibrated center after device selection.
Models sharing an SDL GUID share a profile; active instances remain isolated.

The settings drawing uses a consistent front-view reference of an original
three-button Sega controller. Rendering has no image-library dependency.
Virtual tests cover 21 controllers, inactive-device isolation, disconnect,
manual direction assignment, per-model profiles, calibration and malformed
configuration. Physical Xbox Bluetooth gameplay and the reported cursor drift
still need verification on the user's running app.


### Pause menu and runtime snapshots

The SDL pause menu provides resume, ten save-state slots, controller settings,
fullscreen, volume, debug CPU speed and confirmed quit. Save/load captures the
CPU/device state and uses ymfm's existing FM serialization. Writes use temporary
files plus atomic replacement; reads validate compatibility, lengths, checksums
and selected state bounds before replacing the live machine. Overwrite/load/quit
confirmation starts on Cancel. Slots show presence and modification time.
Snapshots are restricted to a matching ROM/runtime fingerprint/features/ABI;
there is no cross-version migration or thumbnail browser. WAV recording excludes
save/load. Window close remains the standard immediate close operation.
