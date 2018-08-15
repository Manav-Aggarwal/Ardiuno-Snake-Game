/** 
  Author: Manav Aggarwal
  Made for Lab 3x in ENGR40M: An Intro to Making: What is EE
  **/
// Arrays of pin numbers. Fill these in with the pins to which you connected
// your anode (+) wires and cathode (-) wires.
const byte NUM_PINS = 8;
const byte ANODE_PINS[NUM_PINS] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte CATHODE_PINS[NUM_PINS] = {A3, A2, A1, A0, 2, 3, 4, 5};
const int LEFT_BUTTON_RED = A5;
const int RIGHT_BUTTON_GREEN = A4;
const int DEBOUNCE_DELAY = 75; // in milliseconds

const byte UP = 0;
const byte RIGHT = 1;
const byte DOWN = 2;
const byte LEFT = 3;

struct Coord {
  int x, y;
};

class Snake {
    byte length, direction;
    unsigned long speed, lastMove;
    Coord head, body[NUM_PINS*NUM_PINS];
  public:
    void reset();
    void changeDirection(byte);
    void updateHead();
    void addTail();
    void removeTail();
    byte getDirection();
    Coord getBodyAtIndex(byte);
    Coord getHead();
    bool checkMove(unsigned long);
    bool eatsFood();
    bool bodyExistsAt(byte, byte);
} snake;

class GameBoard {
    byte ledState[NUM_PINS][NUM_PINS];
    bool running;
    Coord food;
  public:
    void display();
    void resetLedArray();
    void refreshLedArray();
    void spawnFood();
    void reset();
    void endAnimate(unsigned long);
    void stopGame();
    bool isRunning();
    Coord getFoodLocation();
} board;

class Button {
    byte button_state, last_reading, reading, direction, oppDirection;
    long last_reading_change;
    int pin;
  public:
    Button(int, byte);
    void read();
} leftButton(LEFT_BUTTON_RED, LEFT), rightButton(RIGHT_BUTTON_GREEN, RIGHT);

void Snake::reset() {
  length = 1;
  direction = UP;
  head.x = random(8);
  head.y = random(8);
  speed = 300;
  lastMove = 0;
  for(byte i = 0; i < NUM_PINS*NUM_PINS; i++) {
    body[i].x = body[i].y = -1;
  }
  body[0].y = head.y;
  body[0].x = head.x;
}
void Snake::changeDirection(byte dir) {
  direction = (direction + dir) % 4;
}

bool Snake::checkMove(unsigned long now) {
  if(now - lastMove > speed && board.isRunning()) {
    lastMove = now;
    return true;
  }
  else {
    return false;
  }
}

void Snake::updateHead() {
  switch(direction) {
    case UP:
      head.y++;
      break;
    case RIGHT:
      head.x++;
      break;
    case DOWN:
      head.y += NUM_PINS - 1;
      break;
    case LEFT:
      head.x += NUM_PINS - 1;
  }
  
  head.x %= NUM_PINS;
  head.y %= NUM_PINS;
  
  Serial.print(head.x);
  Serial.print(" ");
  Serial.print(head.y);
  Serial.print("\n");
}

bool Snake::eatsFood() {
  return head.x == board.getFoodLocation().x && head.y == board.getFoodLocation().y;
}

bool Snake::bodyExistsAt(byte x, byte y) {
  for(byte i = 0; i < length; i++) {
    if(body[i].x == x && body[i].y == y) {
      return true;
    }
  }
  return false;
}

void Snake::addTail() {
  body[length].x = head.x;
  body[length].y = head.y;

  length++;
}

void Snake::removeTail() {
  for(int i = 1; i < length; i++) {
    body[i - 1].x = body[i].x;
    body[i - 1].y = body[i].y;
  }

  body[length].x = -1;
  body[length].y = -1;

  length--;
}

Coord Snake::getBodyAtIndex(byte index) {
  return body[index];
}

byte Snake::getDirection(){
  return direction;
}

Coord Snake::getHead() {
  return head;
}

/* Function: display
 * -----------------
 * Runs through one multiplexing cycle of the LEDs, controlling which LEDs are
 * on.
 *
 * During this function, LEDs that should be on will be turned on momentarily,
 * one row at a time. When this function returns, all the LEDs will be off
 * again, so it needs to be called continuously for LEDs to be on.
 */
