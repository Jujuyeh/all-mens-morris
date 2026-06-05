#include "AllMensMorrisGame.h"

#include "Assets.h"
#include "Board.h"
#include "Rules.h"

#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>

namespace {
Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint8_t GAME_FPS = 30;
constexpr uint8_t BASE_FPS = 10;
constexpr uint8_t FPS_SCALE = GAME_FPS / BASE_FPS;
constexpr uint8_t framesAtGameFps(uint8_t baseFrames) {
  return baseFrames * FPS_SCALE;
}
constexpr uint8_t BOARD_OFFSET_X = 32;
constexpr uint8_t BOARD_OFFSET_Y = 0;
constexpr uint8_t HUD_LEFT_X = 1;
constexpr uint8_t HUD_RIGHT_X = 99;
constexpr uint8_t NO_POINT = 255;
constexpr uint8_t MENU_ITEM_COUNT = 2;
constexpr uint8_t BOARD_MENU_COUNT = 3;
constexpr uint8_t BOOT_LOGO_X = 52;
constexpr uint8_t BOOT_LOGO_Y = 26;
constexpr uint8_t BOOT_DUST_START_FRAMES = framesAtGameFps(20);
constexpr uint8_t BOOT_FILL_START_FRAMES = framesAtGameFps(32);
constexpr uint8_t BOOT_CURTAIN_START_FRAMES = framesAtGameFps(47);
constexpr uint8_t BOOT_TOTAL_FRAMES = framesAtGameFps(52);

enum AppScene : uint8_t {
  SCENE_MAIN_MENU,
  SCENE_PLAYING,
};

enum ConfirmAction : uint8_t {
  CONFIRM_NONE,
  CONFIRM_RESET,
  CONFIRM_MAIN_MENU,
};

#ifdef ALL_MENS_MORRIS_DEBUG
enum DebugScenario : uint8_t {
  DEBUG_SCENARIO_MILL,
  DEBUG_SCENARIO_FLY,
  DEBUG_SCENARIO_BLOCK,
  DEBUG_SCENARIO_DRAW,
  DEBUG_SCENARIO_COUNT,
};
#endif

MorrisGameState game;
MorrisGameState undoGame;
AppScene scene = SCENE_MAIN_MENU;
ConfirmAction confirmAction = CONFIRM_NONE;
uint8_t selectedMenuItem = 0;
uint8_t selectedBoardMenuItem = 0;
Player firstPlayer = PLAYER_TWO;
bool hasUndo = false;
uint8_t messageFrames = 0;
uint8_t millFlashFrames = 0;
uint8_t animationFrame = 0;
#ifdef ALL_MENS_MORRIS_DEBUG
uint8_t debugScenario = DEBUG_SCENARIO_MILL;
#endif
const char *message = "";

void setMessage(const char *text) {
  message = text;
  messageFrames = 45;
}

BoardPoint screenPoint(uint8_t index) {
  BoardPoint point = boardPoint(index);
  point.x += BOARD_OFFSET_X;
  point.y += BOARD_OFFSET_Y;
  return point;
}

bool gameStateChanged(const MorrisGameState &before, const MorrisGameState &after) {
  if (before.currentPlayer != after.currentPlayer
      || before.winner != after.winner
      || before.phase != after.phase
      || before.winReason != after.winReason
      || before.phaseAfterCapture != after.phaseAfterCapture
      || before.millPending != after.millPending
      || before.lastMoveMadeMill != after.lastMoveMadeMill
      || before.turnsSinceCapture != after.turnsSinceCapture) {
    return true;
  }

  for (uint8_t i = 0; i < 2; i++) {
    if (before.piecesToPlace[i] != after.piecesToPlace[i]
        || before.piecesOnBoard[i] != after.piecesOnBoard[i]) {
      return true;
    }
  }

  for (uint8_t point = 0; point < MORRIS_POINT_COUNT; point++) {
    if (before.points[point] != after.points[point]) {
      return true;
    }
  }
  return false;
}

void startMatch() {
  resetMorrisGame(game);
  game.currentPlayer = firstPlayer;
  hasUndo = false;
  millFlashFrames = 0;
  confirmAction = CONFIRM_NONE;
  scene = SCENE_PLAYING;
  setMessage("");
}

#ifdef ALL_MENS_MORRIS_DEBUG
void clearDebugBoard() {
  for (uint8_t point = 0; point < MORRIS_POINT_COUNT; point++) {
    game.points[point] = PLAYER_NONE;
  }
  game.cursor = 0;
  game.selected = NO_POINT;
  game.winner = PLAYER_NONE;
  game.winReason = WIN_NONE;
  game.phaseAfterCapture = PHASE_PLACING;
  game.millPending = false;
  game.lastMoveMadeMill = false;
  game.turnsSinceCapture = 0;
  game.piecesToPlace[0] = 0;
  game.piecesToPlace[1] = 0;
  game.piecesOnBoard[0] = 0;
  game.piecesOnBoard[1] = 0;
}

void putDebugPiece(uint8_t point, Player player) {
  game.points[point] = player;
  if (player != PLAYER_NONE) {
    game.piecesOnBoard[playerIndex(player)]++;
  }
}

void loadDebugMillScenario() {
  resetMorrisGame(game);
  clearDebugBoard();
  game.phase = PHASE_PLACING;
  game.currentPlayer = PLAYER_TWO;
  game.piecesToPlace[0] = 7;
  game.piecesToPlace[1] = 7;
  putDebugPiece(0, PLAYER_TWO);
  putDebugPiece(1, PLAYER_TWO);
  putDebugPiece(9, PLAYER_ONE);
  putDebugPiece(10, PLAYER_ONE);
  game.cursor = 2;
  setMessage("DBG MILL");
}

void loadDebugFlyScenario() {
  resetMorrisGame(game);
  clearDebugBoard();
  game.phase = PHASE_MOVING;
  game.currentPlayer = PLAYER_TWO;
  putDebugPiece(0, PLAYER_TWO);
  putDebugPiece(9, PLAYER_TWO);
  putDebugPiece(21, PLAYER_TWO);
  putDebugPiece(2, PLAYER_ONE);
  putDebugPiece(14, PLAYER_ONE);
  putDebugPiece(23, PLAYER_ONE);
  putDebugPiece(4, PLAYER_ONE);
  game.cursor = 0;
  setMessage("DBG FLY");
}

void loadDebugBlockScenario() {
  resetMorrisGame(game);
  clearDebugBoard();
  game.phase = PHASE_GAME_OVER;
  game.currentPlayer = PLAYER_ONE;
  game.winner = PLAYER_TWO;
  game.winReason = WIN_BY_BLOCK;
  putDebugPiece(0, PLAYER_ONE);
  putDebugPiece(2, PLAYER_ONE);
  putDebugPiece(9, PLAYER_ONE);
  putDebugPiece(1, PLAYER_TWO);
  putDebugPiece(10, PLAYER_TWO);
  putDebugPiece(14, PLAYER_TWO);
  putDebugPiece(21, PLAYER_TWO);
  game.cursor = 0;
  setMessage("DBG BLOCK");
}

void loadDebugDrawScenario() {
  resetMorrisGame(game);
  clearDebugBoard();
  game.phase = PHASE_MOVING;
  game.currentPlayer = PLAYER_TWO;
  game.turnsSinceCapture = DRAW_NO_CAPTURE_TURN_LIMIT - 1;
  putDebugPiece(0, PLAYER_TWO);
  putDebugPiece(9, PLAYER_TWO);
  putDebugPiece(21, PLAYER_TWO);
  putDebugPiece(2, PLAYER_ONE);
  putDebugPiece(14, PLAYER_ONE);
  putDebugPiece(23, PLAYER_ONE);
  game.cursor = 0;
  setMessage("DBG DRAW");
}

void loadDebugScenario() {
  hasUndo = false;
  millFlashFrames = 0;
  confirmAction = CONFIRM_NONE;
  scene = SCENE_PLAYING;
  arduboy.digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF);

