#pragma once

#include <Arduino.h>

constexpr uint8_t MORRIS_MAX_POINT_COUNT = 24;
constexpr uint8_t MORRIS_MAX_MILL_COUNT = 16;
constexpr uint8_t MORRIS_MAX_ADJACENCY_SLOTS = 4;
constexpr uint8_t MORRIS_MAX_VISUAL_LINE_COUNT = 16;

struct BoardPoint {
  uint8_t x;
  uint8_t y;
};

struct MillLine {
  uint8_t a;
  uint8_t b;
  uint8_t c;
};

struct BoardVisualLine {
  uint8_t a;
  uint8_t b;
};

struct BoardDefinition {
  const char *id;
  const char *label;
  uint8_t pointCount;
  uint8_t millCount;
  uint8_t adjacencySlots;
  uint8_t visualLineCount;
  const BoardPoint *points;
  const MillLine *mills;
  const uint8_t (*adjacency)[MORRIS_MAX_ADJACENCY_SLOTS];
  const BoardVisualLine *visualLines;
};

extern const BoardPoint ClassicBoardPoints[MORRIS_MAX_POINT_COUNT] PROGMEM;
extern const MillLine ClassicMills[MORRIS_MAX_MILL_COUNT] PROGMEM;
extern const uint8_t ClassicAdjacency[MORRIS_MAX_POINT_COUNT][MORRIS_MAX_ADJACENCY_SLOTS] PROGMEM;
extern const BoardVisualLine ClassicVisualLines[MORRIS_MAX_VISUAL_LINE_COUNT] PROGMEM;
extern const BoardDefinition ClassicBoardDefinition;

BoardPoint boardPoint(const BoardDefinition &board, uint8_t index);
MillLine millLine(const BoardDefinition &board, uint8_t index);
uint8_t adjacentPoint(const BoardDefinition &board, uint8_t point, uint8_t slot);
BoardVisualLine visualLine(const BoardDefinition &board, uint8_t index);
