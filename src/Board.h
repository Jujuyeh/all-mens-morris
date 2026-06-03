#pragma once

#include <Arduino.h>

constexpr uint8_t MORRIS_POINT_COUNT = 24;
constexpr uint8_t MORRIS_MILL_COUNT = 16;
constexpr uint8_t MORRIS_ADJACENCY_SLOTS = 4;

struct BoardPoint {
  uint8_t x;
  uint8_t y;
};

struct MillLine {
  uint8_t a;
  uint8_t b;
  uint8_t c;
};

extern const BoardPoint ClassicBoardPoints[MORRIS_POINT_COUNT] PROGMEM;
extern const MillLine ClassicMills[MORRIS_MILL_COUNT] PROGMEM;
extern const uint8_t ClassicAdjacency[MORRIS_POINT_COUNT][MORRIS_ADJACENCY_SLOTS] PROGMEM;

BoardPoint boardPoint(uint8_t index);
MillLine millLine(uint8_t index);
uint8_t adjacentPoint(uint8_t point, uint8_t slot);
