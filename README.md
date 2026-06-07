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

- Main menu up/down: switch between board selection and first-player setting.
- Main menu left/right: change the selected board or first-player setting.
- Main menu B: start the selected playable board, toggle the first-player
  setting, or show `SOON` for unavailable boards.
- In game d-pad: move the cursor in that board direction.
- B: place/select/confirm/capture.
- Hold A: show quick menu.
- Hold A + up: reset the match, with confirmation.
- Hold A + left: rewind one game state.
- Hold A + down: return to the main menu, with confirmation.
- Hold A + right: toggle `PUT`/`MOVE` in variants that support mixed placement
  and movement.
- Debug build only, hold A + right when no mixed action is available: cycle
  test scenarios for mill, flying, blocked-game-over, and draw states.

## Development

Enter the development shell:

```sh
nix develop
```

Install Arduino dependencies locally:

```sh
make setup
```

Compile:

```sh
make compile
```

Compile debug:

```sh
make compile-debug
```

Regenerate firmware board/rule data from `boards/*.json`:

```sh
make board-data
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

Open TableTop Studio:

```sh
make tabletop-studio
```

The first Studio slice loads board profiles from `boards/`, draws and edits the
playable graph overlay, validates connections/mills/rule settings, and can
create, duplicate, or save profile JSON. `make board-data` converts those
profiles into firmware C++ data for Classic Nine Men's Morris, Long Morris,
Six Men's Morris, and Three Men's Morris.

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
