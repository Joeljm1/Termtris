#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
using namespace std::chrono_literals;

char tetro = 'X';

std::string tetromino[7];
int feildWidth = 12;
int feildHeight = 18;

long score = 0;  // TODO: on 4 give more points
int ScreenWidth = 50;
int ScreenHeight = 30;
unsigned char* pFeild = nullptr;
struct termios ogTermios;

void die(const char* s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &ogTermios) == -1) {
    die("tcsetattr");
  }
  std::cout << "SCORE: " << score;
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &ogTermios) == -1) {
    die("tcgetattr");
  }
  atexit(disableRawMode);  // error handling?????
  struct termios raw = ogTermios;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cflag &= ~CSIZE;
  raw.c_cflag |= CS8;
  // raw.c_oflag &= ~(OPOST); //TODO: may be do this and manually ocntrol new
  // lines later
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // TODO: even if one is succesful tcsetattr will output success so
  //  need to
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

enum editorkey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DELETE_KEY,
};
int editorReadKey() {
  int nread;
  char c;
  // while (((nread = read(STDIN_FILENO, &c, 1)) != 1)) {
  //   if (nread == -1 && nread != EAGAIN) {
  //     die("read");
  //   }
  // }
  nread = read(STDIN_FILENO, &c, 1);
  if (nread != 1) {
    return -1;
  }

  if (nread == -1 && nread != EAGAIN) {
    die("read");
  }
  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return '\x1b';
    }

    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return '\x1b';
    }
    // arrow keys handling
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) {
          return '\x1b';
        }
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1':
              return HOME_KEY;
            case '3':
              return DELETE_KEY;
            case '4':
              return END_KEY;
            case '5':
              return PAGE_UP;

            case '6': {
              return PAGE_DOWN;
            }
            case '7':
              return HOME_KEY;  // Home jeys have different value for different
                                // os
            case '8':
              return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A':
            return ARROW_UP;
          case 'B':
            return ARROW_DOWN;
          case 'C':
            return ARROW_RIGHT;
          case 'D':
            return ARROW_LEFT;
          case 'H':
            return HOME_KEY;
          case 'F':
            return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
      }
    }
    return '\x1b';
  } else {
    return c;
  }
  return c;
}

