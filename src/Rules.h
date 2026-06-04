#pragma once

#include <Arduino.h>

#include "Board.h"

enum Player : uint8_t {
  PLAYER_NONE = 0,
  PLAYER_ONE = 1,
  PLAYER_TWO = 2,
};

enum GamePhase : uint8_t {
  PHASE_PLACING,
  PHASE_CAPTURING,
  PHASE_MOVING,
  PHASE_GAME_OVER,
};

enum WinReason : uint8_t {
  WIN_NONE,
  WIN_BY_MATERIAL,
  WIN_BY_BLOCK,
  WIN_DRAW_NO_CAPTURE,
};

constexpr uint8_t DRAW_NO_CAPTURE_TURN_LIMIT = 50;

struct MorrisGameState {
  Player points[MORRIS_POINT_COUNT] = {};
  Player currentPlayer = PLAYER_ONE;
  Player winner = PLAYER_NONE;
  GamePhase phase = PHASE_PLACING;
  WinReason winReason = WIN_NONE;
  uint8_t cursor = 0;
  uint8_t selected = 255;
  uint8_t piecesToPlace[2] = {9, 9};
  uint8_t piecesOnBoard[2] = {0, 0};
  uint8_t turnsSinceCapture = 0;
  GamePhase phaseAfterCapture = PHASE_PLACING;
  bool millPending = false;
  bool lastMoveMadeMill = false;
};

void resetMorrisGame(MorrisGameState &game);
uint8_t playerIndex(Player player);
Player opponentOf(Player player);
bool isMillAt(const MorrisGameState &game, uint8_t point, Player player);
bool canPlaceAt(const MorrisGameState &game, uint8_t point);
bool playerCanFly(const MorrisGameState &game, Player player);
bool canMovePiece(const MorrisGameState &game, uint8_t from, uint8_t to);
bool canCaptureAt(const MorrisGameState &game, uint8_t point);
bool applyPrimaryAction(MorrisGameState &game);
void advanceCursor(MorrisGameState &game, int8_t delta);
