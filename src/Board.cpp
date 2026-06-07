#include "Board.h"

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
