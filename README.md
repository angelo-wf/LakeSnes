# ElzSnes
A SNES emulator, in C

## About

This is a SNES emulator, written in C, mostly as a followup on my [earlier Javascript version](https://github.com/elzo-d/SnesJs). The main drive behind rewriting it in C was C's speed. The JS version could barely run at 20 FPS on my system, whereas this C version runs at full speed.

The intent is for it to be formed as a library, which could then be used in other projects. (Maybe it could be compiled for the web with Emscripten as well, to replace the core from that JS emulator).

Performance, although much better than my JS version, is still quite bad though, especially when compared to emulators like BSNES or SNES9X (it uses around 80% of one core, whereas SNES9X only uses around 15%).

## Compiling

The Makefile is currently set up for compiling on macOS, provided a copy of `SDL2.framework` is located at `/library/Frameworks/` (can be downloaded from the main SDL2 site) and clang & make are available. With that in place, simply run `make`.

Compiling on Linux will require SDL2 to be available, the Makefile to be modified for including SDL2 the correct way (lines 3 & 4) and possibly changing the `#include` that includes SDL2 in `main.c` (line 8). The used C compiler can also be changed, if needed (Makefile, line 2). Apart from SDL2, no other dependencies are used. After doing that, running `make` should (hopefully) work. Future intent is for this to be possible without needing to modify any files.

Compiling on Windows will most likely require something like Cygwin/Mingw, or WSL. This is left as an exercise for the reader (for now).

## Usage and controls

The emulator can be run with `./elzsnes` and takes an optional path to a ROM-file to open. ROM-files can also be dragged on the emulator window to open them. ZIP-files also work, the first file within with a `.smc` or `.sfc` will be loaded (zip support uses [this](https://github.com/kuba--/zip) zip-library, which uses Miniz, both under the Unlicence).

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

| Key | Action          |
| --- | --------------- |
| R   | Soft reset      |
| E   | Hard reset      |
| P   | Pause           |
| O   | Frame advance   |
| T   | Turbo           |
| L   | Toggle log-mode |
| J   | Dumps some data |

'Log-mode' causes it to log all executed CPU and SPC opcodes, along with register and flag state, while running a limited amount of cycles each frame.
J currently dumps the 128K WRAM, 64K VRAM, 512B CGRAM, 544B OAM and 64K ARAM to a file called `dump.bin`.

Save states are not supported yet, but are planned.

## Compatibility

The emulator currently only supports regular LoROM and HiROM games (no co-processors and such).
Battery saves are not retained yet.

This emulator is definitely not fully accurate. The CPU and SPC are both opcode-based and the 'execute all at once, then stall the correct amount of cycles'-pattern is used a lot. The PPU renders per scanline, so mid-scanline effects are not supported. The DSP executes on a per-sample basis.

Quite a few TODO's are scattered throughout the code for things that are currently not quite fully emulated, mostly related to edge cases, timings, disallowed accesses and some lesser-used PPU features.

Some things that are not emulated at all are full emulation-mode for the 65816, and the test-register ($f0) for the SPC.

Most games that I have tested seem to run without obvious issues, although a few games do seem to glitch somewhat.