  if (debugScenario == DEBUG_SCENARIO_MILL) {
    loadDebugMillScenario();
  } else if (debugScenario == DEBUG_SCENARIO_FLY) {
    loadDebugFlyScenario();
  } else if (debugScenario == DEBUG_SCENARIO_BLOCK) {
    loadDebugBlockScenario();
  } else {
    loadDebugDrawScenario();
  }

  debugScenario = (debugScenario + 1) % DEBUG_SCENARIO_COUNT;
  sound.tone(988, 35, 740, 50);
}
#endif

uint16_t absoluteDelta(int16_t value) {
  return value < 0 ? -value : value;
}

uint8_t cursorToward(int8_t dx, int8_t dy) {
  BoardPoint current = boardPoint(game.cursor);
  uint8_t best = NO_POINT;
  uint16_t bestDistance = 65535;

  for (uint8_t point = 0; point < MORRIS_POINT_COUNT; point++) {
    if (point == game.cursor) {
      continue;
    }

    BoardPoint candidate = boardPoint(point);
    int16_t deltaX = static_cast<int16_t>(candidate.x) - current.x;
    int16_t deltaY = static_cast<int16_t>(candidate.y) - current.y;
    if ((dx < 0 && deltaX >= 0) || (dx > 0 && deltaX <= 0)
        || (dy < 0 && deltaY >= 0) || (dy > 0 && deltaY <= 0)) {
      continue;
    }

    uint16_t primary = dx == 0 ? absoluteDelta(deltaY) : absoluteDelta(deltaX);
    uint16_t secondary = dx == 0 ? absoluteDelta(deltaX) : absoluteDelta(deltaY);
    uint16_t distance = primary * 4 + secondary * 64;
    if (distance < bestDistance) {
      bestDistance = distance;
      best = point;
    }
  }
  return best;
}

