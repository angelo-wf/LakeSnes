# LakeSnes
A SNES emulator, in C

## About

This is a SNES emulator, written in C, mostly as a followup on my [earlier Javascript version](https://github.com/elzo-d/SnesJs). The main drive behind rewriting it in C was C's speed. The JS version could barely run at 20 FPS on my system, whereas this C version runs at full speed.

The intent is for it to be formed as a library, which could then be used in other projects. (Maybe it could be compiled for the web with Emscripten as well, to replace the core from that JS emulator).

Performance, although much better than my JS version, is still quite bad though, especially when compared to emulators like BSNES or SNES9X (it used around 80% of one core whereas SNES9X only used around 15%, on my old hardware).

## Compiling

### MacOS

- Make sure `clang` and `make` are available (`xcode-select --install`)
- Download the SDL2 `.dmg` via the releases on the [SDL Github](https://github.com/libsdl-org/SDL)
- Place the `SDL2.framework` found in the disk image in a subdirectory called `sdl2`
- Run `make`

### Linux

- Make sure `gcc` and `make` are available (`sudo apt install build-essential`)
- Get SDL2-dev via the package manager (`sudo apt install libsdl2-dev`)
- Run `make`

Note that these build are dependend on SDL2 being installed / found in `./sdl2`.

Compiling on Windows will most likely require something like Cygwin/Mingw/Msys2, or WSL. This is left as an exercise for the reader (for now).

## Usage and controls

The emulator can be run with `./lakesnes` and takes an optional path to a ROM-file to open. ROM-files can also be dragged on the emulator window to open them. ZIP-files also work, the first file within with a `.smc` or `.sfc` will be loaded (zip support uses [this](https://github.com/kuba--/zip) zip-library, which uses Miniz, both under the Unlicence).

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

| Key | Action             |
| --- | ------------------ |
| R   | Soft reset         |
| E   | Hard reset         |
| P   | Pause              |
| O   | Frame advance      |
| T   | Turbo              |
| L   | Run one CPU cycle  |
| K   | Run one SPC cycle* |
| J   | Dumps some data    |

*: Does not work yet.

Alt+Enter can be used to toggle fullscreen mode.

L will run one CPU cycle, and then logs the CPU state (opcode, registers, flags).
K does not function yet, but is planned to do the same, but for the SPC.

J currently dumps the 128K WRAM, 64K VRAM, 512B CGRAM, 544B OAM and 64K ARAM to a file called `dump.bin`.

Save states and battery saving are not supported yet, but are planned.

Minimizing or hiding the window can cause high CPU usage as this can cause v-sync to stop working.

## Compatibility

The emulator currently only supports regular LoROM and HiROM games (no co-processors and such).
Battery saves are not retained yet. SPC files can also not be loaded yet.

This emulator is definitely not fully accurate. The PPU renders per scanline, so mid-scanline effects are not supported. The DSP executes on a per-sample basis. The SPC is currently still opcode-based (although the SPC core itself is cycle-accurate). The CPU-side timing should be cycle-accurate now, but the exact timing of certain event is still somewhat off. Communication between the CPU and SPC is also not cycle-accurate.

Quite a few TODO's are scattered throughout the code for things that are currently not quite fully emulated, mostly related to edge cases and some lesser-used PPU features.

Some things that are not emulated at all are full emulation-mode for the 65816, and the test-register ($f0) for the SPC.

Some games that I have tested seem to run without obvious issues, although some games do seem to glitch somewhat or freeze. `bugs.md` contains a non-exhaustive list of games that have emulation-bugs.

## License

This project is licensed under the MIT license. See 'LICENSE.txt' for details.

## Resources

- The WDC datasheet for the W65C816S cpu.
- The [65C816 opcodes tutorial](http://6502.org/tutorials/65c816opcodes.html) from 6502.org.
- The [65816 opcode matrix](http://www.oxyron.de/html/opcodes816.html) from oxyron.de (the cycle timing and notes there aren't fully accurate, and the addressing modes IAL and IAX are swapped for JMP (opcodes $7C and $EC)).
- The [SuperFamicon wiki](https://wiki.superfamicom.org).
- The [SnesDev Wiki](https://snesdev.mesen.ca/wiki/index.php?title=Main_Page) by Sour (seems to be abadoned).
- The [SnesDev Wiki](https://snes.nesdev.org/wiki/Main_Page) at NesDev.
- Some quick peeks at Sour's [Mesen-S source](https://github.com/SourMesen/Mesen-S).
- Some quick peeks at Byuu's [Higan source](https://github.com/byuu/higan).
- The [nocach fullsnes document](https://problemkaputt.de/fullsnes.txt).
- Some posts and resources in the [SnesDev section](https://forums.nesdev.org/viewforum.php?f=12) of the NesDev forums.
- The [Super NES Programming pages](https://en.wikibooks.org/wiki/Super_NES_Programming) in Wikibooks.
- Various roms (especially the CPU tests) by PeterLemon/krom found [here](https://github.com/PeterLemon/SNES).
- The 65816 tests by TomHarte found [here](https://github.com/TomHarte/ProcessorTests).
- The SPC700 tests by raddad772 found [here](https://github.com/raddad772/jsmoo/tree/main/misc/tests/GeneratedTests) (part of JSmoo, a JS-based set of emulators; tests also included in TomHarte's repo).
- Various SPC and DSP test by Blargg (from [here](https://forums.nesdev.org/viewtopic.php?f=12&t=10697&p=121027#p121027) and [here](https://forums.nesdev.org/viewtopic.php?f=12&t=18005)).
- The source for the BRR-tools from [SMW central](https://www.smwcentral.net), found [here](https://github.com/jimbo1qaz/BRRtools/tree/32-bit-samples).
- [This](https://github.com/kuba--/zip) zip-library is used for zipped rom loading support.
