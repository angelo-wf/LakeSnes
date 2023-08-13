# LakeSnes
A SNES emulator, in C

## About

This is a SNES emulator, written in C, mostly as a followup on my [earlier Javascript version](https://github.com/angelo-wf/SnesJs). The main drive behind rewriting it in C was C's speed. The JS version could barely run at 20 FPS on my system, whereas this C version runs at full speed.

The intent is for the actual emulation itself to be split off into a library, which can then be used in other projects. (Maybe it could be compiled for the web with Emscripten as well, to replace the core from that JS emulator). This is not done yet, and as of now a full emulator with basic frontend (using [SDL2](https://www.libsdl.org)) is build.

Performance, although much better than my JS version, is still quite bad though, especially when compared to emulators like BSNES or SNES9X (it used around 80% of one core whereas SNES9X only used around 15%, on my old hardware).

## Compiling

### MacOS (plain executable)

- Install [homebrew](https://brew.sh) (This also install the Xcode CLI-tools, providing clang and make)
- Install SDL2 with homebrew: `brew install sdl2`
- Run `make`

This build depends on SDL2 being installed with homebrew.

### MacOS (app-bundle)

- Make sure clang and make are available (from the Xcode CLI-tools)
- Download the latest SDL2 build (the `.dmg`) from the [SDL2 Releases](https://github.com/libsdl-org/SDL/releases)
- Create a directory called `sdl2` in this repo's directory and copy `SDL2.framework` from the dmg into it
- Run `make LakeSnes.app`

This is a stand-alone application and shows up in the 'open with' menu for smc, sfc and zip files.

### Linux

- Make sure clang (or gcc) and make are available
- Get SDL2-dev via the package manager (Ubuntu/Debian: `sudo apt install libsdl2-dev`)
- Run `make` (or `make CC=gcc` to use gcc)

This build depends on SDL2 being installed.

### Windows

NOTE: Only tested with Msys2 and clang, but gcc should work as well, and using other environments (Cygwin, Mingw, etc) or Visual Studio might also be possible.

- Install [Msys2](https://www.msys2.org)
- Open a `clang64` environment (and run `pacman -Suy`)
- Install clang and make: `pacman -S mingw-w64-clang-x86_64-clang mingw-w64-clang-x86_64-make`
- Install SDL2: `pacman -S mingw-w64-clang-x86_64-SDL2`
- Navigate to this repo's directory (`cd /c/Users/...`)
- Run `mingw32-make lakesnes.exe` (or `mingw32-make` to build iconless exe)
- Download the latest SDL2 build (the `...-win32-x64.zip`) from the [SDL2 Releases](https://github.com/libsdl-org/SDL/releases)
- Copy `SDL2.dll` from it into this repo's directory (next to the build `lakesnes.exe`)

This build depends on `SDL2.dll` being placed next to the executable.

## Usage and controls

The emulator can be run by opening `lakesnes` directly or by running `./lakesnes`, taking an optional path to a ROM-file to open. ROM-files can also be dragged on the emulator window to open them. ZIP-files also work, the first file within with a `.smc` or `.sfc` will be loaded (zip support uses [this](https://github.com/kuba--/zip) zip-library, which uses Miniz, both under the Unlicence).

Currently, only normal joypads are supported, and only controller 1 has controls set up.

| Button | Key         |
| ------ | ----------- |
| Up     | Up arrow    |
| Down   | Down arrow  |
| Left   | Left arrow  |
| Right  | Right arrow |
| Start  | Enter       |
| Select | Right shift |
| A      | X           |
| B      | Z           |
| X      | S           |
| Y      | A           |
| L      | D           |
| R      | C           |

Additionally, the following command are available:

| Key | Action            |
| --- | ----------------- |
| R   | Soft reset        |
| E   | Hard reset        |
| P   | Pause             |
| O   | Frame advance     |
| T   | Turbo             |
| L   | Run one CPU cycle |
| K   | Run one SPC cycle |
| J   | Dumps some data   |

Alt+Enter can be used to toggle fullscreen mode.

L will run one CPU cycle, and then logs the CPU state (opcode, registers, flags).
K does the same, but for the SPC instead (note that this acts as additional SPC cycles).

J currently dumps the 128K WRAM, 64K VRAM, 512B CGRAM, 544B OAM and 64K ARAM to a file called `dump.bin`.

Save states and battery saving are not supported yet, but are planned.

Minimizing or hiding the window can cause high CPU usage as this can cause v-sync to stop working.

## Compatibility

The emulator currently only supports regular LoROM and HiROM games (no co-processors and such).
Battery saves are not retained yet. SPC files can also not be loaded yet.

This emulator is definitely not fully accurate. The PPU renders per scanline, so mid-scanline effects are not supported. The DSP executes on a per-sample basis. The SPC and CPU-side timing should be cycle-accurate now, but the exact timing of certain event is still somewhat off. Communication between the CPU and SPC is also not cycle-accurate.

Quite a few TODO's are scattered throughout the code for things that are currently not quite fully emulated, mostly related to edge cases and some lesser-used PPU features.

Some things that are not emulated at all are full emulation-mode for the 65816, and the test-register ($f0) for the SPC.

Some games that I have tested seem to run without obvious issues, although some games do seem to glitch somewhat or freeze. `bugs.md` contains a non-exhaustive list of games that have emulation-bugs.

## License

This project is licensed under the MIT license, see 'LICENSE.txt' for details.

It uses 'kuba--/zip' which is under the Unlicense, and links against SDL2 which is under the zlib license.

## Resources

- The WDC datasheet for the W65C816S cpu.
- The [65C816 opcodes tutorial](http://6502.org/tutorials/65c816opcodes.html) from 6502.org.
- The [65816 opcode matrix](http://www.oxyron.de/html/opcodes816.html) from oxyron.de (the cycle timing and notes there aren't fully accurate, and the addressing modes IAL and IAX are swapped for JMP (opcodes $7C and $EC)).
- The [SuperFamicon wiki](https://wiki.superfamicom.org).
- The [SnesDev Wiki](https://snesdev.mesen.ca/wiki/index.php?title=Main_Page) by Sour (seems to be abadoned).
- The [SnesDev Wiki](https://snes.nesdev.org/wiki/Main_Page) at NesDev.
- Some quick peeks at Sour's [Mesen-S source](https://github.com/SourMesen/Mesen-S), nuw superseeded by [Mesen 2](https://github.com/SourMesen/Mesen2).
- Some quick peeks at [Higan's source](https://github.com/higan-emu/higan), [Bsnes' source](https://github.com/bsnes-emu/bsnes) and [Ares' source](https://github.com/ares-emulator/ares), all formely by Near.
- The [nocach fullsnes document](https://problemkaputt.de/fullsnes.txt).
- Some posts and resources in the [SnesDev section](https://forums.nesdev.org/viewforum.php?f=12) of the NesDev forums.
- The [Super NES Programming pages](https://en.wikibooks.org/wiki/Super_NES_Programming) in Wikibooks.
- Anomie's docs from [Romhacking.net](https://www.romhacking.net/community/548/).
- Various roms (especially the CPU tests) by PeterLemon/krom found [here](https://github.com/PeterLemon/SNES).
- The 65816 tests by TomHarte found [here](https://github.com/TomHarte/ProcessorTests).
- The SPC700 tests by raddad772 found [here](https://github.com/raddad772/jsmoo/tree/main/misc/tests/GeneratedTests) (part of JSmoo, a JS-based set of emulators; tests also included in TomHarte's repo).
- Various SPC and DSP test by Blargg (from [here](https://forums.nesdev.org/viewtopic.php?f=12&t=10697&p=121027#p121027) and [here](https://forums.nesdev.org/viewtopic.php?f=12&t=18005)).
- The source for the BRR-tools from [SMW central](https://www.smwcentral.net), found [here](https://github.com/jimbo1qaz/BRRtools/tree/32-bit-samples).
- [SDL2](https://www.libsdl.org) is used for the frontend window/rendering/audio/input handling.
- [This](https://github.com/kuba--/zip) zip-library is used for zipped rom loading support.