void moveCursorToward(int8_t dx, int8_t dy) {
  uint8_t next = cursorToward(dx, dy);
  if (next != NO_POINT) {
    game.cursor = next;
  }
}

void ledsOff() {
  arduboy.digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF);
}

bool spritePixelIsBlack(const uint8_t *sprite, uint8_t frame, uint8_t x, uint8_t y) {
  uint8_t width = pgm_read_byte(sprite);
  uint8_t height = pgm_read_byte(sprite + 1);
  uint8_t pages = (height + 7) / 8;
  uint16_t frameOffset = static_cast<uint16_t>(frame) * width * pages;
  uint8_t byte = pgm_read_byte(sprite + 2 + frameOffset + (y / 8) * width + x);
  return ((byte >> (y % 8)) & 1) == 0;
}

void drawSpriteWhitePixelsAsBlack(const uint8_t *sprite, uint8_t frame, int16_t x, int16_t y) {
  uint8_t width = pgm_read_byte(sprite);
  uint8_t height = pgm_read_byte(sprite + 1);
  for (uint8_t py = 0; py < height; py++) {
    for (uint8_t px = 0; px < width; px++) {
      int16_t drawX = x + px;
      int16_t drawY = y + py;
      if (drawX >= 0 && drawX < 128 && drawY >= 0 && drawY < 64
          && !spritePixelIsBlack(sprite, frame, px, py)) {
        arduboy.drawPixel(drawX, drawY, BLACK);
      }
    }
  }
}

void drawBootLogo(uint8_t frame) {
  int8_t dx = (frame % 4 == 0) ? -2 : (frame % 4 == 2) ? 2 : 0;
  int8_t dy = (frame % 6 == 1) ? -1 : (frame % 6 == 4) ? 1 : 0;
  drawSpriteWhitePixelsAsBlack(bootLogo24x12, 0, BOOT_LOGO_X + dx, BOOT_LOGO_Y + dy);
}

void drawBootParticles(uint8_t frame) {
  for (uint8_t i = 0; i < 22; i++) {
    uint8_t fallSpeed = 1 + (i % 3);
    int16_t x = (i * 23 + frame / 2 + ((frame + i * 7) / 8) % 7) % 128;
    int16_t y = (i * 19 + frame * fallSpeed) % 72 - 8;
    if (y >= 0 && y < 64) {
      arduboy.drawPixel(x, y, BLACK);
    }
  }
}

