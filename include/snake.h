#define D_UP 0
#define D_RIGHT 1
#define D_DOWN 2
#define D_LEFT 3

#define MAX_SNAKE_LENGTH (MATRIX_HEIGHT * MATRIX_WIDTH)

extern uint8_t direction;
extern uint8_t nextDirection;

extern uint16_t snakeSpeed;

extern int8_t fruitX;
extern int8_t fruitY;

extern uint8_t snakeHeadIndex;          // Index of the current head of the snake
extern uint8_t snakeLength;             // Initial length of the snake

extern uint8_t snakeBody[MAX_SNAKE_LENGTH]; // Array to store snake body coordinates

void snakeLoop();
void resetGame();