void GameBoard::display()
{
  for (byte aNum = 0; aNum < NUM_PINS; aNum++) { // iterate through anode (+) wires

    // Set up all the cathode (-) wires first
    for (byte cNum = 0; cNum < NUM_PINS; cNum++) { // iterate through cathode (-) wires
      byte value = ledState[aNum][cNum]; // TODO
      if(value > 0) {
        digitalWrite(CATHODE_PINS[cNum], LOW);
      }
      else {
        digitalWrite(CATHODE_PINS[cNum], HIGH);
      }
    }
    digitalWrite(ANODE_PINS[aNum], LOW);
    delayMicroseconds(1000);
    digitalWrite(ANODE_PINS[aNum], HIGH);
  }
}

void GameBoard::resetLedArray() {
  for(byte i = 0; i < NUM_PINS; i++) {
    for(byte j = 0; j < NUM_PINS; j++) {
      ledState[i][j] = 0;
    }
  }
}

void GameBoard::refreshLedArray() {
  resetLedArray();
  for(int i = 0; snake.getBodyAtIndex(i).x != -1; i++) {
    ledState[snake.getBodyAtIndex(i).x][snake.getBodyAtIndex(i).y] = 1;
  }

  ledState[food.x][food.y] = 1;
}

void GameBoard::spawnFood() {
  byte x, y;

  do {
    x = random(8);
    y = random(8);
  } while(snake.bodyExistsAt(x, y));

  food.x = x;
  food.y = y;
}

void GameBoard::reset() {
  resetLedArray();
  snake.reset();
  running = true;
  spawnFood();
}

void GameBoard::endAnimate(unsigned long last) {
  while(millis() - last < 2000) {
    for (byte aNum = 0; aNum < NUM_PINS; aNum++) { // iterate through anode (+) wires
      digitalWrite(ANODE_PINS[aNum], LOW);
      // Set up all the cathode (-) wires first
      for (byte cNum = 0; cNum < NUM_PINS; cNum++) { // iterate through cathode (-) wires
        byte value = ledState[aNum][cNum]; // TODO
          digitalWrite(CATHODE_PINS[cNum], LOW);
          delay(1);
          digitalWrite(CATHODE_PINS[cNum], HIGH);
      }
      digitalWrite(ANODE_PINS[aNum], HIGH);
    }
  }
}

bool GameBoard::isRunning() {
  return running;
}

void GameBoard::stopGame() {
  running = false;
}

Coord GameBoard::getFoodLocation() {
  return food;
}

Button::Button(int pin, byte direction) {
  this->pin = pin;
  this->direction = direction;
  oppDirection = 4 - direction;
  button_state = HIGH;
  last_reading = HIGH;
  last_reading_change = 0;
}

void Button::read() {
  reading = digitalRead(pin);
  unsigned long now = millis();
  
  // Ignore button_state changes within DEBOUNCE_DELAY milliseconds of the last
  // reading change, otherwise accept.
  if (now - last_reading_change > DEBOUNCE_DELAY) {
    if (reading == LOW && button_state == HIGH) { // button pressed down (HIGH to LOW)
      (snake.getDirection() != RIGHT) ? snake.changeDirection(direction) : snake.changeDirection(oppDirection);
    }
    button_state = reading;
  }

  // Prepare for next loop
  if (reading != last_reading) {
    last_reading_change = now;
  }
  last_reading = reading;
}
void setup()
{
  randomSeed(analogRead(0));
  // Make all of the anode (+) wire and cathode (-) wire pins outputs
  for (byte i = 0; i < NUM_PINS; i++) {
    pinMode(ANODE_PINS[i], OUTPUT);
    pinMode(CATHODE_PINS[i], OUTPUT);
  }

  // turn "off" all the LEDs
  for(byte i = 0; i < NUM_PINS; i++) {
    digitalWrite(ANODE_PINS[i], HIGH);
    digitalWrite(CATHODE_PINS[i], HIGH);
  }

  // Initialize serial communication
  // (to be read by Serial Monitor on your computer)
  pinMode(LEFT_BUTTON_RED, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_GREEN, INPUT_PULLUP);

  board.reset();

  Serial.begin(115200);
  Serial.setTimeout(100);
}

void loop()
{
  leftButton.read();
  rightButton.read();
  unsigned long now = millis();

  if(snake.checkMove(now)) {
      snake.updateHead();
      
      //check if snake eats itself
      if(snake.bodyExistsAt(snake.getHead().x, snake.getHead().y)) {
        board.stopGame();
      }
  
      snake.eatsFood() ? board.spawnFood() : snake.removeTail();
      
      snake.addTail();
      board.refreshLedArray();
  }

  if(!board.isRunning()) {
    board.endAnimate(now);
    board.reset();
  }
  
  board.display();
}