uint16_t bootFillPixelCount(uint8_t frame) {
  uint16_t count = 0;
  for (uint8_t i = 0; i <= frame; i++) {
    if (i < framesAtGameFps(5)) {
      count += 10;
    } else if (i < framesAtGameFps(12)) {
      count += 34;
    } else {
      count += 420;
    }
  }
  return count;
}

uint16_t bootFillPixelPosition(uint16_t index) {
  uint16_t value = index * 173U + 911U;
  value ^= value >> 7;
  value *= 197U;
  value ^= value >> 5;
  return value & 8191;
}

void drawBootFillPixels(uint8_t frame) {
  uint16_t count = bootFillPixelCount(frame);
  if (count > 8192) {
    count = 8192;
  }

  for (uint16_t i = 0; i < count; i++) {
    uint16_t p = bootFillPixelPosition(i);
    arduboy.drawPixel(p & 127, p >> 7, BLACK);
  }
}

void drawBootCurtain(uint8_t frame) {
  uint8_t halfWidth = 3 + frame * 5 + (frame * frame) / 8;
  if (halfWidth > 64) {
    halfWidth = 64;
  }
  arduboy.fillRect(64 - halfWidth, 0, halfWidth * 2, 64, BLACK);
}

void playBootAnimation() {
  ledsOff();
  for (uint8_t frame = 0; frame < BOOT_TOTAL_FRAMES; frame++) {
    while (!arduboy.nextFrame()) { }
    arduboy.pollButtons();
    if (arduboy.buttonsState()) {
      break;
    }

    arduboy.fillScreen(WHITE);
    drawBootLogo(frame);
    if (frame >= BOOT_DUST_START_FRAMES) {
      drawBootParticles(frame - BOOT_DUST_START_FRAMES);
    }
    if (frame >= BOOT_FILL_START_FRAMES) {
      drawBootFillPixels(frame - BOOT_FILL_START_FRAMES);
    }
    if (frame >= BOOT_CURTAIN_START_FRAMES) {
      drawBootCurtain(frame - BOOT_CURTAIN_START_FRAMES);
    }
    arduboy.display();
  }
  arduboy.fillScreen(BLACK);
  arduboy.display();
  ledsOff();
}

void startMillEffects() {
  millFlashFrames = 36;
  sound.tone(659, 70, 880, 90, 1175, 120);
}

void updateMillLed() {
  if (millFlashFrames == 0) {
    ledsOff();
    return;
  }

  millFlashFrames--;
  if ((millFlashFrames / 4) % 2 == 0) {
    arduboy.digitalWriteRGB(RGB_ON, RGB_OFF, RGB_OFF);
  } else {
    arduboy.digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_ON);
  }
}

void drawBoardLine(uint8_t a, uint8_t b) {
  BoardPoint pa = screenPoint(a);
  BoardPoint pb = screenPoint(b);
  arduboy.drawLine(pa.x, pa.y, pb.x, pb.y, BLACK);
}

void drawClassicBoard() {
  drawBoardLine(0, 2);
  drawBoardLine(3, 5);
  drawBoardLine(6, 8);
  drawBoardLine(9, 11);
  drawBoardLine(12, 14);
  drawBoardLine(15, 17);
  drawBoardLine(18, 20);
  drawBoardLine(21, 23);

  drawBoardLine(0, 21);
  drawBoardLine(3, 18);
  drawBoardLine(6, 15);
  drawBoardLine(1, 7);
  drawBoardLine(16, 22);
  drawBoardLine(8, 17);
  drawBoardLine(5, 20);
  drawBoardLine(2, 23);
}

void drawPiece(uint8_t point, Player player) {
  BoardPoint p = screenPoint(point);
  if (player == PLAYER_ONE) {
    arduboy.fillCircle(p.x, p.y, 3, BLACK);
  } else if (player == PLAYER_TWO) {
    arduboy.fillCircle(p.x, p.y, 3, WHITE);
    arduboy.drawCircle(p.x, p.y, 3, BLACK);
  } else {
    arduboy.fillRect(p.x - 1, p.y - 1, 3, 3, BLACK);
  }
}

bool dashPixelOn(uint8_t step, uint8_t phase) {
  return ((step + phase) % 8) < 4;
}

