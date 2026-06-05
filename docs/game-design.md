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

The current draw rule is a practical early implementation: after movement has
started, 50 completed turns without a capture ends the game as a draw. Immediate
win conditions, such as blocking the opponent, take priority over this draw.

## Current Playable Slice

Implemented:

- Classic 24-point board.
- Local two-player placement phase.
- Turn switching.
- Mill detection and feedback.
- Capture/removal after a mill, including the classic restriction that pieces
  inside mills are protected unless every opponent piece is inside a mill.
- Movement after all pieces are placed.
- Flying to any empty point when the active player has exactly three pieces.
- Win/loss detection when an opponent is reduced below three pieces after all
  reserves are placed, or when the active player has no legal move.
- Draw detection after 50 movement turns without a capture.
- Main menu scaffold for choosing modes, with Classic Nine Men's Morris active
  and smaller variants listed as future entries.
- Horizontal board selector in the main menu, with left/right cycling board
  titles and up/down moving between board and first-player settings.
- Main menu setting for whether white or black moves first.
- Directional cursor navigation that follows the board geometry.
- Hold-A quick menu for reset, one-state rewind, and returning to the main menu.
- Centered board layout with compact side HUD panels.
- Animated dashed cursor selector.
- First visual pass for the simplified board, side HUD, and inverted main menu
  with lightweight pattern decoration.
- Custom inverted boot animation, with the stock Arduboy boot logo and LED
  animation skipped.
- Sound effects for menu/action feedback and RGB LED flashing on mills.
- Debug-only quick-menu scenarios for mill, flying, blocked-game-over, and draw
  testing.

Not implemented yet:

- AI.
- Playable alternate boards and modes.

## Variant Ideas

- Classic Nine Men's Morris.
- Three Men's Morris and Six Men's Morris.
- Larger or asymmetric boards.
- Puzzle challenges with fixed starting states.
- Timed tactical mode.
- Arduboy FX catalog of board/rule presets.
