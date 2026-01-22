#include <iostream>
#include <conio.h>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>

// GLOBAL VARIABLES

const int height = 10;
const int width = 10;

int head_x, head_y;
int fruitCordX, fruitCordY;
int playerScore;

int snakeTailX[100], snakeTailY[100];
int snake_tail_len;

enum snakes_direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
snakes_direction sDir;

bool isGameOver;
bool autoPlay = false;

// A* DATA STRUCTURES 

struct Node {
    int head_x, head_y;
    int g, h;
    Node* parent;
    int f() const { return g + h; }
};

struct NodeCompare {
    bool operator()(Node* a, Node* b) {
        return a->f() > b->f();
    }
};

// UTILITY FUNCTIONS 

int Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

bool IsBlocked(int nx, int ny) {
    if (nx <= 0 || nx >= width || ny < 0 || ny >= height)
        return true;

    for (int i = 0; i < snake_tail_len; i++) {
        if (snakeTailX[i] == nx && snakeTailY[i] == ny)
            return true;
    }
    return false;
}

// A* PATHFINDING 

snakes_direction FindPathAStar() {
    std::priority_queue<Node*, std::vector<Node*>, NodeCompare> open;
    bool closed[height][width] = { false };

    Node* start = new Node{ head_x, head_y, 0, Heuristic(head_x, head_y, fruitCordX, fruitCordY), nullptr };
    open.push(start);

    Node* goalNode = nullptr;

    int dx[4] = { -1, 1, 0, 0 };
    int dy[4] = { 0, 0, -1, 1 };

    while (!open.empty()) {
        Node* current = open.top();
        open.pop();

        if (current->head_x == fruitCordX && current->head_y == fruitCordY) {
            goalNode = current;
            break;
        }

        closed[current->head_y][current->head_x] = true;

        for (int i = 0; i < 4; i++) {
            int nx = current->head_x + dx[i];
            int ny = current->head_y + dy[i];

            if (IsBlocked(nx, ny) || closed[ny][nx])
                continue;

            Node* neighbor = new Node{
                nx, ny,
                current->g + 1,
                Heuristic(nx, ny, fruitCordX, fruitCordY),
                current
            };

            open.push(neighbor);
        }
    }

    if (!goalNode)
        return sDir;

    Node* step = goalNode;
    while (step->parent && step->parent->parent)
        step = step->parent;

    if (step->head_x < head_x) return LEFT;
    if (step->head_x > head_x) return RIGHT;
    if (step->head_y < head_y) return UP;
    if (step->head_y > head_y) return DOWN;

    return sDir;
}

// GAME FUNCTIONS 

void GameInit() {
    isGameOver = false;
    sDir = STOP;
    head_x = width / 2;
    head_y = height / 2;
    fruitCordX = rand() % (width - 2) + 1;
    fruitCordY = rand() % (height - 2) + 1;
    playerScore = 0;
    snake_tail_len = 0;
}


void clearScreen() {
    #ifdef _WIN32
        std::system("cls");
    #else
        // Assumes POSIX-like system (Linux, macOS, etc)
        std::system("clear");
    #endif
}

void GameRender(const std::string& playerName) {
    clearScreen();

    for (int i = 0; i < width + 2; i++)
        std::cout << "-";
    std::cout << std::endl;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j <= width; j++) {
            if (j == 0 || j == width)
                std::cout << "|";
            else if (i == head_y && j == head_x)
                std::cout << "O";
            else if (i == fruitCordY && j == fruitCordX)
                std::cout << "#";
            else {
                bool printTail = false;
                for (int k = 0; k < snake_tail_len; k++) {
                    if (snakeTailX[k] == j && snakeTailY[k] == i) {
                        std::cout << "o";
                        printTail = true;
                        break;
                    }
                }
                if (!printTail)
                    std::cout << " ";
            }
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < width + 2; i++)
        std::cout << "-";
    std::cout << std::endl;

    std::cout << playerName << "'s Score: " << playerScore << std::endl;
    std::cout << "Mode: " << (autoPlay ? "AI (AutoPlay)" : "Manual") << " | Press 'p' to toggle" << std::endl;
}

void UpdateGame() {
    int prevX = snakeTailX[0];
    int prevY = snakeTailY[0];
    int prev2X, prev2Y;
    snakeTailX[0] = head_x;
    snakeTailY[0] = head_y;

    //moving the snake tail
    for (int i = 1; i < snake_tail_len; i++) {
        prev2X = snakeTailX[i];
        prev2Y = snakeTailY[i];
        snakeTailX[i] = prevX;
        snakeTailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    //moving the snake head
    switch (sDir) {
    case LEFT:  head_x--; break;
    case RIGHT: head_x++; break;
    case UP:    head_y--; break;
    case DOWN:  head_y++; break;
    }

    //if the head hits the wall = gameover
    if (head_x <= 0 || head_x >= width || head_y < 0 || head_y >= height)
        isGameOver = true;

    // game over if snake head touches tail
    for (int i = 0; i < snake_tail_len; i++) {
        if (snakeTailX[i] == head_x && snakeTailY[i] == head_y)
            isGameOver = true;
    }

    //snake head ate the fruit
    if (head_x == fruitCordX && head_y == fruitCordY) {
        playerScore += 10;
        if (snake_tail_len < 100)
            snake_tail_len++;

        // generate fruit avoiding the body
        while (true) {
            fruitCordX = rand() % (width - 2) + 1;
            fruitCordY = rand() % (height - 2) + 1;
            int i = 0;
            bool noCollision = fruitCordX != head_x || fruitCordY != head_y;
            for(int i = 0; i < snake_tail_len; i++) {
                if(fruitCordX != snakeTailX[i] || fruitCordY != snakeTailY[i]) {
                    continue;
                } else {
                    noCollision = false;
                    break;
                }
            }

            if (noCollision == true) {
                break;
            }
        }
    }
}


void UserInput() {
    if (_kbhit()) {
        char key = _getch();
        
        if (key == 'p' || key == 'P') {
            autoPlay = !autoPlay;
            return;
        }
        
        if (!autoPlay) {
            switch (key) {
                case 'a': if (sDir != RIGHT) sDir = LEFT; break;
                case 'd': if (sDir != LEFT) sDir = RIGHT; break;
                case 'w': if (sDir != DOWN) sDir = UP; break;
                case 's': if (sDir != UP) sDir = DOWN; break;
                case 'x': isGameOver = true; break;
            }
        }
    }
    
    if (autoPlay && sDir != STOP) {
        sDir = FindPathAStar();
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    std::string playerName;
    std::cout << "Enter your name: ";
    std::cin >> playerName;

    std::cout << "\nChoose mode:\n";
    std::cout << "1. Manual Control\n";
    std::cout << "2. AI AutoPlay\n";
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;
    autoPlay = (choice == 2);

    GameInit();

    if (autoPlay)
        sDir = RIGHT;

    while (!isGameOver) {
        UserInput();
        UpdateGame();
        GameRender(playerName);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    clearScreen();
    std::cout << "Game Over!\n";
    std::cout << playerName << "'s Final Score: " << playerScore << std::endl;

    return 0;
}