void drawDashedHLine(int8_t x, int8_t y, uint8_t w, uint8_t phase, uint8_t color) {
  for (uint8_t i = 0; i < w; i++) {
    if (dashPixelOn(i, phase)) {
      arduboy.drawPixel(x + i, y, color);
    }
  }
}

void drawDashedVLine(int8_t x, int8_t y, uint8_t h, uint8_t phase, uint8_t color) {
  for (uint8_t i = 0; i < h; i++) {
    if (dashPixelOn(i, phase)) {
      arduboy.drawPixel(x, y + i, color);
    }
  }
}

void drawDashedRect(int8_t x, int8_t y, uint8_t w, uint8_t h, uint8_t phase, uint8_t color) {
  drawDashedHLine(x, y, w, phase, color);
  drawDashedVLine(x + w - 1, y, h, phase + w, color);
  drawDashedHLine(x, y + h - 1, w, phase + w + h, color);
  drawDashedVLine(x, y, h, phase + h, color);
}

void drawCursor() {
  uint8_t phase = animationFrame / 4;
  BoardPoint p = screenPoint(game.cursor);
  drawDashedRect(p.x - 5, p.y - 5, 11, 11, phase, BLACK);
  if (game.selected != 255) {
    BoardPoint selected = screenPoint(game.selected);
    drawDashedRect(selected.x - 6, selected.y - 6, 13, 13, phase + 4, BLACK);
  }
}

void drawHudRule(uint8_t x) {
  for (uint8_t y = 2; y < 62; y += 4) {
    arduboy.drawPixel(x, y, BLACK);
    arduboy.drawPixel(x, y + 1, BLACK);
  }
}

void drawHudPiece(uint8_t x, uint8_t y, Player player) {
  if (player == PLAYER_ONE) {
    arduboy.fillCircle(x, y, 2, BLACK);
  } else {
    arduboy.fillCircle(x, y, 2, WHITE);
    arduboy.drawCircle(x, y, 2, BLACK);
  }
}

void drawHud() {
  drawHudRule(29);
  drawHudRule(98);

  tinyfont.setCursor(HUD_LEFT_X, 2);
  tinyfont.print("STATE");
  tinyfont.setCursor(HUD_LEFT_X, 9);
  if (game.phase == PHASE_PLACING) {
    tinyfont.print("PLACE");
  } else if (game.phase == PHASE_CAPTURING) {
    tinyfont.print("CAP");
  } else if (game.phase == PHASE_GAME_OVER) {
    tinyfont.print("OVER");
  } else if (playerCanFly(game, game.currentPlayer)) {
    tinyfont.print("FLY");
  } else {
    tinyfont.print("MOVE");
  }
  tinyfont.setCursor(HUD_LEFT_X, 22);
  if (game.phase == PHASE_GAME_OVER) {
    if (game.winner == PLAYER_NONE) {
      tinyfont.print("DRAW");
    } else {
      tinyfont.print(game.winner == PLAYER_TWO ? "W" : "B");
      tinyfont.print(" WINS");
    }
  } else {
    tinyfont.print("TURN");
    drawHudPiece(HUD_LEFT_X + 4, 34, game.currentPlayer);
    tinyfont.setCursor(HUD_LEFT_X + 10, 32);
    tinyfont.print(game.currentPlayer == PLAYER_TWO ? "W" : "B");
    tinyfont.setCursor(HUD_LEFT_X, 45);
    tinyfont.print(game.phase == PHASE_CAPTURING ? "TAKE" : "TURN");
  }

  tinyfont.setCursor(HUD_RIGHT_X, 2);
  tinyfont.print("HAND");
  drawHudPiece(HUD_RIGHT_X + 4, 14, PLAYER_ONE);
  tinyfont.setCursor(HUD_RIGHT_X + 10, 12);
  tinyfont.print(game.piecesToPlace[0]);
  drawHudPiece(HUD_RIGHT_X + 4, 23, PLAYER_TWO);
  tinyfont.setCursor(HUD_RIGHT_X + 10, 21);
  tinyfont.print(game.piecesToPlace[1]);

  tinyfont.setCursor(HUD_RIGHT_X, 31);
  tinyfont.print("BOARD");
  tinyfont.setCursor(HUD_RIGHT_X, 38);
  tinyfont.print("B ");
  tinyfont.print(game.piecesOnBoard[0]);
  tinyfont.setCursor(HUD_RIGHT_X + 13, 38);
  tinyfont.print("W ");
  tinyfont.print(game.piecesOnBoard[1]);

  if (messageFrames > 0) {
    tinyfont.setCursor(HUD_RIGHT_X, 48);
    tinyfont.print(message);
  } else if (game.phase == PHASE_GAME_OVER) {
    tinyfont.setCursor(HUD_RIGHT_X, 48);
    if (game.winReason == WIN_DRAW_NO_CAPTURE) {
      tinyfont.print("NO CAP");
    } else {
      tinyfont.print(game.winReason == WIN_BY_BLOCK ? "BLOCK" : "2 MEN");
    }
  }

  tinyfont.setCursor(HUD_RIGHT_X, 57);
  tinyfont.print("A MENU");
}

