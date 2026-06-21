# All Men's Morris

All Men's Morris is an Arduboy / Arduboy FX strategy game built around Nine
Men's Morris and related board variants.

The first playable slice implements local two-player Morris variants with piece
placement, movement, flying with three pieces, mill detection, mandatory capture
after a mill, basic win detection, and sound/RGB feedback. The project is
structured so future modes can add different boards, rule variants, AI, and
campaign-style ideas without mixing rule logic into rendering code.

## Status

Early scaffold. The project compiles and runs, but the full game is not complete
yet. Current controls:

- Main menu up/down: switch between board, ruleset, first-player, and opponent
  settings.
- Main menu left/right: change the selected board, ruleset, first-player
  setting, or opponent setting.
- Main menu B: start the selected playable board/ruleset pairing, cycle the
  selected setting, or show `SOON` for unavailable boards.
- Main menu idle: after 20 seconds without input, enter a silent CPU demo.
- Demo mode: any button returns to the main menu and restarts menu music,
  rotating through short menu themes.
- In game d-pad: move the cursor in that board direction, preferring connected
  neighboring points; press two non-opposite directions together for diagonals.
- B: place/select/confirm/capture.
- Hold A: show quick menu.
- Hold A + left: rewind through the fixed in-RAM game history.
- Hold A + down: return to the main menu, with confirmation.
- Hold A + right: redo through the fixed in-RAM game history.
- Debug build only, hold A + up: cycle test scenarios for mill, flying,
  blocked-game-over, and draw states.

## Development

Enter the development shell:

```sh
nix develop
```

Install Arduino dependencies locally:

```sh
make setup
```

`make setup` pins ArduboyI2C from GitHub because the FX-C link build needs the
newer bus-busy and cable-orientation support that is not present in the older
registry header.

Compile:

```sh
make compile
```

Compile debug:

```sh
make compile-debug
```

Compile the Arduboy FX-C link-cable build:

```sh
make compile-fxc
```

Regenerate firmware board/rule data from `boards/*.json`:

```sh
make board-data
```

Regenerate menu music from the editable TableTop Studio audio source:

```sh
make music-data
```

Run with libretro:

```sh
make libretro
```

Prepare an Arduboy FX catalog entry:

```sh
make fx-entry
```

The entry is prepared under the `TableTop` category by default.

Prepare an Arduboy FX-C catalog entry with link-cable support:

```sh
make fx-entry-fxc
```

Open TableTop Studio:

```sh
make tabletop-studio
```

The Studio loads board profiles from `boards/`, draws and edits the playable
graph overlay, validates connections/mills/rule settings, edits global sprites,
and includes a piano-roll audio editor. `make board-data` converts board
profiles into firmware C++ data; `make music-data` converts the editable menu
music source into `src/MenuMusic.*`.

## Layout

```text
.
|-- all-mens-morris.ino
|-- boards/
|-- src/
|   |-- Assets.*
|   |-- Board.*
|   |-- Rules.*
|   |-- Game.cpp
|   `-- AllMensMorrisGame.h
|-- docs/
|-- tools/tabletop-studio/
|-- tools/board-data/
|-- skills/
|-- nix/
|-- .github/workflows/
|-- flake.nix
`-- Makefile
```

## License

Code and project tooling are released under GPL-3.0-or-later. Art, docs, and
game data are released under CC-BY-SA-4.0. See `LICENSES/README.md`.
