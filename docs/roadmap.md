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

## Next Cycle

1. Test Six Men's Morris on device/libretro and tune its graph/art direction.
2. Add a Three Men's Morris profile and decide whether it should be playable.
3. Continue the pixel-art pass for pieces and final board/menu assets.

## Later

- AI opponent.
- Board/rule variant definitions.
- More advanced draw rules, such as repeated-position detection.
- TableTop Studio for designing board sprites and playable graph data together.
- Save/settings support.
- Arduboy FX packaging workflow with banner art.
- Sound effects and optional music.
