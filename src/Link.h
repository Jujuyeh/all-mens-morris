#pragma once

#include <Arduino.h>

#include "Rules.h"

enum LinkEventKind : uint8_t {
  LINK_EVENT_NONE,
  LINK_EVENT_START,
  LINK_EVENT_ACTION,
};

struct LinkEvent {
  LinkEventKind kind = LINK_EVENT_NONE;
  uint8_t board = 0;
  Player firstPlayer = PLAYER_TWO;
  TurnActionMode mode = TURN_ACTION_MOVE;
  uint8_t from = 255;
  uint8_t to = 255;
};

void linkBegin(uint32_t seed);
void linkUpdate(bool inMainMenu, uint8_t board, Player firstPlayer);
bool linkPeerAvailable();
bool linkCableFlipped();
bool linkLocalIsPlayerOne();
Player linkLocalPlayer(Player firstPlayer);
bool linkConsumeEvent(LinkEvent &event);
void linkSendStart(uint8_t board, Player firstPlayer);
void linkSendAction(TurnActionMode mode, uint8_t from, uint8_t to);
