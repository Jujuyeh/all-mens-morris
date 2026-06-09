#pragma once

#include <Arduino.h>

constexpr uint8_t MENU_MUSIC_EVENT_COUNT = 148;
constexpr uint8_t MENU_MUSIC_MIN_NOTE = 36;
constexpr uint8_t MENU_MUSIC_MAX_NOTE = 64;
constexpr uint8_t MENU_MUSIC_TICK_MS = 69;

extern const uint8_t MenuMusicNotes[] PROGMEM;
extern const uint8_t MenuMusicDurations[] PROGMEM;
extern const uint16_t MenuMusicFrequencies[] PROGMEM;
