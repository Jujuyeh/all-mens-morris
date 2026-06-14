#include "MenuMusic.h"

// Generated from TableTop Studio menu music data.
// Menu themes are compact monophonic Arduboy tone event streams.

const uint8_t MenuMusicTheme0Notes[] PROGMEM = {
  48, 0, 55, 52, 0, 48, 55, 0, 50, 0, 57, 53, 0, 50, 57, 0,
};

const uint8_t MenuMusicTheme0Durations[] PROGMEM = {
  3, 1, 1, 2, 2, 2, 1, 2, 3, 1, 1, 2, 2, 2, 1, 2,
};
const uint8_t MenuMusicTheme1Notes[] PROGMEM = {
  45, 0, 52, 57, 0, 52, 45, 0, 47, 0, 54, 59, 0, 54, 47, 0,
};

const uint8_t MenuMusicTheme1Durations[] PROGMEM = {
  3, 1, 1, 2, 2, 2, 1, 2, 3, 1, 1, 2, 2, 2, 1, 2,
};
const uint8_t MenuMusicTheme2Notes[] PROGMEM = {
  43, 0, 50, 55, 0, 50, 55, 0, 45, 0, 52, 57, 0, 52, 57, 0,
};

const uint8_t MenuMusicTheme2Durations[] PROGMEM = {
  3, 1, 1, 2, 2, 2, 1, 2, 3, 1, 1, 2, 2, 2, 1, 2,
};

const uint8_t MenuMusicThemeEventCounts[] PROGMEM = { 16, 16, 16 };

const uint8_t *const MenuMusicNotesByTheme[] PROGMEM = { MenuMusicTheme0Notes, MenuMusicTheme1Notes, MenuMusicTheme2Notes };

const uint8_t *const MenuMusicDurationsByTheme[] PROGMEM = { MenuMusicTheme0Durations, MenuMusicTheme1Durations, MenuMusicTheme2Durations };

const uint16_t MenuMusicFrequencies[] PROGMEM = {
  98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233,
  247,
};
