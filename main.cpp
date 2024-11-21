#include <conio.h>
#include <windows.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

const int width = 40;
const int height = 20;

int speed = 150;  // ms
const int speedIncrement = 10;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

char fruitC = '\235';
char headC = '@';
char bodyC = 'o';
char tailC = '*';
char wallC = '=';
char sideWallC = '|';

int x, y, fruitX, fruitY, score;
bool gameOver;
vector<pair<int, int>> snakeBody;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 + 0x9e3779b9 + (hash2 << 6) + (hash2 >> 2);
    }
};
unordered_set<pair<int, int>, pair_hash> snakeOccupied;

// move cursor in the console
void gotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void clearScreen() {
    system("cls");
}

void drawWalls() {
    // red code
    cout << "\033[31m";

    // draw the top and bottom walls
    for (int i = 0; i <= width; i++) {
        cout << "\033[0;" << i + 1 << "H" << wallC;
        cout << "\033[" << height + 2 << ";" << i + 1 << "H" << wallC;
    }

    // draw side walls
    for (int i = 1; i <= height; i++) {
        cout << "\033[" << i + 1 << ";1H" << sideWallC;
        cout << "\033[" << i + 1 << ";" << width + 1 << "H" << sideWallC;
    }

    cout << "\033[0m";
}

void setup() {
    gameOver = false;
    dir = STOP;
    speed = 150;  // Reset speed

    x = rand() % (width - 2) + 1;
    y = rand() % (height - 2) + 1;

    // initialize fruit position, ensuring it doesn't overlap with the snake's
    do {
        fruitX = rand() % (width - 2) + 1;
        fruitY = rand() % (height - 2) + 1;
    } while (fruitX == x && fruitY == y);

    score = 0;

    // start with a snake of length 1
    snakeBody.clear();
    snakeOccupied.clear();
    snakeBody.push_back({x, y});
    snakeOccupied.insert({x, y});

    clearScreen();  // Clear the screen
    drawWalls();
}

void draw() {
    // Draw the fruit
    gotoXY(fruitX, fruitY);
    cout << fruitC;

    // head
    gotoXY(snakeBody[0].first, snakeBody[0].second);
    cout << headC;

    // body and tail
    for (size_t i = 1; i < snakeBody.size(); ++i) {
        gotoXY(snakeBody[i].first, snakeBody[i].second);
        cout << ((i == snakeBody.size() - 1) ? tailC : bodyC);
    }

    // print out the score, so users know how good they are
    gotoXY(0, height + 2);
    cout << "Score: " << score << "\n";
}

// capture user input without blocking
void input() {
    while (_kbhit()) {  // Keep looping while there's input in the buffer
        int ch = _getch();

        // If it's an extended key, handle it
        if (ch == 0 || ch == 224) {
            ch = _getch();
        }

        switch (ch) {
            case 'w':
            case 72:  // Mũi tên lên
                if (dir != DOWN) dir = UP;
                return;
            case 's':
            case 80:  // Mũi tên xuống
                if (dir != UP) dir = DOWN;
                return;
            case 'a':
            case 75:  // Mũi tên trái
                if (dir != RIGHT) dir = LEFT;
                return;
            case 'd':
            case 77:  // Mũi tên phải
                if (dir != LEFT) dir = RIGHT;
                return;
            case 'p':  // Pause the game
                while (true) {
                    if (_kbhit() && _getch() == 'p') {
                        return;  // Resume the game
                    }
                }
            default:
                // Ignore other keys and continue looping
                break;
        }
    }
}

void moveSnake(Direction dir) {
    switch (dir) {
        case LEFT:
            x--;
            break;
        case RIGHT:
            x++;
            break;
        case UP:
            y--;
            break;
        case DOWN:
            y++;
            break;
        default:
            break;
    }

    // remove tail
    auto tail = snakeBody.back();
    gotoXY(tail.first, tail.second);
    cout << " ";
    snakeOccupied.erase(tail);

    // the new head of the snake is at (x, y) based on the direction, shifting
    // the rest of the body to the right
    rotate(snakeBody.rbegin(), snakeBody.rbegin() + 1, snakeBody.rend());

    if (snakeOccupied.count({x, y})) {
        gameOver = true;
    }

    snakeOccupied.insert({x, y});
    snakeBody[0] = {x, y};
}

void spawnNewFruit() {
    do {
        fruitX = (rand() % (width - 2)) + 1;  // avoids edge positions
        fruitY = (rand() % (height - 2)) + 1;
    } while (snakeOccupied.count({fruitX, fruitY}));
}

// update game state, check if the game is over.
void updateGameState() {
    moveSnake(dir);

    // snake has crossed the line
    if (x >= width || x <= 0 || y >= height + 1 || y <= 0) {
        gameOver = true;
    }

    // eat fruit
    if (x == fruitX && y == fruitY) {
        score += 10;
        spawnNewFruit();

        snakeOccupied.insert({x, y});
        snakeBody.push_back({x, y});

        // increase the speed as the snake grows.
        if (speed > 50) speed -= speedIncrement;
    }
}

// sleep and adjust speed
void sleepGameSpeed() {
    if (dir == UP || dir == DOWN) {
        Sleep(speed * 1.5);  // slower for vertical movement
    } else {
        Sleep(speed);  // normal speed for horizontal movement
    }
}

void startGame() {
    hideCursor();
    setup();
    draw();

    // wait for user to press the first key to start
    bool validKeyPressed = false;
    while (!validKeyPressed) {
        int ch = _getch();
        switch (ch) {
            case 'w':
            case 72:
                dir = UP;
                validKeyPressed = true;
                break;
            case 's':
            case 80:
                dir = DOWN;
                validKeyPressed = true;
                break;
            case 'a':
            case 75:
                dir = LEFT;
                validKeyPressed = true;
                break;
            case 'd':
            case 77:
                dir = RIGHT;
                validKeyPressed = true;
                break;
        }
    }

    while (!gameOver) {
        input();
        updateGameState();
        draw();
        sleepGameSpeed();
    }

    cout << "Game Over! Your final score was: " << score << endl;
}

int main() {
    char option;

    do {
        startGame();
        cout << "Press 'r' to play again or 'q' to quit: ";
        while (true) {
            option = _getch();
            if (option == 'r' || option == 'q') {
                break;
            }
        }
    } while (option == 'r');

    cout << "Thanks for playing!" << endl;
    return 0;
}
