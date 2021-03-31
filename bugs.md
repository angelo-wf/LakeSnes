
# Bugs

The following is a non-exhaustive list of games that have emulation bugs. Note that there are probably more games that are buggy, but these are the ones that have been verified to not work correctly.

(this list consist mostly from games from the [Mesen-S thread](http://forums.nesdev.com/viewtopic.php?f=12&t=18658) on the NesDev Forums, the list of tricky-to-emulate-games from [Sour's Snesdev Wiki](https://snesdev.mesen.ca/wiki/index.php?title=Tricky-to-emulate_games), the [why accuracy matters article](https://floating.muncher.se/byuu/accuracy/) by Near and additionally some other games that I happend to have tried that have emulation bugs)

Generally, at least the following general issues seem at play:
- Something with the DSP not releasing notes properly, causing music and sfx to sound off
- Something with the SPC engine used in games developed by Enix causes issues and freezes
- A possible bug in the SPC (or also DSP?) causing some sound-issues that do not occur in SnesJs

## List

- Actraiser
  - Music sounds off (wrong samples)
  - Audio freezes when stating new game, game freezes at following SPC upload
- Aerobiz
  - Mode 7 during intro is completely wrong
- Air Strike Patrol
  - Bugs due to no mid-scanline effects
  - Some sound effects sound off, and start mixing into the music
  - Clearing of text and such is too slow, some other odd glitches
- Battle Blaze
  - Title on title screen is scrolled wrong for a frame every so often (more often after menu shows up)
- Battle Grand Prix
  - Audio freezes after pressing start, game freezes at following SPC upload
- Breath of fire 2
  - Sound effects sound off (not released properly?)
- Cybernator
  - Sound effects sound off (not released properly?)
- Dekikate high school
  - Windowing on title screen is entirely wrong
  - Menu after title screen is mostly invisible (windowing as well?)
- F1 Grand Prix
  - HUD is glitchy
- Final Fantasy II
  - Music sounds somewhat off (some instruments)
- Final Fantasy III
  - Freezes after intro (stuck in SPC upload)
- Home Alone 2
  - Music and sound effects sound off
- Hook
  - Intros have incorrect colours and glitchy text
- Jumbo Ozaki no Hole in One
  - Weird colour-gradients during course preview
- Krusty's Super Fun House
  - Freezes on boot (PC ends up off the rails)
- Mecarobot Golf
  - Flashing of the ground in-game during movement
  - Sometimes freezes (hard to reproduce, happened once in lesson mode)
- Illusion of Gaia
  - Title screen is silent
  - Freezes when pressing start (stuck in SPC upload)  
- Jurassic Park
  - Sides of screen are not masked properly
- Jurassic Park 2
  - Title screen palette seems to be slowly overwritten
  - Music sounds off (wrong samples) [does not happen in SnesJS]
- Sink or Swim
  - Water works incorrectly (fills the screen, breaks scrolling)
  - Music sounds off, sound effects are missing
- Soul Blazer
  - Music sounds off (wrong samples)
- Sparkster
  - Scroll is not updated correctly at the end of stage 1 when scrolling to the boss-area
- Super Bomberman
  - Options on title-screen have sprite-dropout(?)
- Taz-Mania
  - Audio breaks during gameplay (might cause game freeze at following SPC upload)
- Toy Story
  - Scroll is not updated correctly during the two chase levels ("Run Rex, run" and "Roller Bob")
  - Sprite dropout problems in "Really inside the claw machine" when holding an alien (and some other places) [does not happen in SnesJS]
  - Odd noise when starting a new level, some sound effects keep going during and when exiting levels [does not happen in SnesJS]
