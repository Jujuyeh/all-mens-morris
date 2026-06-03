# All Men's Morris

All Men's Morris is an Arduboy / Arduboy FX strategy game built around Nine
Men's Morris and related board variants.

The first playable slice implements the classic 24-point board, local two-player
piece placement, movement, flying with three pieces, mill detection, mandatory
capture after a mill, basic win detection, and sound/RGB feedback. The project
is structured so future modes can add different boards, rule variants, AI, and
campaign-style ideas without mixing rule logic into rendering code.

## Status

Early scaffold. The project compiles and runs, but the full game is not complete
yet. Current controls:

- Main menu up/down: choose a mode.
- Main menu B: start the selected mode, or toggle a selected setting.
- Main menu First setting: choose whether white or black moves first.
- In game d-pad: move the cursor in that board direction.
- B: place/select/confirm/capture.
- Hold A: show quick menu.
- Hold A + up: reset the match, with confirmation.
- Hold A + left: rewind one game state.
- Hold A + down: return to the main menu, with confirmation.

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

Run with libretro:

```sh
make libretro
```

Prepare an Arduboy FX catalog entry:

```sh
make fx-entry
```

The entry is prepared under the `TableTop` category by default.

## Layout

```text
.
|-- all-mens-morris.ino
|-- src/
|   |-- Assets.*
|   |-- Board.*
|   |-- Rules.*
|   |-- Game.cpp
|   `-- AllMensMorrisGame.h
|-- docs/
|-- skills/
|-- nix/
|-- .github/workflows/
|-- flake.nix
`-- Makefile
```

## License

Code and project tooling are released under GPL-3.0-or-later. Art, docs, and
game data are released under CC-BY-SA-4.0. See `LICENSES/README.md`.
