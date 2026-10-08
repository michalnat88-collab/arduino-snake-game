#include <LedControl.h>

// =====================================================
// MAX7219
// =====================================================

const int DIN_PIN = 11;
const int CLK_PIN = 13;
const int CS_PIN  = 10;

LedControl matrix(DIN_PIN, CLK_PIN, CS_PIN, 1);


// =====================================================
// BUTTONS
// =====================================================

const int UP_BUTTON    = 2;
const int DOWN_BUTTON  = 3;
const int RIGHT_BUTTON = 4;
const int LEFT_BUTTON  = 5;


// =====================================================
// BOARD
// =====================================================

const int WIDTH = 8;
const int HEIGHT = 8;
const int MAX_LENGTH = 64;


// =====================================================
// SNAKE
// =====================================================

int snakeX[MAX_LENGTH];
int snakeY[MAX_LENGTH];

int snakeLength;

int foodX;
int foodY;


// =====================================================
// DIRECTIONS
// =====================================================

enum Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

Direction direction;
Direction nextDirection;


// =====================================================
// SPEED
// =====================================================

unsigned long lastMove = 0;

int moveDelay = 700;

const int MIN_SPEED = 300;
const int SPEED_INCREASE = 30;


// =====================================================
// FOOD BLINK
// =====================================================

unsigned long lastBlink = 0;

bool foodVisible = true;

const unsigned long blinkDelay = 300;


// =====================================================
// MIRROR
// =====================================================

const bool MIRROR_X = true;


// =====================================================
// DRAW PIXEL
// =====================================================

void drawPixel(int x, int y, bool state) {

  int displayX = x;

  if (MIRROR_X) {
    displayX = 7 - x;
  }

  matrix.setLed(0, y, displayX, state);
}


// =====================================================
// CLEAR MATRIX
// =====================================================

void clearMatrix() {
  matrix.clearDisplay(0);
}


// =====================================================
// START SCREEN - SHARP TRIANGLE ▼
// =====================================================

void showStart() {

  clearMatrix();

  /*
       █
      ███
     █████
    ███████

       ▼

  חד ושפיצי
  */

  bool play[4][7] = {

    {0, 0, 0, 1, 0, 0, 0},

    {0, 0, 1, 1, 1, 0, 0},

    {0, 1, 1, 1, 1, 1, 0},

    {1, 1, 1, 1, 1, 1, 1}

  };


  // ממורכז על מטריצת 8x8
  for (int y = 0; y < 4; y++) {

    for (int x = 0; x < 7; x++) {

      if (play[y][x]) {

        int displayX = x;
        int displayY = y + 2;

        if (MIRROR_X) {
          displayX = 7 - displayX;
        }

        matrix.setLed(
          0,
          displayY,
          displayX,
          true
        );
      }
    }
  }
}


// =====================================================
// WAIT FOR START
// =====================================================

Direction waitForStart() {

  showStart();

  while (true) {

    // UP → RIGHT
    if (digitalRead(UP_BUTTON) == LOW) {

      while (digitalRead(UP_BUTTON) == LOW) {
        delay(10);
      }

      return RIGHT;
    }


    // DOWN → LEFT
    if (digitalRead(DOWN_BUTTON) == LOW) {

      while (digitalRead(DOWN_BUTTON) == LOW) {
        delay(10);
      }

      return LEFT;
    }


    // RIGHT → UP
    if (digitalRead(RIGHT_BUTTON) == LOW) {

      while (digitalRead(RIGHT_BUTTON) == LOW) {
        delay(10);
      }

      return UP;
    }


    // LEFT → DOWN
    if (digitalRead(LEFT_BUTTON) == LOW) {

      while (digitalRead(LEFT_BUTTON) == LOW) {
        delay(10);
      }

      return DOWN;
    }

    delay(10);
  }
}


// =====================================================
// CREATE FOOD
// =====================================================

void createFood() {

  bool valid = false;

  while (!valid) {

    foodX = random(0, WIDTH);
    foodY = random(0, HEIGHT);

    valid = true;

    for (int i = 0; i < snakeLength; i++) {

      if (snakeX[i] == foodX &&
          snakeY[i] == foodY) {

        valid = false;
        break;
      }
    }
  }

  foodVisible = true;
}


// =====================================================
// NEW GAME
// =====================================================