void drawCenteredPanel(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  arduboy.fillRect(x, y, w, h, WHITE);
  arduboy.drawRect(x, y, w, h, BLACK);
}

const char *boardMenuTitle() {
  if (selectedBoardMenuItem == 1) {
    return "SIX MEN";
  }
  if (selectedBoardMenuItem == 2) {
    return "THREE MEN";
  }
  return "CLASSIC 9";
}

uint8_t textPixelWidth(const char *text) {
  uint8_t width = 0;
  while (*text != '\0') {
    width += 4;
    text++;
  }
  return width;
}

void drawMenuChevron(uint8_t x, uint8_t y, bool left) {
  if (left) {
    arduboy.drawLine(x + 4, y, x, y + 3, WHITE);
    arduboy.drawLine(x, y + 3, x + 4, y + 6, WHITE);
  } else {
    arduboy.drawLine(x, y, x + 4, y + 3, WHITE);
    arduboy.drawLine(x + 4, y + 3, x, y + 6, WHITE);
  }
}

void drawMenuPatterns() {
  for (uint8_t x = 4; x < 24; x += 5) {
    for (uint8_t y = 4; y < 22; y += 5) {
      arduboy.drawPixel(x, y, WHITE);
    }
  }

  for (uint8_t x = 104; x < 126; x += 4) {
    arduboy.drawLine(x, 4, x, 18, WHITE);
  }
  for (uint8_t y = 4; y < 20; y += 4) {
    arduboy.drawLine(104, y, 124, y, WHITE);
  }

  for (uint8_t x = 5; x < 28; x += 6) {
    arduboy.drawLine(x, 48, x + 3, 51, WHITE);
  }
  for (uint8_t x = 96; x < 122; x += 6) {
    arduboy.drawLine(x, 33, x + 3, 30, WHITE);
  }
}

void drawMainMenu() {
  arduboy.fillScreen(BLACK);
  tinyfont.setTextColor(WHITE);
  drawMenuPatterns();

  tinyfont.setCursor(47, 5);
  tinyfont.print("ALL MEN'S");
  tinyfont.setCursor(53, 12);
  tinyfont.print("MORRIS");
  arduboy.drawLine(30, 20, 98, 20, WHITE);

  const char *title = boardMenuTitle();
  uint8_t titleWidth = textPixelWidth(title);
  uint8_t titleX = 62 - titleWidth / 2;
  uint8_t leftX = titleX > 12 ? titleX - 12 : 2;
  uint8_t rightX = titleX + titleWidth + 12;

  tinyfont.setCursor(52, 27);
  tinyfont.print("BOARD");
  drawMenuChevron(leftX, 37, true);
  drawMenuChevron(rightX, 37, false);
  tinyfont.setCursor(titleX - 1, 38);
  tinyfont.print(title);
  if (selectedMenuItem == 0) {
    drawDashedRect(leftX - 3, 35, rightX - leftX + 11, 11, animationFrame / 5, WHITE);
  }
  if (selectedBoardMenuItem != 0) {
    tinyfont.setCursor(108, 56);
    tinyfont.print("SOON");
  }

  tinyfont.setCursor(38, 52);
  tinyfont.print("FIRST ");
  tinyfont.print(firstPlayer == PLAYER_TWO ? "WHITE" : "BLACK");
  if (selectedMenuItem == 1) {
    drawDashedRect(35, 49, 60, 11, animationFrame / 5, WHITE);
  }
}

