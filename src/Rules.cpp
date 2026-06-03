#include "Rules.h"

void resetMorrisGame(MorrisGameState &game) {
  for (uint8_t i = 0; i < MORRIS_POINT_COUNT; i++) {
    game.points[i] = PLAYER_NONE;
  }
  game.currentPlayer = PLAYER_ONE;
  game.phase = PHASE_PLACING;
  game.cursor = 0;
  game.selected = 255;
  game.piecesToPlace[0] = 9;
  game.piecesToPlace[1] = 9;
  game.piecesOnBoard[0] = 0;
  game.piecesOnBoard[1] = 0;
  game.phaseAfterCapture = PHASE_PLACING;
  game.millPending = false;
  game.lastMoveMadeMill = false;
}

uint8_t playerIndex(Player player) {
  return player == PLAYER_TWO ? 1 : 0;
}

Player opponentOf(Player player) {
  return player == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE;
}

bool isMillAt(const MorrisGameState &game, uint8_t point, Player player) {
  if (player == PLAYER_NONE) {
    return false;
  }

  for (uint8_t i = 0; i < MORRIS_MILL_COUNT; i++) {
    MillLine line = millLine(i);
    bool containsPoint = line.a == point || line.b == point || line.c == point;
    if (!containsPoint) {
      continue;
    }
    if (game.points[line.a] == player
        && game.points[line.b] == player
        && game.points[line.c] == player) {
      return true;
    }
  }
  return false;
}

bool canPlaceAt(const MorrisGameState &game, uint8_t point) {
  return game.phase == PHASE_PLACING && game.points[point] == PLAYER_NONE;
}

bool canMovePiece(const MorrisGameState &game, uint8_t from, uint8_t to) {
  if (game.phase != PHASE_MOVING
      || game.points[from] != game.currentPlayer
      || game.points[to] != PLAYER_NONE) {
    return false;
  }

  for (uint8_t slot = 0; slot < MORRIS_ADJACENCY_SLOTS; slot++) {
    if (adjacentPoint(from, slot) == to) {
      return true;
    }
  }
  return false;
}

static bool playerHasPieceOutsideMill(const MorrisGameState &game, Player player) {
  for (uint8_t point = 0; point < MORRIS_POINT_COUNT; point++) {
    if (game.points[point] == player && !isMillAt(game, point, player)) {
      return true;
    }
  }
  return false;
}

bool canCaptureAt(const MorrisGameState &game, uint8_t point) {
  if (game.phase != PHASE_CAPTURING || !game.millPending) {
    return false;
  }

  Player opponent = opponentOf(game.currentPlayer);
  if (game.points[point] != opponent) {
    return false;
  }

  return !isMillAt(game, point, opponent)
      || !playerHasPieceOutsideMill(game, opponent);
}

static void endTurn(MorrisGameState &game) {
  game.currentPlayer = opponentOf(game.currentPlayer);
}

static void finishPlacementIfReady(MorrisGameState &game) {
  if (game.piecesToPlace[0] == 0 && game.piecesToPlace[1] == 0) {
    game.phase = PHASE_MOVING;
  }
}

static void enterCapture(MorrisGameState &game, GamePhase phaseAfterCapture) {
  game.phase = PHASE_CAPTURING;
  game.phaseAfterCapture = phaseAfterCapture;
  game.millPending = true;
}

static void finishAction(MorrisGameState &game, GamePhase currentPhase) {
  if (game.lastMoveMadeMill) {
    enterCapture(game, currentPhase);
    return;
  }

  finishPlacementIfReady(game);
  endTurn(game);
}

static bool placeAtCursor(MorrisGameState &game) {
  if (!canPlaceAt(game, game.cursor)) {
    return false;
  }

  uint8_t index = playerIndex(game.currentPlayer);
  game.points[game.cursor] = game.currentPlayer;
  game.piecesToPlace[index]--;
  game.piecesOnBoard[index]++;
  game.lastMoveMadeMill = isMillAt(game, game.cursor, game.currentPlayer);
  finishAction(game, PHASE_PLACING);
  return true;
}

static bool moveFromSelection(MorrisGameState &game) {
  if (game.selected == 255) {
    if (game.points[game.cursor] == game.currentPlayer) {
      game.selected = game.cursor;
      return true;
    }
    return false;
  }

  if (game.cursor == game.selected) {
    game.selected = 255;
    return true;
  }

  if (!canMovePiece(game, game.selected, game.cursor)) {
    return false;
  }

  game.points[game.cursor] = game.currentPlayer;
  game.points[game.selected] = PLAYER_NONE;
  game.lastMoveMadeMill = isMillAt(game, game.cursor, game.currentPlayer);
  game.selected = 255;
  finishAction(game, PHASE_MOVING);
  return true;
}

static bool captureAtCursor(MorrisGameState &game) {
  if (!canCaptureAt(game, game.cursor)) {
    return false;
  }

  uint8_t opponentIndex = playerIndex(opponentOf(game.currentPlayer));
  game.points[game.cursor] = PLAYER_NONE;
  if (game.piecesOnBoard[opponentIndex] > 0) {
    game.piecesOnBoard[opponentIndex]--;
  }

  game.phase = game.phaseAfterCapture;
  game.phaseAfterCapture = PHASE_PLACING;
  game.millPending = false;
  game.lastMoveMadeMill = false;
  game.selected = 255;
  finishPlacementIfReady(game);
  endTurn(game);
  return true;
}

bool applyPrimaryAction(MorrisGameState &game) {
  game.lastMoveMadeMill = false;
  if (game.phase == PHASE_PLACING) {
    return placeAtCursor(game);
  }
  if (game.phase == PHASE_CAPTURING) {
    return captureAtCursor(game);
  }
  if (game.phase == PHASE_MOVING) {
    return moveFromSelection(game);
  }
  return false;
}

void advanceCursor(MorrisGameState &game, int8_t delta) {
  int8_t next = static_cast<int8_t>(game.cursor) + delta;
  while (next < 0) {
    next += MORRIS_POINT_COUNT;
  }
  while (next >= MORRIS_POINT_COUNT) {
    next -= MORRIS_POINT_COUNT;
  }
  game.cursor = static_cast<uint8_t>(next);
}
