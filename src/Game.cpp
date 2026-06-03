#include "AllMensMorrisGame.h"

#include "Board.h"
#include "Rules.h"

#include <Arduboy2.h>
#include <Tinyfont.h>

namespace {
Arduboy2 arduboy;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint8_t GAME_FPS = 30;
constexpr uint8_t BOARD_OFFSET_X = 2;
constexpr uint8_t BOARD_OFFSET_Y = 0;
constexpr uint8_t HUD_X = 72;
constexpr uint8_t NO_POINT = 255;

enum AppScene : uint8_t {
  SCENE_MAIN_MENU,
  SCENE_PLAYING,
};

enum ConfirmAction : uint8_t {
  CONFIRM_NONE,
  CONFIRM_RESET,
  CONFIRM_MAIN_MENU,
};

MorrisGameState game;
MorrisGameState undoGame;
AppScene scene = SCENE_MAIN_MENU;
ConfirmAction confirmAction = CONFIRM_NONE;
uint8_t selectedMenuItem = 0;
bool hasUndo = false;
uint8_t messageFrames = 0;
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
      || before.lastMoveMadeMill != after.lastMoveMadeMill) {
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
  hasUndo = false;
  confirmAction = CONFIRM_NONE;
  scene = SCENE_PLAYING;
  setMessage("");
}

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
    arduboy.fillCircle(p.x, p.y, 2, WHITE);
    arduboy.drawCircle(p.x, p.y, 2, BLACK);
  }
}

void drawCursor() {
  BoardPoint p = screenPoint(game.cursor);
  arduboy.drawRect(p.x - 5, p.y - 5, 11, 11, BLACK);
  if (game.selected != 255) {
    BoardPoint selected = screenPoint(game.selected);
    arduboy.drawRect(selected.x - 6, selected.y - 6, 13, 13, BLACK);
  }
}

void drawHud() {
  tinyfont.setCursor(HUD_X, 2);
  tinyfont.print("ALL MEN");
  tinyfont.setCursor(HUD_X, 8);
  tinyfont.print("MORRIS");

  tinyfont.setCursor(HUD_X, 20);
  if (game.phase == PHASE_PLACING) {
    tinyfont.print("PLACE");
  } else if (game.phase == PHASE_CAPTURING) {
    tinyfont.print("CAPTURE");
  } else if (game.phase == PHASE_GAME_OVER) {
    tinyfont.print("GAME OVER");
  } else if (playerCanFly(game, game.currentPlayer)) {
    tinyfont.print("FLY");
  } else {
    tinyfont.print("MOVE");
  }
  tinyfont.setCursor(HUD_X, 27);
  if (game.phase == PHASE_GAME_OVER) {
    tinyfont.print("P");
    tinyfont.print(game.winner == PLAYER_ONE ? "1" : "2");
    tinyfont.print(" WINS");
  } else {
    tinyfont.print("P");
    tinyfont.print(game.currentPlayer == PLAYER_ONE ? "1" : "2");
    tinyfont.print(game.phase == PHASE_CAPTURING ? " TAKE" : " TURN");
  }

  tinyfont.setCursor(HUD_X, 39);
  tinyfont.print("P1 ");
  tinyfont.print(game.piecesToPlace[0]);
  tinyfont.setCursor(HUD_X, 46);
  tinyfont.print("P2 ");
  tinyfont.print(game.piecesToPlace[1]);

  tinyfont.setCursor(HUD_X, 57);
  tinyfont.print("A MENU B OK");

  if (messageFrames > 0) {
    tinyfont.setCursor(HUD_X, 33);
    tinyfont.print(message);
  } else if (game.phase == PHASE_GAME_OVER) {
    tinyfont.setCursor(HUD_X, 33);
    tinyfont.print(game.winReason == WIN_BY_BLOCK ? "BLOCK" : "2 MEN");
  }
}

void drawCenteredPanel(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  arduboy.fillRect(x, y, w, h, WHITE);
  arduboy.drawRect(x, y, w, h, BLACK);
}

