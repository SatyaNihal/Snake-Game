#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <limits>

using namespace std;

// play area
const int WIDTH = 40;
const int HEIGHT = 20;

//enum to store direction
enum Direction { NONE, LEFT, RIGHT, UP, DOWN };

//snake
class Snake {
public:
    int x, y; //current head @
    vector<int> tailX, tailY; //use vector for parallel coordinates
    int score;
    Direction dir; //curr direction

    Snake() {
        x = WIDTH / 2;
        y = HEIGHT / 2;
        dir = NONE;     // idle until the first keypress (W/A/S/D)
        score = 0;
    }

    void move() {
        if (dir == NONE) return; //dont move until press

        if (!tailX.empty()) { //move tail segment forward
            for (int i = (int)tailX.size() - 1; i > 0; --i) {
                tailX[i] = tailX[i - 1];
                tailY[i] = tailY[i - 1];
            }
            tailX[0] = x; //first tail segment follows head
            tailY[0] = y;
        }

        switch (dir) { //move head
            case LEFT:  --x; break;
            case RIGHT: ++x; break;
            case UP:    --y; break;
            case DOWN:  ++y; break;
            default: break;
        }
    }

    void grow() { //add score and tail when food
        tailX.push_back(x);
        tailY.push_back(y);
        score += 10;
    }

    bool collideSelf() { //head overlap w/ self
        for (size_t i = 0; i < tailX.size(); ++i) {
            if (tailX[i] == x && tailY[i] == y) return true;
        }
        return false;
    }
};

// RAII guard
class StdinRawGuard {
    struct termios oldt{}; // save termnial settings
    int oldf{-1}; //save file descriptor flags
public:
    StdinRawGuard() {
        tcgetattr(STDIN_FILENO, &oldt);
        struct termios raw = oldt;
        raw.c_lflag &= ~(ICANON | ECHO);  //no line buffering, no echo
        raw.c_cc[VMIN] = 0;               // read returns immediately
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    }
    ~StdinRawGuard() {
        if (oldf != -1) fcntl(STDIN_FILENO, F_SETFL, oldf);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
};

class Game {
private:
    Snake snake;
    int foodX, foodY;
    bool running; //running = false ends game
    int delay;      // milliseconds between frames
    string user; //user name

public:
    Game() {
        srand((unsigned)time(0)); //first food
        running = true;
        spawnFood();
    }

    void start() { //game routine
        welcome();
        showInstructions();
        delay = chooseDifficulty();

        // switch terminal to raw + non-blocking only during gameplay.
        {
            StdinRawGuard tty_guard;

            while (running) {
                render();
                input();   // non-blocking read
                update();
                usleep(delay * 1000);
            }
        } // terminal restored here

        showGameOver();
    }

private:
    void welcome() {
        cout << "\nWELCOME TO SNAKE GAME\n" << endl;
        cout << "Enter your name: ";
        cin >> user;
    }

    void showInstructions() {
        system("clear"); //clear terminal to get rid of prev commands/games
        cout << "\nINSTRUCTIONS\n" << endl;
        cout << "W A S D to move\n";
        cout << "X to quit\n";
        cout << "* = food, O = head, o = tail\n";
        cout << "\nPress Enter to start...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear leftover newline
        cin.get();
    }

    int chooseDifficulty() {
        int level;
        cout << "\nSelect Difficulty (1-Easy, 2-Medium, 3-Hard): ";
        cin >> level;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (level == 1) return 150; //unless 1 or 3 selected - set medium
        if (level == 3) return 50;
        return 100;
    }

    void spawnFood() { //random food but loop if food = head
        do {
            foodX = rand() % WIDTH;
            foodY = rand() % HEIGHT;
        } while (foodX == snake.x && foodY == snake.y);
    }

    void render() {
        system("clear");
        cout << "\nSNAKE GAME\n\n";

        cout << "+";
        for (int i = 0; i < WIDTH; ++i) cout << "-";
        cout << "+\n";

        for (int i = 0; i < HEIGHT; ++i) {
            cout << "|";
            for (int j = 0; j < WIDTH; ++j) {
                if (i == snake.y && j == snake.x) {
                    cout << "O";
                } else if (i == foodY && j == foodX) {
                    cout << "*";
                } else {
                    bool printed = false;
                    for (size_t k = 0; k < snake.tailX.size(); ++k) {
                        if (snake.tailX[k] == j && snake.tailY[k] == i) {
                            cout << "o";
                            printed = true;
                            break;
                        }
                    }
                    if (!printed) cout << " ";
                }
            }
            cout << "|\n";
        }

        cout << "+";
        for (int i = 0; i < WIDTH; ++i) cout << "-";
        cout << "+\n";

        cout << "\n" << user << "'s Score: " << snake.score << "\n";
    }

    void input() {
        // unix std lib to only keep last key pressed and remove buffer
        unsigned char c;
        int last = -1;
        while (read(STDIN_FILENO, &c, 1) == 1) {
            last = c;
        }
        if (last == -1) return;

        switch (last) {
            case 'w': if (snake.dir != DOWN)  snake.dir = UP;    break;
            case 's': if (snake.dir != UP)    snake.dir = DOWN;  break;
            case 'a': if (snake.dir != RIGHT) snake.dir = LEFT;  break;
            case 'd': if (snake.dir != LEFT)  snake.dir = RIGHT; break;
            case 'x': running = false; break;
            default: break; // ignore others
        }
    }

    void update() { //end if hitwall or hit self, grow if food
        snake.move();

        if (snake.x < 0 || snake.x >= WIDTH || snake.y < 0 || snake.y >= HEIGHT)
            running = false;

        if (snake.collideSelf())
            running = false;

        if (snake.x == foodX && snake.y == foodY) {
            snake.grow();
            spawnFood();
        }
    }

    void showGameOver() {
        system("clear");
        cout << "\nGAME OVER\n";
        cout << "\nPlayer: " << user << "\nScore: " << snake.score << endl;

        if (snake.score < 50)      cout << "\nTry again for a better score!";
        else if (snake.score < 100) cout << "\nGood job!";
        else                        cout << "\nExcellent work!";

        cout << "\n\nPress Enter to exit...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
};

int main() {
    Game g;
    g.start();
    return 0;
}
