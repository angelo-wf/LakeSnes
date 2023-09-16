
# Bugs

The following is a non-exhaustive list of games that have emulation bugs. Note that there are probably more games that are buggy, but these are the ones that have been verified to not work correctly.

(this list consist mostly from games from the [Mesen-S thread](http://forums.nesdev.com/viewtopic.php?f=12&t=18658) on the NesDev Forums, the lists of tricky-to-emulate-games from [Sour's Snesdev Wiki](https://snesdev.mesen.ca/wiki/index.php?title=Tricky-to-emulate_games) and the [Snesdev wiki](https://snes.nesdev.org/wiki/Tricky-to-emulate_games), the [Why accuracy matters-article](https://floating.muncher.se/byuu/accuracy/) by Near and additionally some other games that I happened to have tried that have emulation bugs)

Generally, at least the following general issues seem at play:
- Something with SPC-CPU sync causing freezes in SPC-upload loops

## List

- Air Strike Patrol
  - Bugs due to not emulating mid-scanline effects
- Death Brade
  - Match ends instantly
- Firepower 2000
  - Garbage line on title screen background
- Hook
  - Intros have incorrect colours and glitchy text
- Mecarobot Golf
  - Some odd flashing of the ground in-game during movement
  - Sometimes freezes (?, hard to reproduce, happened once in lesson mode)
- Jurassic Park
  - Sides of screen are not masked properly
- Power Drive
  - Broken graphics on name select screen
  - Cannot control once in-game, only pause
- Tales of Phantasia
  - Voices sound fine until music starts playing, then everything gets garbled.
    (probably unrelated to spc-upload -- investigating.)