void drawMainMenu() {
  tinyfont.setCursor(28, 8);
  tinyfont.print("ALL MEN'S");
  tinyfont.setCursor(36, 15);
  tinyfont.print("MORRIS");

  tinyfont.setCursor(23, 28);
  tinyfont.print(selectedMenuItem == 0 ? "> CLASSIC 9" : "  CLASSIC 9");
  tinyfont.setCursor(23, 36);
  tinyfont.print(selectedMenuItem == 1 ? "> SIX MEN" : "  SIX MEN");
  tinyfont.setCursor(23, 44);
  tinyfont.print(selectedMenuItem == 2 ? "> THREE MEN" : "  THREE MEN");

  tinyfont.setCursor(17, 56);
  tinyfont.print(selectedMenuItem == 0 ? "B START" : "B SOON");

  if (messageFrames > 0) {
    tinyfont.setCursor(77, 56);
    tinyfont.print(message);
  }
}

void drawQuickMenu() {
  drawCenteredPanel(19, 15, 90, 35);
  tinyfont.setCursor(43, 19);
  tinyfont.print("QUICK");
  tinyfont.setCursor(27, 27);
  tinyfont.print("UP RESET");
  tinyfont.setCursor(27, 34);
  tinyfont.print("LEFT REWIND");
  tinyfont.setCursor(27, 41);
  tinyfont.print("DOWN MENU");
}

void drawConfirm() {
  drawCenteredPanel(18, 20, 92, 25);
  tinyfont.setCursor(32, 25);
  tinyfont.print(confirmAction == CONFIRM_RESET ? "RESET GAME?" : "MAIN MENU?");
  tinyfont.setCursor(27, 36);
  tinyfont.print("B YES  LEFT NO");
}

void drawGame() {
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
    selectedMenuItem = selectedMenuItem == 0 ? 2 : selectedMenuItem - 1;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
    selectedMenuItem = selectedMenuItem == 2 ? 0 : selectedMenuItem + 1;
  }
  if (arduboy.justPressed(B_BUTTON)) {
    if (selectedMenuItem == 0) {
      startMatch();
    } else {
      setMessage("SOON");
    }
  }
}

void handleConfirmInput() {
  if (arduboy.justPressed(B_BUTTON)) {
    if (confirmAction == CONFIRM_RESET) {
      startMatch();
      setMessage("RESET");
    } else if (confirmAction == CONFIRM_MAIN_MENU) {
      scene = SCENE_MAIN_MENU;
      confirmAction = CONFIRM_NONE;
      hasUndo = false;
      setMessage("");
    }
    return;
  }

  if (arduboy.justPressed(LEFT_BUTTON)) {
    confirmAction = CONFIRM_NONE;
    setMessage("");
  }
}

void handleQuickMenuInput() {
  if (!arduboy.pressed(A_BUTTON)) {
    return;
  }

  if (arduboy.justPressed(UP_BUTTON)) {
    confirmAction = CONFIRM_RESET;
    setMessage("");
  } else if (arduboy.justPressed(LEFT_BUTTON)) {
    if (hasUndo) {
      game = undoGame;
      hasUndo = false;
      setMessage("REWIND");
    } else {
      setMessage("NO UNDO");
    }
  } else if (arduboy.justPressed(DOWN_BUTTON)) {
    confirmAction = CONFIRM_MAIN_MENU;
    setMessage("");
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
    } else if (gameStateChanged(beforeAction, game)) {
      undoGame = beforeAction;
      hasUndo = true;
    }

    if (!moved) {
      return;
    } else if (game.phase == PHASE_GAME_OVER) {
      setMessage(game.winner == PLAYER_ONE ? "P1 WIN" : "P2 WIN");
    } else if (game.lastMoveMadeMill) {
      setMessage("MILL!");
    } else if (phaseBeforeAction == PHASE_CAPTURING) {
      setMessage("TAKEN");
    } else {
      setMessage("");
    }
  }
}
}

void gameSetup() {
  arduboy.begin();
  arduboy.setFrameRate(GAME_FPS);
  arduboy.setTextColor(BLACK);
  tinyfont.setTextColor(BLACK);
  resetMorrisGame(game);
}

void gameLoop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();
  handleInput();
  if (messageFrames > 0) {
    messageFrames--;
  }

  arduboy.fillScreen(WHITE);
  drawScene();
  arduboy.display();
}
