#include "Board.h"

// Screen-space coordinates for the classic 24-point Nine Men's Morris board.
// They are deliberately data-only so future board variants can live beside them.
const BoardPoint ClassicBoardPoints[MORRIS_MAX_POINT_COUNT] PROGMEM = {
  {4, 4}, {32, 4}, {60, 4}, {12, 12}, {32, 12}, {52, 12},
  {20, 20}, {32, 20}, {44, 20}, {4, 32}, {12, 32}, {20, 32},
  {44, 32}, {52, 32}, {60, 32}, {20, 44}, {32, 44}, {44, 44},
  {12, 52}, {32, 52}, {52, 52}, {4, 60}, {32, 60}, {60, 60},
};

// Every possible mill in the classic board. Detection checks these triples.
const MillLine ClassicMills[MORRIS_MAX_MILL_COUNT] PROGMEM = {
  {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11},
  {12, 13, 14}, {15, 16, 17}, {18, 19, 20}, {21, 22, 23},
  {0, 9, 21}, {3, 10, 18}, {6, 11, 15}, {1, 4, 7},
  {16, 19, 22}, {8, 12, 17}, {5, 13, 20}, {2, 14, 23},
};

// 255 marks an empty adjacency slot. Keeping a fixed width avoids heap use.
const uint8_t ClassicAdjacency[MORRIS_MAX_POINT_COUNT][MORRIS_MAX_ADJACENCY_SLOTS] PROGMEM = {
  {1, 9, 255, 255},
  {0, 2, 4, 255},
  {1, 14, 255, 255},
  {4, 10, 255, 255},
  {1, 3, 5, 7},
  {4, 13, 255, 255},
  {7, 11, 255, 255},
  {4, 6, 8, 255},
  {7, 12, 255, 255},
  {0, 10, 21, 255},
  {3, 9, 11, 18},
  {6, 10, 15, 255},
  {8, 13, 17, 255},
  {5, 12, 14, 20},
  {2, 13, 23, 255},
  {11, 16, 255, 255},
  {15, 17, 19, 255},
  {12, 16, 255, 255},
  {10, 19, 255, 255},
  {16, 18, 20, 22},
  {13, 19, 255, 255},
  {9, 22, 255, 255},
  {19, 21, 23, 255},
  {14, 22, 255, 255},
};

const BoardDefinition ClassicBoardDefinition = {
  "classic-nine",
  "CLASSIC 9",
  MORRIS_MAX_POINT_COUNT,
  MORRIS_MAX_MILL_COUNT,
  MORRIS_MAX_ADJACENCY_SLOTS,
  ClassicBoardPoints,
  ClassicMills,
  ClassicAdjacency,
};

BoardPoint boardPoint(const BoardDefinition &board, uint8_t index) {
  BoardPoint point;
  memcpy_P(&point, &board.points[index], sizeof(BoardPoint));
  return point;
}

MillLine millLine(const BoardDefinition &board, uint8_t index) {
  MillLine line;
  memcpy_P(&line, &board.mills[index], sizeof(MillLine));
  return line;
}

uint8_t adjacentPoint(const BoardDefinition &board, uint8_t point, uint8_t slot) {
  return pgm_read_byte(&board.adjacency[point][slot]);
}