void drawQuickMenu() {
#ifdef ALL_MENS_MORRIS_DEBUG
  drawCenteredPanel(19, 12, 90, 42);
  tinyfont.setCursor(43, 16);
#else
  drawCenteredPanel(19, 15, 90, 35);
  tinyfont.setCursor(43, 19);
#endif
  tinyfont.print("QUICK");
#ifdef ALL_MENS_MORRIS_DEBUG
  tinyfont.setCursor(27, 24);
#else
  tinyfont.setCursor(27, 27);
#endif
  tinyfont.print("UP RESET");
#ifdef ALL_MENS_MORRIS_DEBUG
  tinyfont.setCursor(27, 31);
#else
  tinyfont.setCursor(27, 34);
#endif
  tinyfont.print("LEFT REWIND");
#ifdef ALL_MENS_MORRIS_DEBUG
  tinyfont.setCursor(27, 38);
#else
  tinyfont.setCursor(27, 41);
#endif
  tinyfont.print("DOWN MENU");
#ifdef ALL_MENS_MORRIS_DEBUG
  tinyfont.setCursor(27, 45);
  tinyfont.print("RIGHT DBG");
#endif
}

void drawConfirm() {
  drawCenteredPanel(18, 20, 92, 25);
  tinyfont.setCursor(32, 25);
  tinyfont.print(confirmAction == CONFIRM_RESET ? "RESET GAME?" : "MAIN MENU?");
  tinyfont.setCursor(27, 36);
  tinyfont.print("B YES  LEFT NO");
}

void drawGame() {
  tinyfont.setTextColor(BLACK);
  drawClassicBoard();
  for (uint8_t i = 0; i < MORRIS_POINT_COUNT; i++) {
    drawPiece(i, game.points[i]);
  }
  drawCursor();
  drawHud();
  if (confirmAction != CONFIRM_NONE) {
    drawConfirm();
  } else if (arduboy.pressed(A_BUTTON)) {
    drawQuickMenu();
  }
}

void drawScene() {
  if (scene == SCENE_MAIN_MENU) {
    drawMainMenu();
    return;
  }

  drawGame();
}

void handleMainMenuInput() {
  if (arduboy.justPressed(UP_BUTTON)) {
    selectedMenuItem = selectedMenuItem == 0 ? MENU_ITEM_COUNT - 1 : selectedMenuItem - 1;
    sound.tone(523, 35);
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
    selectedMenuItem = selectedMenuItem == MENU_ITEM_COUNT - 1 ? 0 : selectedMenuItem + 1;
    sound.tone(392, 35);
  }
  if (arduboy.justPressed(LEFT_BUTTON)) {
    if (selectedMenuItem == 0) {
      selectedBoardMenuItem = selectedBoardMenuItem == 0 ? BOARD_MENU_COUNT - 1 : selectedBoardMenuItem - 1;
    } else {
      firstPlayer = firstPlayer == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE;
    }
    sound.tone(494, 35);
  }
  if (arduboy.justPressed(RIGHT_BUTTON)) {
    if (selectedMenuItem == 0) {
      selectedBoardMenuItem = (selectedBoardMenuItem + 1) % BOARD_MENU_COUNT;
    } else {
      firstPlayer = firstPlayer == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE;
    }
    sound.tone(587, 35);
  }
  if (arduboy.justPressed(B_BUTTON)) {
    if (selectedMenuItem == 0 && selectedBoardMenuItem == 0) {
      startMatch();
      sound.tone(660, 45, 880, 70);
    } else if (selectedMenuItem == 1) {
      firstPlayer = firstPlayer == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE;
      setMessage(firstPlayer == PLAYER_TWO ? "WHITE" : "BLACK");
      sound.tone(784, 45);
    } else {
      setMessage("SOON");
      sound.tone(180, 90);
    }
  }
}

