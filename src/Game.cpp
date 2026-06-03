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

MorrisGameState game;
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
  tinyfont.print(game.phase == PHASE_PLACING ? "PLACE" : "MOVE");
  tinyfont.setCursor(HUD_X, 27);
  tinyfont.print("P");
  tinyfont.print(game.currentPlayer == PLAYER_ONE ? "1" : "2");
  tinyfont.print(" TURN");

  tinyfont.setCursor(HUD_X, 39);
  tinyfont.print("P1 ");
  tinyfont.print(game.piecesToPlace[0]);
  tinyfont.setCursor(HUD_X, 46);
  tinyfont.print("P2 ");
  tinyfont.print(game.piecesToPlace[1]);

  tinyfont.setCursor(HUD_X, 57);
  tinyfont.print("A RST B OK");

  if (messageFrames > 0) {
    tinyfont.setCursor(HUD_X, 33);
    tinyfont.print(message);
  }
}

void drawGame() {
  drawClassicBoard();
  for (uint8_t i = 0; i < MORRIS_POINT_COUNT; i++) {
    drawPiece(i, game.points[i]);
  }
  drawCursor();
  drawHud();
}

void handleInput() {
  if (arduboy.justPressed(A_BUTTON)) {
    resetMorrisGame(game);
    setMessage("RESET");
  }
  if (arduboy.justPressed(LEFT_BUTTON) || arduboy.justPressed(UP_BUTTON)) {
    advanceCursor(game, -1);
  }
  if (arduboy.justPressed(RIGHT_BUTTON) || arduboy.justPressed(DOWN_BUTTON)) {
    advanceCursor(game, 1);
  }
  if (arduboy.justPressed(B_BUTTON)) {
    bool moved = applyPrimaryAction(game);
    if (!moved) {
      setMessage("NOPE");
    } else if (game.lastMoveMadeMill) {
      setMessage("MILL!");
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
  drawGame();
  arduboy.display();
}