void init() {
  enableRawMode();
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
      if (y == feildHeight - 1) {
        if(x==0||x==feildWidth-1){
        pFeild[y * feildWidth + x] = 10;
        }else{
        pFeild[y * feildWidth + x] = 11;
        }
      } else if (x == 0 || x == feildWidth - 1) {
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

void printChar(char c) {
  std::string s = "█";
  switch (c) {
      // " ABCDEFG=#"
    case 'A':
      std::cout << "\e[0;101m ";
      break;
    case 'B':
      std::cout << "\e[0;102m ";
      break;
    case 'C':
      std::cout << "\e[0;103m ";
      break;
    case 'D':
      std::cout << "\e[0;104m ";
      break;
    case 'E':
      std::cout << "\e[0;105m ";
      break;
    case 'F':
      std::cout << "\e[0;106m ";
      break;
    case 'G':
      std::cout << "\e[0;107m ";
      break;
    case '=':
      std::cout << "✦";
      break;
    case '#':
      std::cout << "\e[0;37m║";
      break;
    case '-':
      std::cout << "\e[0;37m╚";
      break;
    case '(':
      std::cout << "\e[0;37m═";
      break;
    default:
      std::cout << "\e[0;37m" << c;
  }
}

void displayScreen(char* screen) {
  for (int y = 0; y < ScreenHeight; y++) {
    for (int x = 0; x < ScreenWidth; x++) {
      printChar(screen[x + ScreenWidth * y]);
    }
    std::cout << "\n";
  }

  moveUp(ScreenHeight);
}

void printStringToScreen(char* screen, const std::string& str, int row) {
  assert(feildWidth + 10 + str.size() < ScreenWidth && " string to long");
  int scoreIdx = feildWidth + 10;
  for (auto& c : str) {
    screen[row * ScreenWidth + scoreIdx] = c;
    scoreIdx++;
  }
}

void printSlowly(char* screen, const std::string& str, int row) {
  std::string s = "";
  for (auto c : str) {
    s += c;
    printStringToScreen(screen, s, 10);
    std::this_thread::sleep_for(50ms);
    displayScreen(screen);
  }
}

int main() {
  std::cout << "\033[?25l";  // diable cursor
  init();

  // random seed
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, 6);

  char* screen = new char[ScreenWidth * ScreenHeight];

  // GAME LOGIC STUFF
  bool gameOver = false;
  int currPeice = dist(gen);
  int currRotation = 0;
  int currX = feildWidth / 2;
  int currY = 0;
  bool rotatePress = false;
  int speed = 10;
  int speedCounter = 0;
  bool forceDown = false;
  bool needHelp = false;
  std::vector<int> completeLines;

  while (!gameOver) {
    // GAME TIMING
    std::this_thread::sleep_for(20ms);
    speedCounter += std::max(score / 10, 1l);
    if (speedCounter >= speed) {
      forceDown = true;
    } else {
      forceDown = false;
    }

    // INPUT
    int inp = editorReadKey();
    switch (inp) {
      case ARROW_LEFT:
        if (DoesPeiceFit(currPeice, currRotation, currX - 1, currY)) {
          currX--;
        }
        break;
      case ARROW_RIGHT:
        if (DoesPeiceFit(currPeice, currRotation, currX + 1, currY)) {
          currX++;
        }
        break;
      case ARROW_UP:
        if (!rotatePress &&
            DoesPeiceFit(currPeice, currRotation + 1, currX, currY)) {
          rotatePress = true;
          currRotation++;
        }
        break;
      case ARROW_DOWN:
        if (DoesPeiceFit(currPeice, currRotation, currX, currY + 1)) {
          currY++;
        }
        break;
      case '?':
        needHelp = !needHelp;
        break;
      case 'q':
        exit(0);
      case -1:
        rotatePress = false;
        break;
    }

    // GAME LOGIC
    if (forceDown) {
      if (DoesPeiceFit(currPeice, currRotation, currX, currY + 1)) {
        currY++;
      } else {
        // lock in feild
        for (int py = 0; py < 4; py++) {
          for (int px = 0; px < 4; px++) {
            if (tetromino[currPeice][Rotate(px, py, currRotation)] == 'X')
              pFeild[((py + currY) * feildWidth) + (px + currX)] =
                  currPeice + 1;
          }
        }
        // check lines only last termonio lines
        for (int py = 0; py < 4; py++) {
          if (currY + py < feildHeight - 1) {
            bool line = true;
            for (int px = 1; px < feildWidth - 1; px++) {
              if (pFeild[(currY + py) * feildWidth + px] == 0) {
                line = false;
              }
            }
            if (line) {
              score += 1;  // TODO: on 4 give more points
              completeLines.push_back(currY + py);
              for (int px = 1; px < feildWidth - 1; px++) {
                pFeild[(currY + py) * feildWidth + px] = 8;
              }
            }
          }
        }

        // chose next peice
        currX = feildWidth / 2;
        currY = 0;
        currRotation = 0;
        currPeice = dist(gen);
        //
        // if peice cant fit game over
        gameOver = !DoesPeiceFit(currPeice, currRotation, currX, currY);
      }

      speedCounter = 0;
    }

    // RENDER OUTPUT

    // DRAW FEILD
    memset(screen, ' ', ScreenWidth * ScreenHeight);
    for (int x = 0; x < feildWidth; x++) {
      for (int y = 0; y < feildHeight; y++) {
        screen[(y + 2) * ScreenWidth + (x + 2)] =
            " ABCDEFG=#-("[pFeild[y * feildWidth + x]];
      }
    }

    std::string scoreText = "Score : ";
    scoreText += std::to_string(score);
    printStringToScreen(screen, scoreText, 3);
    if (!needHelp) {
      printStringToScreen(screen, "Press ? for help", 5);
    } else {
      printStringToScreen(screen, "Press up for rotating", 5);
      printStringToScreen(screen, "Press down to go down", 6);
      printStringToScreen(screen, "Press left/right to move", 7);
      printStringToScreen(screen, "Press q to quit", 8);
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
    if (!completeLines.empty()) {
      if (completeLines.size() == 4) {
        printSlowly(screen, "TETRIS", 10);
        score += 8;
      }

      std::vector<int> tmp{};
      for (int py = feildHeight - 2; py >= 0; py--) {
        if (std::find(completeLines.begin(), completeLines.end(), py) ==
            completeLines.end()) {
          tmp.push_back(py);
        }
      }
      int py = feildHeight - 2;
      for (int tmpY = 0; tmpY < tmp.size(); tmpY++, py--) {
        for (int px = 1; px < feildWidth - 1; px++) {
          pFeild[py * feildWidth + px] = pFeild[tmp[tmpY] * feildWidth + px];
        }
      }
      for (; py >= 0; py--) {
        for (int px = 1; px < feildWidth - 1; px++) {
          pFeild[py * feildWidth + px] = 0;
        }
      }
      completeLines.clear();
    }

    // DISPLAY FRAME
    displayScreen(screen);
  }
}
