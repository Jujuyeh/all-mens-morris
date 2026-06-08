#include "Ai.h"

namespace {
constexpr int16_t WIN_SCORE = 30000;
constexpr uint8_t NO_POINT = 255;

struct AiAction {
  uint8_t from;
  uint8_t to;
  TurnActionMode mode;
};

int16_t absoluteScore(int16_t value) {
  return value < 0 ? -value : value;
}

uint8_t countCompleteMills(const MorrisGameState &game, Player player) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < game.board->millCount; i++) {
    MillLine line = millLine(*game.board, i);
    if (game.points[line.a] == player
        && game.points[line.b] == player
        && game.points[line.c] == player) {
      count++;
    }
  }
  return count;
}

uint8_t countOpenTwoMills(const MorrisGameState &game, Player player) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < game.board->millCount; i++) {
    MillLine line = millLine(*game.board, i);
    uint8_t owned = 0;
    uint8_t empty = 0;
    Player a = game.points[line.a];
    Player b = game.points[line.b];
    Player c = game.points[line.c];
    owned += a == player ? 1 : 0;
    owned += b == player ? 1 : 0;
    owned += c == player ? 1 : 0;
    empty += a == PLAYER_NONE ? 1 : 0;
    empty += b == PLAYER_NONE ? 1 : 0;
    empty += c == PLAYER_NONE ? 1 : 0;
    if (owned == 2 && empty == 1) {
      count++;
    }
  }
  return count;
}

uint8_t pointDegree(const MorrisGameState &game, uint8_t point) {
  uint8_t count = 0;
  for (uint8_t slot = 0; slot < game.board->adjacencySlots; slot++) {
    if (adjacentPoint(*game.board, point, slot) != 255) {
      count++;
    }
  }
  return count;
}

uint8_t mobilityFor(const MorrisGameState &game, Player player) {
  if (game.phase != PHASE_MOVING && game.phase != PHASE_PLACING) {
    return 0;
  }

  uint8_t moves = 0;
  for (uint8_t from = 0; from < game.board->pointCount; from++) {
    if (game.points[from] != player) {
      continue;
    }

    if (playerCanFly(game, player)) {
      for (uint8_t to = 0; to < game.board->pointCount; to++) {
        if (game.points[to] == PLAYER_NONE) {
          moves++;
        }
      }
      continue;
    }

    for (uint8_t slot = 0; slot < game.board->adjacencySlots; slot++) {
      uint8_t to = adjacentPoint(*game.board, from, slot);
      if (to != 255 && game.points[to] == PLAYER_NONE) {
        moves++;
      }
    }
  }
  return moves;
}

int16_t evaluateState(const MorrisGameState &game, Player aiPlayer) {
  Player opponent = opponentOf(aiPlayer);
  if (game.phase == PHASE_GAME_OVER) {
    if (game.winner == aiPlayer) {
      return WIN_SCORE;
    }
    if (game.winner == opponent) {
      return -WIN_SCORE;
    }
    return 0;
  }

  uint8_t aiIndex = playerIndex(aiPlayer);
  uint8_t opponentIndex = playerIndex(opponent);
  int16_t score = 0;
  score += static_cast<int16_t>(game.piecesOnBoard[aiIndex] - game.piecesOnBoard[opponentIndex]) * 120;
  score += static_cast<int16_t>(game.piecesToPlace[aiIndex] - game.piecesToPlace[opponentIndex]) * 12;
  score += static_cast<int16_t>(countCompleteMills(game, aiPlayer) - countCompleteMills(game, opponent)) * 90;
  score += static_cast<int16_t>(countOpenTwoMills(game, aiPlayer) - countOpenTwoMills(game, opponent)) * 35;
  score += static_cast<int16_t>(mobilityFor(game, aiPlayer) - mobilityFor(game, opponent)) * 4;

  for (uint8_t point = 0; point < game.board->pointCount; point++) {
    if (game.points[point] == aiPlayer) {
      score += pointDegree(game, point) * 3;
    } else if (game.points[point] == opponent) {
      score -= pointDegree(game, point) * 3;
    }
  }

  if (game.phase == PHASE_CAPTURING && game.currentPlayer == aiPlayer) {
    score += 500;
  }
  if (game.lastMoveMadeMill && game.currentPlayer == aiPlayer) {
    score += game.rules->millAction == MILL_ACTION_WIN ? 1200 : 500;
  }
  return score;
}

