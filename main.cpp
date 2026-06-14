#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
using namespace std::chrono_literals;

char tetro = 'X';

std::string tetromino[7];
int feildWidth = 12;
int feildHeight = 18;

int ScreenWidth = 80;
int ScreenHeight = 30;
unsigned char* pFeild = nullptr;

void init() {
  // create assets
  tetromino[0].append("..X.");
  tetromino[0].append("..X.");
  tetromino[0].append("..X.");
  tetromino[0].append("..X.");

  tetromino[1].append(".X..");
  tetromino[1].append(".XX.");
  tetromino[1].append("..X.");
  tetromino[1].append("....");

  tetromino[2].append("..X.");
  tetromino[2].append(".XX.");
  tetromino[2].append(".X..");
  tetromino[2].append("....");

  tetromino[3].append("....");
  tetromino[3].append(".XX.");
  tetromino[3].append(".XX.");
  tetromino[3].append("....");

  tetromino[4].append("..X.");
  tetromino[4].append(".XX.");
  tetromino[4].append("..X.");
  tetromino[4].append("....");

  tetromino[5].append("....");
  tetromino[5].append(".XX.");
  tetromino[5].append("..X.");
  tetromino[5].append("..X.");

  tetromino[6].append("....");
  tetromino[6].append(".XX.");
  tetromino[6].append(".X..");
  tetromino[6].append(".X..");
  pFeild = new unsigned char[feildHeight * feildWidth];
  for (int x = 0; x < feildWidth; x++) {
    for (int y = 0; y < feildHeight; y++) {
      if (x == 0 || x == feildWidth - 1 || y == feildHeight - 1) {
        pFeild[y * feildWidth + x] = 9;
      } else {
        pFeild[y * feildWidth + x] = 0;
      }
    }
  }
}
int Rotate(int px, int py, int r) {
  switch (r % 4) {
    case 0:
      return py * 4 + px;
    case 1:
      return 12 + py - (px * 4);
    case 2:
      return 15 - (py * 4) - px;
    case 3:
      return 3 - py + (px * 4);
  }
  return -1;  // just to remove warning
}

bool DoesPeiceFit(int tetriminoNo, int rotation, int posX, int posY) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      int cellId = Rotate(px, py, rotation);                 // in the tetromino
      int feildId = (posY + py) * feildWidth + (posX + px);  // in the feild;
      if (tetromino[tetriminoNo][cellId] == tetro && pFeild[feildId] != 0) {
        return false;
      }
    }
  }
  return true;
}

void moveUp(int n) { std::cout << "\033[" << n << "A"; }
int main() {
  std::cout << "\033[?25l";  // diable cursor
  init();

  char* screen = new char[ScreenWidth * ScreenHeight];

  // GAME LOGIC STUFF
  bool gameOver = false;
  int currPeice = 0;
  int currRotation = 0;
  int currX = feildWidth / 2;
  int currY = 0;

  while (!gameOver) {
    // GAME TIMING
    std::this_thread::sleep_for(50ms);

    // INPUT
    // TODO: get input for left,right,rotate,down (leftArrow,rightArrow,upArrow,downArrow)

    // GAME LOGIC

    // RENDER OUTPUT

    // DRAW FEILD
    memset(screen, ' ', ScreenWidth * ScreenHeight);
    for (int x = 0; x < feildWidth; x++) {
      for (int y = 0; y < feildHeight; y++) {
        screen[(y + 2) * ScreenWidth + (x + 2)] =
            " ABCDEFG=#"[pFeild[y * feildWidth + x]];
      }
    }

    // DRAW CURR PEICE
    for (int py = 0; py < 4; py++) {
      for (int px = 0; px < 4; px++) {
        if (tetromino[currPeice][Rotate(px, py, currRotation)] == tetro) {
          screen[(currY + py + 2) * ScreenWidth + (currX + px + 2)] =
              currPeice + 'A';
        }
      }
    }

    // DISPLAY FRAME
    for (int y = 0; y < ScreenHeight; y++) {
      for (int x = 0; x < ScreenWidth; x++) {
        std::cout << screen[x + ScreenWidth * y];
      }
      std::cout << "\n";
    }

    moveUp(ScreenHeight);
  }
}
