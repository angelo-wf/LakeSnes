
# Bugs

The following is a non-exhaustive list of games that have emulation bugs. Note that there are probably more games that are buggy, but these are the ones that have been verified to not work correctly.

(this list consist mostly from games from the [Mesen-S thread](http://forums.nesdev.com/viewtopic.php?f=12&t=18658) on the NesDev Forums, the list of tricky-to-emulate-games from [Sour's Snesdev Wiki](https://snesdev.mesen.ca/wiki/index.php?title=Tricky-to-emulate_games), the [Why accuracy matters-article](https://floating.muncher.se/byuu/accuracy/) by Near and additionally some other games that I happened to have tried that have emulation bugs)

Generally, at least the following general issues seem at play:
- Something with the DSP not releasing notes properly, causing music and sfx to sound off
- Something with the audio-engine used in games developed by Enix causing issues and freezes
- A possible bug in the SPC (or also DSP?) causing some sound-issues that do not occur in SnesJs

## List

- Actraiser
  - Music sounds off (wrong samples)
  - Audio freezes when stating new game, game freezes at following SPC upload
- Actraiser 2
  - Freezes after splash-screen (stuck in SPC upload)
- Aladdin
  - Music and sounds effects sound off (not released properly?)
- Air Strike Patrol
  - Bugs due to not emulating mid-scanline effects
  - Some sound effects sound off, and start mixing into the music
  - Freezes on, or before mission info screen
- Alien vs Predator
  - Flashes black every other frame in-game
- Battle Grand Prix
  - Audio freezes after pressing start, game freezes at following SPC upload
- Breath of fire 2
  - Sound effects sound off (not released properly?)
- Cybernator
  - Sound effects sound off (not released properly?)
- Death Brade
  - Match ends instantly
- F1 Grand Prix
  - HUD is glitchy
- Final Fantasy II
  - Music sounds somewhat off (some instruments)
- Final Fantasy III
  - Freezes after intro (stuck in SPC upload)
- Ghouls 'n Ghosts
  - Music and sound effects sounds off (not released properly?)
- Hiouden - Mamono-tachi to no Chikai
  - Initial music sounds off
  - gets stuck on Japanese warning screen after splash-screen
- Home Alone 2
  - Music and sound effects sound off
- Hook
  - Intros have incorrect colours and glitchy text
- Jumbo Ozaki no Hole in One
  - Weird colour-gradients during course preview
- King of Dragons
  - Music on title screen sounds wrong
- Krusty's Super Fun House
  - Freezes on boot (CPU's execution ends up off the rails)
- Mecarobot Golf
  - Flashing of the ground in-game during movement
  - Sometimes freezes (hard to reproduce, happened once in lesson mode)
- Illusion of Gaia
  - Title screen is silent
  - Freezes when pressing start (stuck in SPC upload)  
- Jurassic Park
  - Sides of screen are not masked properly
- Jurassic Park 2
  - Music sounds off (wrong samples) [does not happen in SnesJS]
- Power Drive
  - Broken graphics on name select screen
  - Cannot move once in-game (?)
- Rendering Ranger R2
  - Freezes on boot (stuck in SPC upload)
- Robocop versus the Terminator
  - Flashes black every other frame in-game
- Sink or Swim
  - Water works incorrectly (fills the screen, breaks scrolling)
  - Music sounds off, sound effects are missing
- Soul Blazer
  - Music sounds off (wrong samples)
- Super Bomberman
  - Options on title-screen have sprite-dropout(?)
- Taz-Mania
  - Audio breaks during gameplay (might cause game freeze at following SPC upload)
- Toy Story
  - Sprite-dropout problems in "Really inside the claw machine" when holding an alien (and some other places) [does not happen in SnesJS]
  - Odd noise when starting a new level, some sound effects keep going during and when exiting levels [does not happen in SnesJS]
- Wild Guns
  - Flashes to incorrect scroll every other frame in-game
