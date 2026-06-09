# Codebase Notes

## Current Shape

The sketch entrypoint is intentionally small:

```text
all-mens-morris.ino -> gameSetup() / gameLoop()
```

The implementation is split into:

- `src/Game.cpp`: Arduboy setup, custom boot animation, input, rendering,
  sound/RGB feedback, main menu, quick menu, CPU turn animation/dispatch, and
  current UI.
- `src/Ai.*`: generic CPU opponent. It enumerates legal rule actions, simulates
  them on `MorrisGameState`, and scores the resulting state without knowing
  about screen coordinates. Easy uses the direct heuristic, while Hard reuses
  the same evaluator through a depth-2 minimax pass. `Game.cpp` owns the
  visible cursor animation before applying the chosen result.
- `src/Board.*`: board helper APIs for `BoardDefinition` graph coordinates,
  mill triples, and adjacency. Adjacency is both the visible board connection
  and the legal movement edge.
- `src/Rules.*`: mutable game state, `RuleSet` configuration, legal actions,
  mill capture rules, flying, win detection, and phase transitions.
- `src/GeneratedBoards.*`: generated firmware board and rule data from
  `boards/*.json`, currently Classic Nine Men's Morris, Flower, Long Morris,
  Six Men's Morris, and Three Men's Morris.
- `src/MenuMusic.*`: generated compact menu music arrays from TableTop Studio
  audio data, initially derived from Mutopia's public-domain MIDI source for
  Scott Joplin's "The Entertainer". The MIDI converter emits alternating menu
  themes as one firmware voice, prioritizing the right-hand melody and using
  left-hand bass notes when the melody rests.
- `src/Assets.*`: shared PROGMEM sprites, currently including title and boot
  logo assets.
- `boards/*.json`: editable board/rule profiles consumed by TableTop Studio and
  used as the source for generated firmware board data.
- `tools/tabletop-studio/`: local browser tool for inspecting board graphs,
  validating profiles, and editing JSON board/rule data.
- `tools/board-data/`: JSON validator/generator for `src/GeneratedBoards.*`.
- `tools/music/`: MIDI-to-Arduboy menu music generator, editable audio JSON
  sources, and checked-in public-domain source files.

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

Board and rules data are deliberately separate. `make board-data` generates a
board definition from the graph overlay, and a rule set from variant settings
such as piece count, flying, capture protection, material wins, blocked wins,
reserve-gated block wins, mixed placement/movement, mill behavior, and draw
counters.

## Near-Term Refactor Targets

- Separate scene state from match state.
- Continue moving variant-specific menu/debug behavior onto generated profile
  metadata.
- Keep the CPU opponent generic by adding rule/board helpers instead of
  variant-specific AI branches.
- Add debug-only helpers behind `ALL_MENS_MORRIS_DEBUG`.
- Add save/settings support only after gameplay rules settle.
- Use the TableTop Studio plan in `docs/tabletop-studio.md` when board variant
  data and editor tooling become the next focus.
