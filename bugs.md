
# Bugs

The following is a non-exhaustive list of games that have emulation bugs. Note that there are probably more games that are buggy, but these are the ones that have been verified to not work correctly.

(this list consist mostly from games from the [Mesen-S thread](http://forums.nesdev.com/viewtopic.php?f=12&t=18658) on the NesDev Forums, the list of tricky-to-emulate-games from [Sour's Snesdev Wiki](https://snesdev.mesen.ca/wiki/index.php?title=Tricky-to-emulate_games), the [Why accuracy matters-article](https://floating.muncher.se/byuu/accuracy/) by Near and additionally some other games that I happened to have tried that have emulation bugs)

Generally, at least the following general issues seem at play:
- Something with the DSP not releasing notes properly, causing music and sfx to sound off
- Something with the audio-engine used in games developed by Enix causing issues and freezes

## List

- Actraiser 2
  - Freezes after splash-screen (stuck in SPC upload)
- Aladdin
  - Music sound off (notes not released properly)
- Air Strike Patrol
  - Bugs due to not emulating mid-scanline effects
  - Some sound effects sound off, and start mixing into the music
- Battle Grand Prix
  - Audio freezes after pressing start (SPC stuck in upload loop), game freezes at following SPC upload
- Breath of fire 2
  - Sound effects sound off (not released properly?)
- Cybernator
  - Some sound effects keep going after ending
- Death Brade
  - Match ends instantly
- Final Fantasy II
  - Music sounds somewhat off (some instruments)
- Super Ghouls 'n Ghosts
  - Music sounds off (notes not released properly?)
- Hiouden - Mamono-tachi to no Chikai
  - Initial music is missing (?, should there be music?)
  - gets stuck on SPC upload after intro
- Home Alone 2
  - Music and sound effects sound extremely off
- Hook
  - Intros have incorrect colours and glitchy text
- Mecarobot Golf
  - Some odd flashing of the ground in-game during movement
  - Sometimes freezes (?, hard to reproduce, happened once in lesson mode)
- Illusion of Gaia
  - Title screen is silent
  - Freezes when pressing start (stuck in SPC upload)  
- Jurassic Park
  - Sides of screen are not masked properly
- Power Drive
  - Broken graphics on name select screen
  - Cannot control once in-game, only pause
- Rendering Ranger R2
  - Freezes on boot (stuck in SPC upload)
- Soul Blazer
  - Music sounds slightly off (missing drums on title intro)
- Toy Story
  - Some sound effect keep playing when unintended
