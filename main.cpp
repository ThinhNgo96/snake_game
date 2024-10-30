#include <conio.h>
#include <windows.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

const int width = 50;
const int height = 25;

int speed = 150;  // ms
const int speedIncrement = 10;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

int x, y, fruitX, fruitY, score;
bool gameOver;
vector<pair<int, int>> snakeBody;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ (hash2 << 1);  // Combine the hashes
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

void drawWalls() {
    // draw the top and bottom walls
    for (int i = 0; i < width + 1; i++) {
        gotoXY(i, 0);
        cout << "#";
    }
    for (int i = 0; i < width + 1; i++) {
        gotoXY(i, height + 1);
        cout << "#";
    }

    // draw side walls
    for (int i = 1; i < height + 1; i++) {
        gotoXY(0, i);
        cout << "#";
        gotoXY(width, i);
        cout << "#";
    }
}

void setup() {
    gameOver = false;

    do {
        x = rand() % (width - 2) + 1;   // Range from 1 to width-2
        y = rand() % (height - 2) + 1;  // Range from 1 to height-2
    } while (x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1);

    // Initialize fruit position, ensuring it doesn't overlap with the snake's
    // initial position
    do {
        fruitX = rand() % (width - 2) + 1;   // Range from 1 to width-2
        fruitY = rand() % (height - 2) + 1;  // Range from 1 to height-2
    } while (fruitX == x && fruitY == y);

    score = 0;

    // Start with a snake of length 1
    snakeBody.reserve(width * height);
    snakeBody.clear();
    snakeBody.push_back({x, y});
    snakeOccupied.insert({x, y});

    drawWalls();
}

void draw() {
    // Draw the fruit
    gotoXY(fruitX, fruitY);
    cout << "@";

    // head
    gotoXY(snakeBody[0].first, snakeBody[0].second);
    cout << "0";

    // tail
    if (snakeBody.size() > 1) {
        // replace old head with "o"
        gotoXY(snakeBody[1].first, snakeBody[1].second);
        cout << "o";

        int tailX = snakeBody.back().first;
        int tailY = snakeBody.back().second;
        gotoXY(tailX, tailY);
        cout << ".";
    }

    // print out the score, so users know how good they are
    gotoXY(0, height + 2);
    cout << "Score: " << score << "\n";
}

// capture user input without blocking
void input() {
    if (_kbhit()) {
        switch (_getch()) {
            case 'w':
                if (dir != DOWN) dir = UP;
                break;
            case 's':
                if (dir != UP) dir = DOWN;
                break;
            case 'a':
                if (dir != RIGHT) dir = LEFT;
                break;
            case 'd':
                if (dir != LEFT) dir = RIGHT;
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
    if (snakeOccupied.count({x, y}) > 0) {
        gameOver = true;
    }

    snakeOccupied.insert({x, y});
    snakeBody[0] = {x, y};
}

void spawnNewFuit() {
    do {
        fruitX = (rand() % (width - 2)) + 1;   // Avoids edge positions
        fruitY = (rand() % (height - 2)) + 1;  // Avoids edge positions
    } while (snakeOccupied.count({fruitX, fruitY}) > 0);
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
        spawnNewFuit();

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

// Main game loop
int main() {
    hideCursor();
    setup();
    draw();

    // wait for user press the first key to start
    bool validKeyPressed = false;
    while (!validKeyPressed) {
        int ch = getch();
        switch (ch) {
            case 'w':
                dir = UP;
                validKeyPressed = true;
                break;
            case 's':
                dir = DOWN;
                validKeyPressed = true;
                break;
            case 'a':
                dir = LEFT;
                validKeyPressed = true;
                break;
            case 'd':
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
    cout << "Press 'q' to quit." << endl;
    while (true) {
        if (_kbhit() && _getch() == 'q') {
            break;
        }
    }

    return 0;
}
