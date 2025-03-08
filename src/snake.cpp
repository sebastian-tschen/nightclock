#include <main.hpp>
#include <snake.h>
#include <PS4Controller.h>
#include <cstdlib> // for rand()
#include <ctime>   // for time()
#include "espLogger.h"
#include <tiniFont.h>
#include <config.h>

#define D_UP 0
#define D_RIGHT 1
#define D_DOWN 2
#define D_LEFT 3

#define PLAYING 0
#define SHOW_SCORE 1
#define SNAKE_SPEED_INCREASE_INTERVAL 10000

#define HS_NONE 0
#define HS_DAILY 1
#define HS_ALL_TIME 2

#define MAX_SNAKE_LENGTH (MATRIX_HEIGHT * MATRIX_WIDTH)

uint8_t direction = D_UP;
uint8_t nextDirection = D_UP;
uint32_t lastMove = 0;
uint32_t lastSpeedIncrease = 0;
unsigned long gameEndedAt = 0;
const uint8_t scoreHue = 144;
const uint8_t scoreSaturation = 200;
const uint8_t fruitHue = 0;
const uint8_t fruitSaturation = 240;
const uint8_t snakeHue = 80;
const uint8_t snakeSaturation = 140;
const uint8_t gameBrightnessOffset = 50;
uint8_t gameBrightness = gameBrightnessOffset;

uint16_t score = 0;
// wether we are currently playing or showing score or something else
uint8_t snakemode = PLAYING;
uint8_t highscore = HS_NONE;

uint16_t snakeSpeed = 700;                // the snake speed in milliseconds
const uint8_t speedIncreaseQuotient = 20; // fraction by which the speed increases

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
    displayMode = TIME;
    snakeHeadIndex = 0;
    snakeLength = 2;
    direction = D_UP;
    snakeSpeed = 700;
    findNewFruitLocation();
    snakeBody[0] = 0x55;
    snakeBody[1] = 0x54;
    score = 0;
    snakemode = PLAYING;
    displayMode = TIME;
    lastSpeedIncrease = millis();
}

uint16_t getDailyHighScore()
{
    uint32_t lastDailyHighScore = conf.get("dailyHighScoreDate").toInt();
    uint32_t today = timeinfo.tm_mday + 100 * timeinfo.tm_mon + 10000 * timeinfo.tm_year;
    if (lastDailyHighScore != today)
    {
        return 0;
    }
    return conf.get("dailyHighScore").toInt();
}
uint16_t getAllTimeHighScore()
{
    return conf.get("allTimeHighScore").toInt();
}

void setDailyHighScore(uint16_t new_score)
{
    conf.put("dailyHighScore", new_score);
    uint32_t today = timeinfo.tm_mday + 100 * timeinfo.tm_mon + 10000 * timeinfo.tm_year;
    conf.put("dailyHighScoreDate", today);
    conf.saveConfigFile();
}

void setAllTimeHighScore(uint16_t new_score)
{
    conf.put("allTimeHighScore", new_score);
    conf.saveConfigFile();
}

void endGame()
{        
    highscore = HS_NONE;
    if (score > getDailyHighScore())
    {
        setDailyHighScore(score);
        highscore = HS_DAILY;
    }
    if (score > getAllTimeHighScore())
    {
        setAllTimeHighScore(score);
        highscore = HS_ALL_TIME;
    }
    snakemode = SHOW_SCORE;

    gameEndedAt = millis();
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

    // commit the next direction
    direction = nextDirection;
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
        endGame();
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
            endGame();
            return;
        }
    }

    // Update snake head position
    snakeHeadIndex = (snakeHeadIndex + 1) % MAX_SNAKE_LENGTH;
    snakeBody[snakeHeadIndex] = (newHeadX << 4) | newHeadY;

    // Check if the snake is on the fruit
    if (newHeadX == fruitX && newHeadY == fruitY)
    {
        // award points
        score += 2;
        // Increase snake length
        snakeLength++;
        // increase speed
        snakeSpeed -= snakeSpeed / speedIncreaseQuotient;
        // Move the fruit
        findNewFruitLocation();
    }
    {
        // Draw the snake
        matrix_clear();
        for (uint8_t i = 0; i < snakeLength; i++)
        {
            uint8_t index = (snakeHeadIndex + MAX_SNAKE_LENGTH - i) % MAX_SNAKE_LENGTH;
            uint8_t coord = snakeBody[index];
            int8_t x = coord >> 4;
            int8_t y = coord & 0x0F;
            matrix.DrawPixel(x, y, CHSV(snakeHue, snakeSaturation, gameBrightness));
        }

        // Draw the fruit
        matrix.DrawPixel(fruitX, fruitY, CHSV(fruitHue, fruitSaturation, gameBrightness));
    }
}
void showScore()
{   uint16_t showtime = 5000;
    if (highscore != HS_NONE)
    {
        showtime = 15000;
    }
    if (millis() - gameEndedAt > showtime)
    {
        resetGame();
        return;
    }
    if (millis() - gameEndedAt < 500)
    {
        PS4.setRumble(100,100);    
        PS4.setLed(255,0, 0);
        PS4.setFlashRate(20,20);
        PS4.sendToController();
    }else{
        PS4.setRumble(0,0); 
        PS4.setLed(0,100, 100);
        PS4.setFlashRate(0,0);
        PS4.sendToController();
    }

    matrix.setFont(&Font3x5FixedNum);
    CRGB color;
    uint8_t thisHue = beat8(30, 255);
    switch (highscore)
    {
    case HS_NONE:
        color = CHSV(scoreHue, scoreSaturation, gameBrightness);
        break;
    case HS_ALL_TIME:
        for (int i = 0; i < MATRIX_HEIGHT; i++)
        {
            matrix.DrawLine(0, i, MATRIX_WIDTH-1, i, CHSV(thisHue + (i * 20), 200, gameBrightness));
        }
        color = CHSV(255, 0, gameBrightness);
        break;
    case HS_DAILY:
        color = CHSV(thisHue, 200, gameBrightness);
        break;
    default:
        color = CRGB(255, 125, 0);
        break;
    }
    matrix.DrawChar(1, 8, score / 10 + '0', color, 0, 1);
    matrix.DrawChar(5, 8, score % 10 + '0', color, 0, 1);
    matrix_show();
}
void updateGameBrightness()
{
    gameBrightness = map(averageAmbientBrightness, 0, 4095, MIN_DARK_BRIGHTNESS, 255) + gameBrightnessOffset;
    if (gameBrightness < gameBrightnessOffset) // an overflow occured during the last operation
    {
        gameBrightness = 255;
    }
}
void snakeLoop()
{
    // check if are in SNAKE mode
    if (displayMode != SNAKE)
    {
        return;
    }
    // update game brightness
    updateGameBrightness();
    // check if we are showing the score
    if (snakemode == SHOW_SCORE)
    {
        showScore();
        return;
    }
    // check if we need to increase the snake speed
    if (millis() - lastSpeedIncrease > SNAKE_SPEED_INCREASE_INTERVAL)
    {
        lastSpeedIncrease = millis();
        snakeSpeed -= snakeSpeed / speedIncreaseQuotient;
        score++;
    }
    // check if the snake moved
    if (millis() - lastMove > snakeSpeed)
    {
        lastMove = millis();
        moveSnake();
    }
}