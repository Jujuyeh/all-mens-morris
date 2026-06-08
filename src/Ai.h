#pragma once

#include <Arduino.h>

#include "Rules.h"

struct AiAction {
  uint8_t from;
  uint8_t to;
  TurnActionMode mode;
};

bool chooseAiAction(const MorrisGameState &game, MorrisGameState &result);
bool chooseAiAction(const MorrisGameState &game, AiAction &action, MorrisGameState &result);
