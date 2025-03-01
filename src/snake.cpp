#include <main.hpp>
#include <snake.h>

#include <cstdlib> // for rand()
#include <ctime>   // for time()
#include "espLogger.h"

#define D_UP 0
#define D_RIGHT 1
#define D_DOWN 2
#define D_LEFT 3

#define MAX_SNAKE_LENGTH (MATRIX_HEIGHT * MATRIX_WIDTH)

uint8_t direction = 0;
uint32_t lastMove = 0;

uint16_t snakeSpeed = 700; // the snake speed in milliseconds
uint8_t speedIncrease = 30;

int8_t fruitX = 5;
int8_t fruitY = 5;

uint8_t snakeBody[MAX_SNAKE_LENGTH]; // Array to store snake body coordinates
uint8_t snakeHeadIndex = 0;          // Index of the current head of the snake
uint8_t snakeLength = 2;             // Initial length of the snake

void findNewFruitLocation()
{
    srand(time(0)); // Seed the random number generator

    bool validLocation = false;
    while (!validLocation)
    {
        fruitX = rand() % MATRIX_WIDTH;
        fruitY = rand() % MATRIX_HEIGHT;

        validLocation = true;
        for (uint8_t i = 0; i < snakeLength; i++)
        {
            uint8_t index = (snakeHeadIndex + MAX_SNAKE_LENGTH - i) % MAX_SNAKE_LENGTH;
            uint8_t coord = snakeBody[index];
            int8_t x = coord >> 4;
            int8_t y = coord & 0x0F;
            if (x == fruitX && y == fruitY)
            {
                validLocation = false;
                break;
            }
        }
    }
}
void resetGame()
{
    snakeHeadIndex = 0;
    snakeLength = 2;
    direction = D_UP;
    snakeSpeed = 700;
    findNewFruitLocation();
    snakeBody[0] = 0x55;
    snakeBody[1] = 0x54;
    displayMode = TIME;
}
void moveSnake()
{
    // Get current head position
    uint8_t currentHead = snakeBody[snakeHeadIndex];
    int8_t snakeX = currentHead >> 4;
    int8_t snakeY = currentHead & 0x0F;

    // Calculate new head position
    int8_t newHeadX = snakeX;
    int8_t newHeadY = snakeY;

    switch (direction)
    {
    case D_UP: // Up
        newHeadY--;
        break;
    case D_RIGHT: // Right
        newHeadX++;
        break;
    case D_DOWN: // Down
        newHeadY++;
        break;
    case D_LEFT: // Left
        newHeadX--;
        break;
    }

    // Check if the snake is out of bounds
    if (newHeadX < 0 || newHeadX >= MATRIX_WIDTH || newHeadY < 0 || newHeadY >= MATRIX_HEIGHT)
    {
        // return back to clock mode
        LOG_D("out of bounds");
        resetGame();
        return;
    }

    // Check if the snake is eating itself
    for (uint8_t i = 0; i < snakeLength; i++)
    {
        uint8_t index = (snakeHeadIndex + MAX_SNAKE_LENGTH - i) % MAX_SNAKE_LENGTH;
        uint8_t coord = snakeBody[index];
        int8_t x = coord >> 4;
        int8_t y = coord & 0x0F;
        if (newHeadX == x && newHeadY == y)
        {
            // return back to clock mode
            LOG_D("eating itself");
            resetGame();
            return;
        }
    }

    // Update snake head position
    snakeHeadIndex = (snakeHeadIndex + 1) % MAX_SNAKE_LENGTH;
    snakeBody[snakeHeadIndex] = (newHeadX << 4) | newHeadY;

    // Check if the snake is on the fruit
    if (newHeadX == fruitX && newHeadY == fruitY)
    {
        // Increase snake length
        snakeLength++;
        //increase speed
        snakeSpeed -= speedIncrease;
        // Move the fruit
        findNewFruitLocation();
    }


    // Draw the snake
    matrix_clear();
    for (uint8_t i = 0; i < snakeLength; i++)
    {
        uint8_t index = (snakeHeadIndex + MAX_SNAKE_LENGTH - i) % MAX_SNAKE_LENGTH;
        uint8_t coord = snakeBody[index];
        int8_t x = coord >> 4;
        int8_t y = coord & 0x0F;
        matrix.DrawPixel(x, y, CRGB(0, 255, 0));
    }

    // Draw the fruit
    matrix.DrawPixel(fruitX, fruitY, CRGB(255, 0, 0));

}

void snakeLoop()
{
    // check if are in SNAKE mode
    if (displayMode != SNAKE)
    {
        return;
    }
    // check if the snake moved
    if (millis() - lastMove > snakeSpeed)
    {
        lastMove = millis();
        moveSnake();
    }
}