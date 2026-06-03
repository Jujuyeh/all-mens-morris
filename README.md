# All Men's Morris

All Men's Morris is an Arduboy / Arduboy FX strategy game built around Nine
Men's Morris and related board variants.

The first playable slice implements the classic 24-point board, local two-player
piece placement, turn switching, and mill detection. The project is structured
so future modes can add different boards, rule variants, AI, and campaign-style
ideas without mixing rule logic into rendering code.

## Status

Early scaffold. The project compiles and runs, but the full game is not complete
yet. Current controls:

- Left/up: previous point.
- Right/down: next point.
- B: place/select/confirm.
- A: reset the match.

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
