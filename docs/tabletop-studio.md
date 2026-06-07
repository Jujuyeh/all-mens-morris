# TableTop Studio Plan

## Goal

Copy the useful shape of Pocket Pixel's Pet Studio into this project as a local
browser tool named TableTop Studio. Instead of editing pet personality profiles,
the tool should edit Morris-family board profiles.

Each board profile should describe:

- the board connections players see and pieces use for legal movement;
- the mill triples that define scoring/capture patterns;
- optional sprite/banner assets for richer presentation.

The important constraint is that graph and rules data stay explicit. Board
connections are both visual lines and movement edges. Mill lines remain separate
because custom boards may have scoring triples that are not obvious from simple
geometry.

## Pocket Pixel Pieces To Reuse

Use Pocket Pixel's `tools/pet-studio/` as the starting point for:

- the local browser editor structure;
- profile loading/saving from JSON files;
- sprite canvas editing and PNG/header export;
- banner editing workflow for FX catalog images;
- project-local server target, similar to `make pet-studio`.

Rename and reshape the concepts rather than carrying over pet-specific language.

## Board Profile Shape

TableTop Studio profiles should live under a future `boards/` directory. A board
profile should include:

- `id` and display name;
- variant family, such as `classic-nine`, `six-men`, or `three-men`;
- piece count per player;
- optional first-player/default settings;
- board canvas size and point coordinates;
- adjacency graph as point-index pairs or fixed adjacency lists, used both for
  board drawing and legal movement;
- mill lines as point-index triples;
- optional rule flags, such as flying enabled or capture restrictions;
- asset paths for board sprite, FX banner, and preview art.

The generated firmware data should stay data-first, matching the current
`Board.*` direction. Rules should consume point indices, adjacency, and mill
lines, not pixels.

Firmware currently separates this into:

- `BoardDefinition`: board id, display label, point coordinates, mill triples,
  and adjacency slots;
- `RuleSet`: rule id, pieces per player, flying settings, protected mill
  capture behavior, material/block win switches, and no-capture draw limit.

Studio output should target these structures first, then grow into richer board
sprites once the graph/rule data is stable.

## Editor Modes

Initial TableTop Studio modes:

- `Boards`: choose, duplicate, rename, and edit board JSON profiles.
- `Graph`: place numbered points over a board reference image, draw board
  connections/legal movement edges, and define mill triples.
- `Sprites`: draw or import the visible board sprite and FX banner.
- `Validate`: check graph consistency before generation.

Validation should catch:

- disconnected points;
- adjacency references to missing points;
- mill lines that reference missing or repeated points;
- duplicate edges or duplicate mill lines;
- points outside the target screen bounds;
- mismatch between piece count and expected variant shape.

## Reference Overlay

The editor must support a reference graph overlay on top of the board sprite.
This overlay is the contract between art and play:

- sprite pixels define how the board looks;
- point coordinates define where pieces can be placed;
- adjacency edges define both board lines and legal movement;
- mill triples define scoring/capture triggers.

The overlay should be exportable as JSON and as generated C++/PROGMEM data.

## Future Build Integration

Implemented first:

- `boards/classic-nine.json` as the first board profile;
- `boards/six-men.json` as the first non-classic study profile, using the
  two-square European Six Men's Morris layout with six pieces per player and
  mill triples on the square sides;
- `make tabletop-studio`, serving `tools/tabletop-studio/`;
- graph overlay preview from points, adjacency, and mills;
- direct graph editing modes for points, board connections, mill triples, and
  point deletion;
- layer toggles so board connections and mill bands can be inspected
  separately;
- board connections render as a thin black/white dashed line and always draw
  above mill bands, while every mill gets a distinct deterministic color. Mill
  bands only get wider when multiple mills share the same edge, not merely when
  they share a point;
- hover coordinate tracking on the board canvas, using zero-based pixel
  indices in the board profile's logical coordinate space, such as `0..63` for
  a `64x64` board;
- basic validation for missing points, duplicate/self edges, non-bidirectional
  adjacency, malformed mills, and core numeric rule fields;
- JSON duplication/saving through local project endpoints.

Still future:

```sh
make board-data
```

`make board-data` should validate selected board profiles and generate C++ data
files consumed by `Board.*`.
