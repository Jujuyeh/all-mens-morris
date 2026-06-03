# Game Design

## Core Premise

All Men's Morris starts from Nine Men's Morris and expands into a collection of
Morris-family variants: classic rules, alternate boards, faster modes, and
custom rule experiments.

## Classic Mode Target

Classic Nine Men's Morris has three phases:

1. Placement: players alternate placing nine pieces.
2. Movement: players move pieces to adjacent points.
3. Flying: when a player has only three pieces, they may move to any empty
   point.

Whenever a player forms a mill, they remove one opponent piece that is not
inside a mill unless no such piece exists.

## Current Playable Slice

Implemented:

- Classic 24-point board.
- Local two-player placement phase.
- Turn switching.
- Mill detection and feedback.
- Capture/removal after a mill, including the classic restriction that pieces
  inside mills are protected unless every opponent piece is inside a mill.
- Basic movement phase scaffolding after all pieces are placed.

Not implemented yet:

- Win/loss detection.
- Flying rule.
- Draw/stalemate handling.
- AI.
- Alternate boards and modes.

## Variant Ideas

- Classic Nine Men's Morris.
- Three Men's Morris and Six Men's Morris.
- Larger or asymmetric boards.
- Puzzle challenges with fixed starting states.
- Timed tactical mode.
- Arduboy FX catalog of board/rule presets.