void newGame(Direction startDirection) {

  snakeLength = 2;

  // התחלה במרכז
  snakeX[0] = 4;
  snakeY[0] = 4;

  snakeX[1] = 3;
  snakeY[1] = 4;

  direction = startDirection;
  nextDirection = startDirection;

  moveDelay = 700;

  createFood();

  lastMove = millis();
  lastBlink = millis();
}


// =====================================================
// READ BUTTONS
// =====================================================

void readButtons() {

  // UP → RIGHT
  if (digitalRead(UP_BUTTON) == LOW) {

    if (direction != LEFT) {
      nextDirection = RIGHT;
    }
  }


  // DOWN → LEFT
  if (digitalRead(DOWN_BUTTON) == LOW) {

    if (direction != RIGHT) {
      nextDirection = LEFT;
    }
  }


  // RIGHT → UP
  if (digitalRead(RIGHT_BUTTON) == LOW) {

    if (direction != DOWN) {
      nextDirection = UP;
    }
  }


  // LEFT → DOWN
  if (digitalRead(LEFT_BUTTON) == LOW) {

    if (direction != UP) {
      nextDirection = DOWN;
    }
  }
}


// =====================================================
// CHECK SNAKE COLLISION
// =====================================================

bool hitsSnake(int x, int y) {

  for (int i = 0; i < snakeLength; i++) {

    if (snakeX[i] == x &&
        snakeY[i] == y) {

      return true;
    }
  }

  return false;
}


// =====================================================
// MOVE SNAKE
// =====================================================

bool moveSnake() {

  direction = nextDirection;

  int newX = snakeX[0];
  int newY = snakeY[0];


  if (direction == UP) {
    newY--;
  }

  if (direction == DOWN) {
    newY++;
  }

  if (direction == RIGHT) {
    newX++;
  }

  if (direction == LEFT) {
    newX--;
  }


  // פגיעה בקיר
  if (newX < 0 ||
      newX >= WIDTH ||
      newY < 0 ||
      newY >= HEIGHT) {

    return false;
  }


  // פגיעה בגוף
  if (hitsSnake(newX, newY)) {

    return false;
  }


  // האם נאכל האוכל?
  bool ateFood =
    (newX == foodX &&
     newY == foodY);


  // גדילת הנחש
  if (ateFood &&
      snakeLength < MAX_LENGTH) {

    snakeLength++;
  }


  // הזזת הגוף
  for (int i = snakeLength - 1; i > 0; i--) {

    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }


  // הזזת הראש
  snakeX[0] = newX;
  snakeY[0] = newY;


  // אוכל חדש והאצה
  if (ateFood) {

    createFood();

    if (moveDelay > MIN_SPEED) {

      moveDelay -= SPEED_INCREASE;

      if (moveDelay < MIN_SPEED) {
        moveDelay = MIN_SPEED;
      }
    }
  }

  return true;
}


// =====================================================
// DRAW GAME
// =====================================================

void drawGame() {

  clearMatrix();

  // ציור הנחש
  for (int i = 0; i < snakeLength; i++) {

    drawPixel(
      snakeX[i],
      snakeY[i],
      true
    );
  }


  // ציור האוכל
  if (foodVisible) {

    drawPixel(
      foodX,
      foodY,
      true
    );
  }
}


// =====================================================
// GAME OVER
// =====================================================

void gameOverAnimation() {

  clearMatrix();

  // X
  for (int i = 0; i < 8; i++) {

    drawPixel(i, i, true);

    drawPixel(
      7 - i,
      i,
      true
    );
  }

  delay(1000);

  clearMatrix();

  delay(500);
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  matrix.shutdown(0, false);

  matrix.setIntensity(0, 5);

  matrix.clearDisplay(0);


  // הגדרת הכפתורים
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);


  // יצירת מספר אקראי
  randomSeed(analogRead(A0));


  // מסך פתיחה
  Direction startDirection = waitForStart();


  // התחלת המשחק
  newGame(startDirection);
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  unsigned long now = millis();


  // קריאת כפתורים
  readButtons();


  // הבהוב האוכל
  if (now - lastBlink >= blinkDelay) {

    lastBlink = now;

    foodVisible = !foodVisible;
  }


  // תנועת הנחש
  if (now - lastMove >= moveDelay) {

    lastMove = now;

    bool alive = moveSnake();


    // GAME OVER
    if (!alive) {

      gameOverAnimation();


      // חזרה למסך Play
      Direction startDirection =
        waitForStart();


      // משחק חדש
      newGame(startDirection);

      return;
    }
  }


  // ציור המשחק
  drawGame();
}
