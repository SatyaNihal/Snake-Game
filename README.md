# Snake-Game

A terminal-based Snake game written in C++ using an object-oriented design (& termios for real time input). Control the snake to eat food, grow longer, and score points—while avoiding the walls and your own tail.  
Created by **Satya Nihal Kodukula**.

## Features

- Object-oriented structure using `Snake` and `Game` classes
- Real-time controls (WASD) without pressing Enter
- Difficulty settings: Easy, Medium, Hard
- Score tracking and tail growth on eating food
- Game over screen with feedback based on performance

## How It Works

1. Enter your name and select a difficulty level.
2. Control the snake using W (up), A (left), S (down), D (right).
3. Eat the food (`*`) to grow and gain points.
4. Avoid running into walls or your own tail (`o`).
5. The game ends if you collide or press X to quit.

## Requirements

- C++ compiler (e.g., `g++`)
- Unix-based terminal

## Compile and Run

```bash
g++ snake_game.cpp -o snake_game
./snake_game
```
