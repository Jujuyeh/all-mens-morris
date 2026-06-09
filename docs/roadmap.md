# Roadmap

## Completed

- Initialized an Arduboy/Nix project scaffold.
- Added classic board topology data.
- Added a first rules state machine.
- Added a playable placement-phase UI.
- Added mandatory capture after forming a mill.
- Added movement, flying, and classic win detection.
- Added a main menu, board-direction cursor navigation, and hold-A quick menu.
- Added sound/RGB feedback and a centered board layout.
- Added debug-only scenarios for fast mill, flying, and blocked-game-over tests.
- Added basic draw detection after 50 movement turns without capture.
- Added a first visual pass for animated cursor, simplified board, HUD, and
  inverted horizontal main-menu board selector.
- Added a custom inverted boot animation that skips the stock Arduboy logo and
  LED startup animation.
- Added `BoardDefinition` and `RuleSet` scaffolding so future variants can
  change board graph data and rule knobs independently.
- Added the first TableTop Studio slice for board profile viewing, graph
  editing, validation, duplication, and JSON editing.
- Added a Six Men's Morris study profile in TableTop Studio.
- Added `make board-data` generation from board profiles into firmware
  `BoardDefinition`/`RuleSet` data.
- Made Six Men's Morris playable from the main menu.
- Added mill behavior as a rule knob so variants can capture on mills or win
  immediately on mills.
- Added a playable Three Men's Morris profile.
- Added Long Morris as a playable custom variant with mixed placement/movement
  rules on the classic board.
- Added Flower as a playable custom 20-point board profile.
- Added a generic CPU opponent that works across generated board/rule profiles.
- Added visible CPU cursor/action animation with turn-paced movement sounds.
- Added CPU Easy and CPU Hard opponent settings, with Hard using depth-2
  minimax over the shared heuristic and weighted top-action choice.
- Added reproducible menu music generation from Mutopia's public-domain MIDI
  source for Scott Joplin's "The Entertainer".
- Added a TableTop Studio piano-roll audio editor for menu music and sound
  effect drafts.

## Next Cycle

1. Test CPU Easy/Hard behavior across Classic, Six Men's Morris, Three Men's Morris, Long
   Morris, and Flower on device/libretro.
2. Keep future CPU tuning under a strict flash/RAM budget so linked multiplayer
   support for two Arduboy FX-C units over USB-C still has room.
3. Consider a second menu music section for attract-mode variety. Measures
   33-64 of the current MIDI source would add about 374 bytes of note/duration
   data plus a few bytes of shared frequency table and playback selection code.
4. Tune board graph/art direction after CPU and human playtests.
5. Add generated profile metadata for menu labels, availability, and future
   variant descriptions.
6. Continue the pixel-art pass for pieces and final board/menu assets.

## Later

- Linked multiplayer between two Arduboy FX-C units, with protocol and UI kept
  small enough to coexist with board variants and CPU difficulty levels.
- More board/rule variant definitions.
- More advanced draw rules, such as repeated-position detection.
- TableTop Studio for designing board sprites and playable graph data together.
- Save/settings support.
- Arduboy FX packaging workflow with banner art.
- Sound effects and optional music.