void handleConfirmInput() {
  if (arduboy.justPressed(B_BUTTON)) {
    if (confirmAction == CONFIRM_RESET) {
      startMatch();
      setMessage("RESET");
      sound.tone(262, 45, 392, 65);
    } else if (confirmAction == CONFIRM_MAIN_MENU) {
      scene = SCENE_MAIN_MENU;
      confirmAction = CONFIRM_NONE;
      hasUndo = false;
      setMessage("");
      sound.tone(392, 45, 262, 65);
    }
    return;
  }

  if (arduboy.justPressed(LEFT_BUTTON)) {
    confirmAction = CONFIRM_NONE;
    setMessage("");
    sound.tone(220, 40);
  }
}

void handleQuickMenuInput() {
  if (!arduboy.pressed(A_BUTTON)) {
    return;
  }

  if (arduboy.justPressed(UP_BUTTON)) {
    confirmAction = CONFIRM_RESET;
    setMessage("");
    sound.tone(523, 35);
  } else if (arduboy.justPressed(LEFT_BUTTON)) {
    if (hasUndo) {
      game = undoGame;
      hasUndo = false;
      setMessage("REWIND");
      sound.tone(440, 45, 330, 70);
    } else {
      setMessage("NO UNDO");
      sound.tone(160, 80);
    }
  } else if (arduboy.justPressed(DOWN_BUTTON)) {
    confirmAction = CONFIRM_MAIN_MENU;
    setMessage("");
    sound.tone(330, 35);
#ifdef ALL_MENS_MORRIS_DEBUG
  } else if (arduboy.justPressed(RIGHT_BUTTON)) {
    loadDebugScenario();
#endif
  }
}

void handleInput() {
  if (scene == SCENE_MAIN_MENU) {
    handleMainMenuInput();
    return;
  }

  if (confirmAction != CONFIRM_NONE) {
    handleConfirmInput();
    return;
  }

  if (arduboy.pressed(A_BUTTON)) {
    handleQuickMenuInput();
    return;
  }

  if (arduboy.justPressed(LEFT_BUTTON)) {
    moveCursorToward(-1, 0);
  }
  if (arduboy.justPressed(RIGHT_BUTTON)) {
    moveCursorToward(1, 0);
  }
  if (arduboy.justPressed(UP_BUTTON)) {
    moveCursorToward(0, -1);
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
    moveCursorToward(0, 1);
  }
  if (arduboy.justPressed(B_BUTTON)) {
    GamePhase phaseBeforeAction = game.phase;
    MorrisGameState beforeAction = game;
    bool moved = applyPrimaryAction(game);
    if (!moved) {
      setMessage("NOPE");
      sound.tone(120, 80);
    } else if (gameStateChanged(beforeAction, game)) {
      undoGame = beforeAction;
      hasUndo = true;
    }

    if (!moved) {
      return;
    } else if (game.phase == PHASE_GAME_OVER) {
      if (game.winner == PLAYER_NONE) {
        setMessage("DRAW");
        sound.tone(392, 80, 330, 80, 262, 140);
      } else {
        setMessage(game.winner == PLAYER_ONE ? "B WIN" : "W WIN");
        sound.tone(523, 80, 659, 80, 1047, 160);
      }
    } else if (game.lastMoveMadeMill) {
      setMessage("MILL!");
      startMillEffects();
    } else if (phaseBeforeAction == PHASE_CAPTURING) {
      setMessage("TAKEN");
      sound.tone(330, 60, 220, 80);
    } else {
      setMessage("");
      sound.tone(440, 30);
    }
  }
}
}

void gameSetup() {
  arduboy.beginDoFirst();
  arduboy.waitNoButtons();
  ledsOff();
  arduboy.setFrameRate(GAME_FPS);
  arduboy.setTextColor(BLACK);
  arduboy.audio.begin();
  tinyfont.setTextColor(BLACK);
  resetMorrisGame(game);
  playBootAnimation();
}

void gameLoop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  animationFrame++;
  arduboy.pollButtons();
  handleInput();
  updateMillLed();
  if (messageFrames > 0) {
    messageFrames--;
  }

  arduboy.fillScreen(WHITE);
  drawScene();
  arduboy.display();
}
