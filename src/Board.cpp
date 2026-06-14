#include "Board.h"

BoardPoint boardPoint(const BoardDefinition &board, uint8_t index) {
  BoardPoint point;
  memcpy_P(&point, &board.points[index], sizeof(BoardPoint));
  return point;
}

MillLine millLine(const BoardDefinition &board, uint8_t index) {
  uint16_t packed = pgm_read_word(&board.mills[index]);
  return {
    static_cast<uint8_t>(packed & 31),
    static_cast<uint8_t>((packed >> 5) & 31),
    static_cast<uint8_t>((packed >> 10) & 31),
  };
}

uint8_t adjacentPoint(const BoardDefinition &board, uint8_t point, uint8_t slot) {
  uint8_t start = pgm_read_byte(&board.adjacencyOffsets[point]);
  uint8_t end = pgm_read_byte(&board.adjacencyOffsets[point + 1]);
  if (slot >= end - start) {
    return MORRIS_NO_POINT;
  }
  return pgm_read_byte(&board.adjacency[start + slot]);
}

uint8_t adjacencyCount(const BoardDefinition &board, uint8_t point) {
  uint8_t start = pgm_read_byte(&board.adjacencyOffsets[point]);
  uint8_t end = pgm_read_byte(&board.adjacencyOffsets[point + 1]);
  return end - start;
}