bool applyAiAction(const MorrisGameState &game, const AiAction &action, MorrisGameState &result) {
  result = game;
  result.selected = action.from;
  result.cursor = action.to;
  result.actionMode = action.mode;
  return applyPrimaryAction(result);
}

int16_t scoreAction(const MorrisGameState &game, const AiAction &action, Player aiPlayer) {
  MorrisGameState after;
  if (!applyAiAction(game, action, after)) {
    return -WIN_SCORE;
  }

  int16_t score = evaluateState(after, aiPlayer);
  if (game.phase == PHASE_CAPTURING && game.points[action.to] == opponentOf(aiPlayer)) {
    score += isMillAt(game, action.to, opponentOf(aiPlayer)) ? 10 : 80;
    score += pointDegree(game, action.to) * 8;
  }
  if (after.phase == PHASE_CAPTURING && after.currentPlayer == aiPlayer) {
    score += 600;
  }
  if (after.phase == PHASE_GAME_OVER && after.winner == aiPlayer) {
    score += 2000;
  }
  score -= absoluteScore(static_cast<int16_t>(action.to) - static_cast<int16_t>(game.cursor));
  return score;
}

bool considerAction(const MorrisGameState &game, const AiAction &action, Player aiPlayer,
                    bool &hasBest, int16_t &bestScore, AiAction &bestAction) {
  int16_t score = scoreAction(game, action, aiPlayer);
  if (!hasBest || score > bestScore) {
    hasBest = true;
    bestScore = score;
    bestAction = action;
  }
  return hasBest;
}

void considerPlaceActions(const MorrisGameState &game, Player aiPlayer, bool &hasBest,
                          int16_t &bestScore, AiAction &bestAction, TurnActionMode mode) {
  for (uint8_t point = 0; point < game.board->pointCount; point++) {
    MorrisGameState candidate = game;
    candidate.actionMode = mode;
    candidate.cursor = point;
    if (canPlaceAt(candidate, point)) {
      considerAction(game, {NO_POINT, point, mode}, aiPlayer, hasBest, bestScore, bestAction);
    }
  }
}

void considerMoveActions(const MorrisGameState &game, Player aiPlayer, bool &hasBest,
                         int16_t &bestScore, AiAction &bestAction) {
  for (uint8_t from = 0; from < game.board->pointCount; from++) {
    if (game.points[from] != aiPlayer) {
      continue;
    }
    for (uint8_t to = 0; to < game.board->pointCount; to++) {
      MorrisGameState candidate = game;
      candidate.selected = from;
      candidate.cursor = to;
      candidate.actionMode = TURN_ACTION_MOVE;
      if (canMovePiece(candidate, from, to)) {
        considerAction(game, {from, to, TURN_ACTION_MOVE}, aiPlayer, hasBest, bestScore, bestAction);
      }
    }
  }
}

void considerCaptureActions(const MorrisGameState &game, Player aiPlayer, bool &hasBest,
                            int16_t &bestScore, AiAction &bestAction) {
  for (uint8_t point = 0; point < game.board->pointCount; point++) {
    MorrisGameState candidate = game;
    candidate.cursor = point;
    if (canCaptureAt(candidate, point)) {
      considerAction(game, {NO_POINT, point, game.actionMode}, aiPlayer, hasBest, bestScore, bestAction);
    }
  }
}
}

bool chooseAiAction(const MorrisGameState &game, MorrisGameState &result) {
  if (game.phase == PHASE_GAME_OVER) {
    return false;
  }

  Player aiPlayer = game.currentPlayer;
  bool hasBest = false;
  int16_t bestScore = -WIN_SCORE;
  AiAction bestAction = {NO_POINT, game.cursor, game.actionMode};

  if (game.phase == PHASE_CAPTURING) {
    considerCaptureActions(game, aiPlayer, hasBest, bestScore, bestAction);
  } else if (game.phase == PHASE_PLACING) {
    considerPlaceActions(game, aiPlayer, hasBest, bestScore, bestAction, TURN_ACTION_PLACE);
  } else if (game.phase == PHASE_MOVING) {
    if (game.rules->mixedPlacementMovement) {
      considerPlaceActions(game, aiPlayer, hasBest, bestScore, bestAction, TURN_ACTION_PLACE);
    }
    considerMoveActions(game, aiPlayer, hasBest, bestScore, bestAction);
  }

  return hasBest && applyAiAction(game, bestAction, result);
}
