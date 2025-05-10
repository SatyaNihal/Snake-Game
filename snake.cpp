#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
using namespace std;

const int WIDTH = 40;
const int HEIGHT = 20;

enum Direction
{
    NONE,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

int keyHit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

char getChar()
{
    char buf = 0;
    struct termios old = {0};
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &old);
    read(0, &buf, 1);
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);
    return buf;
}

class Snake
{
public:
    int x, y;
    vector<int> tailX, tailY;
    int score;
    Direction dir;

    Snake()
    {
        x = WIDTH / 2;
        y = HEIGHT / 2;
        dir = NONE;
        score = 0;
    }

    void move()
    {
        if (dir == NONE)
            return;

        if (!tailX.empty())
        {
            for (int i = tailX.size() - 1; i > 0; i--)
            {
                tailX[i] = tailX[i - 1];
                tailY[i] = tailY[i - 1];
            }
            tailX[0] = x;
            tailY[0] = y;
        }

        switch (dir)
        {
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
    }

    void grow()
    {
        tailX.push_back(x);
        tailY.push_back(y);
        score += 10;
    }

    bool collideSelf()
    {
        for (size_t i = 0; i < tailX.size(); i++)
        {
            if (tailX[i] == x && tailY[i] == y)
                return true;
        }
        return false;
    }
};

class Game
{
private:
    Snake snake;
    int foodX, foodY;
    bool running;
    int delay;
    string user;

public:
    Game()
    {
        srand(time(0));
        running = true;
        spawnFood();
    }

    void start()
    {
        welcome();
        showInstructions();
        delay = chooseDifficulty();
        while (running)
        {
            render();
            input();
            update();
            usleep(delay * 1000);
        }
        showGameOver();
    }

    void welcome()
    {
        cout << "\nWELCOME TO SNAKE GAME\n"
             << endl;
        cout << "Enter your name: ";
        cin >> user;
    }

    void showInstructions()
    {
        system("clear");
        cout << "\nINSTRUCTIONS\n"
             << endl;
        cout << "W A S D to move\n";
        cout << "X to quit\n";
        cout << "* = food, O = head, o = tail\n";
        cout << "\nPress Enter to start...";
        cin.ignore();
        cin.get();
    }

    int chooseDifficulty()
    {
        int level;
        cout << "\nSelect Difficulty (1-Easy, 2-Medium, 3-Hard): ";
        cin >> level;
        if (level == 1)
            return 150;
        if (level == 3)
            return 50;
        return 100;
    }

    void spawnFood()
    {
        do
        {
            foodX = rand() % WIDTH;
            foodY = rand() % HEIGHT;
        } while (foodX == snake.x && foodY == snake.y);
    }

    void render()
    {
        system("clear");
        cout << "\nSNAKE GAME\n\n";
        cout << "+";
        for (int i = 0; i < WIDTH; i++)
            cout << "-";
        cout << "+\n";

        for (int i = 0; i < HEIGHT; i++)
        {
            cout << "|";
            for (int j = 0; j < WIDTH; j++)
            {
                if (i == snake.y && j == snake.x)
                    cout << "O";
                else if (i == foodY && j == foodX)
                    cout << "*";
                else
                {
                    bool printed = false;
                    for (size_t k = 0; k < snake.tailX.size(); k++)
                    {
                        if (snake.tailX[k] == j && snake.tailY[k] == i)
                        {
                            cout << "o";
                            printed = true;
                            break;
                        }
                    }
                    if (!printed)
                        cout << " ";
                }
            }
            cout << "|\n";
        }

        cout << "+";
        for (int i = 0; i < WIDTH; i++)
            cout << "-";
        cout << "+\n";

        cout << "\n"
             << user << "'s Score: " << snake.score << "\n";
    }

    void input()
    {
        if (keyHit())
        {
            char key = getChar();
            switch (key)
            {
            case 'w':
                if (snake.dir != DOWN)
                    snake.dir = UP;
                break;
            case 's':
                if (snake.dir != UP)
                    snake.dir = DOWN;
                break;
            case 'a':
                if (snake.dir != RIGHT)
                    snake.dir = LEFT;
                break;
            case 'd':
                if (snake.dir != LEFT)
                    snake.dir = RIGHT;
                break;
            case 'x':
                running = false;
                break;
            }
        }
    }

    void update()
    {
        snake.move();

        if (snake.x < 0 || snake.x >= WIDTH || snake.y < 0 || snake.y >= HEIGHT)
            running = false;

        if (snake.collideSelf())
            running = false;

        if (snake.x == foodX && snake.y == foodY)
        {
            snake.grow();
            spawnFood();
        }
    }

    void showGameOver()
    {
        system("clear");
        cout << "\nGAME OVER\n";
        cout << "\nPlayer: " << user << "\nScore: " << snake.score << endl;

        if (snake.score < 50)
            cout << "\nTry again for a better score!";
        else if (snake.score < 100)
            cout << "\nGood job!";
        else
            cout << "\nExcellent work!";

        cout << "\n\nPress Enter to exit...";
        cin.ignore();
        cin.get();
    }
};

int main()
{
    Game g;
    g.start();
    return 0;
}
