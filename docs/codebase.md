# Codebase Notes

## Current Shape

The sketch entrypoint is intentionally small:

```text
all-mens-morris.ino -> gameSetup() / gameLoop()
```

The implementation is split into:

- `src/Game.cpp`: Arduboy setup, custom boot animation, input, rendering,
  sound/RGB feedback, main menu, quick menu, and current UI.
- `src/Board.*`: `BoardDefinition` data for board graph coordinates, mill
  triples, adjacency, and simple visual line rendering.
- `src/Rules.*`: mutable game state, `RuleSet` configuration, legal actions,
  mill capture rules, flying, win detection, and phase transitions.
- `src/Assets.*`: shared PROGMEM sprites, currently including title and boot
  logo assets.

Startup uses `arduboy.beginDoFirst()` and `arduboy.waitNoButtons()` instead of
`arduboy.begin()` so the stock Arduboy boot logo and LED animation are skipped.
A lightweight custom boot animation runs with inverted Pocket Pixel styling and
keeps the RGB LED off throughout startup.

## Design Constraints

Arduboy has very limited RAM, so board definitions live in `PROGMEM` and runtime
state uses fixed-size arrays. Avoid dynamic allocation in gameplay code.

The rule engine should not know about screen layout. It should operate on point
indices and board data so later variants can swap topology without rewriting the
state machine.

Board and rules data are deliberately separate. Future TableTop Studio output
should generate a board definition from the visual board sprite plus graph
overlay, and a rule set from variant settings such as piece count, flying,
capture protection, material wins, blocked wins, and draw counters.

## Near-Term Refactor Targets

- Separate scene state from match state.
- Add the first non-classic board definition once Classic is stable on the new
  `BoardDefinition`/`RuleSet` contract.
- Add debug-only helpers behind `ALL_MENS_MORRIS_DEBUG`.
- Add save/settings support only after gameplay rules settle.
- Use the TableTop Studio plan in `docs/tabletop-studio.md` when board variant
  data and editor tooling become the next focus.
