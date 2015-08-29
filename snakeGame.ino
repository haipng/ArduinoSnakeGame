// SNAKE GAME on 8x8 LED matrix
// using Arduino and 2 74HC595 shift register.

// Pin connected to Pin 12 of 74HC595 (Latch)
int latchPin = 8;

// Pin connected to Pin 11 of 74HC595 (Clock)
int clockPin = 12;

// Pin connected to Pin 14 of 74HC595 (Data)
int dataPin = 11;

// Screen
byte led[8];

// button pin
int btn_up = 3;
int btn_down = 4;
int btn_left = 5;
int btn_right = 6;

// game variables
typedef struct Link {
  int x;
  int y;
  struct Link * next;
} Link;

Link * pHead = NULL;
Link * pTail = NULL;

int curDirection = 4;
int newDirection = 4;
int appleX = 5;
int appleY = 5;

unsigned long oldTimer, curTimer;

boolean dead = 0;

void setup() {
  // Seed Random Generator with noise from analog pin 0  
  randomSeed(analogRead(0));

  // set pins to output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  // set button pins to input
  pinMode(btn_up, INPUT_PULLUP);
  pinMode(btn_down, INPUT_PULLUP);
  pinMode(btn_left, INPUT_PULLUP);
  pinMode(btn_right, INPUT_PULLUP);
  
  Serial.begin(9600);
  // clear screen
  clrscr();
}

void loop() {

  snakeInit();
  screenUpdate();
  oldTimer = millis();
  curTimer = millis();

  while(!dead) {
    curTimer = millis();
    
    setDirection();
    
    if(curTimer-oldTimer >= 320) {
      curDirection = newDirection;
      moveSnake(curDirection);
      screenUpdate();
      oldTimer = millis();
    }
    
    // update screen
    screenDisplay();
  }
  
  int count = 0;
  while(count<8) {
    curTimer = millis();
    if(curTimer-oldTimer >= 100) {
      led[count]=B11111111;
      oldTimer = millis();
      count++;
    }
    screenDisplay();
  }
  
  clrscr();
  
  while(1) {
    curTimer = millis();
    if(curTimer-oldTimer >= 700) {
      for(int i=0; i<8; i++) {
        led[i]=~led[i];
      }
      oldTimer = millis();
    }
    screenDisplay();
  }
  
  
}

void
addHead(int x, int y)
{
  Link *temp;
  temp = (Link*) malloc (sizeof(Link));
  	
  // create new head
  temp->x = x;
  temp->y = y;
  temp->next = NULL;
  	
  if(pHead!=NULL)
    pHead->next = temp;
  	
  // point to new head
  pHead = temp;
}


void snakeInit() {
  int x = 3;
  int y = 3;
  for (int i=0; i<2; i++, x++) {
    addHead(x,y);
    if (i == 0)
      pTail = pHead;
  }
}

void setDirection() {
  if(digitalRead(btn_up) == LOW) {
    if(curDirection!=2)
      newDirection = 1;
  }
  if(digitalRead(btn_down) == LOW) {
    if(curDirection!=1)
      newDirection = 2;
  }
  if(digitalRead(btn_left) == LOW) {
    if(curDirection!=4)
      newDirection = 3;
  }
  if(digitalRead(btn_right) == LOW) {
    if(curDirection!=3)
      newDirection = 4;
  }
}

void moveSnake(int direction)
{
  int newX = pHead->x;
  int newY = pHead->y;
  if(direction==1)
    newY--;
  if(direction==2)
    newY++;
  if(direction==3)
    newX--;
  if(direction==4)
    newX++;

  if(newX > 8)
    newX=1;
  if(newX < 1)
    newX=8;
  if(newY > 8)
    newY=1;
  if(newY < 1)
    newY=8;

  dead |= check(newX, newY);

  if(!dead) {
    if(newX==appleX && newY==appleY) {
      addHead(newX, newY);
      newApple();
    }
    else {
      Link *temp = pTail;
      			
      // point to new tail
      pTail = pTail->next;
      			
      // new head
      pHead->next = temp;
      pHead = temp;
      			
      pHead->x = newX;
      pHead->y = newY;			
      pHead->next = NULL;
    } 
  }
}

void newApple() {
  boolean check = 0;
  Link * ptr = pTail;
  do {
    check = 0;
    appleX = random(7) + 1;
    appleY = random(7) + 1;
    Serial.println(appleX);
    while(ptr!=NULL) {
      if(appleX==(ptr->x) && appleY==(ptr->y)) {
        check = 1;
        break;
      }
      ptr = ptr->next;
    }
  } while (check == 1);
}
    
boolean check(int x, int y) {
  Link *ptr;
  ptr = pTail;
  while(ptr!=NULL)
  {
    if(x==ptr->x && y==ptr->y)
      return 1;
    ptr=ptr->next;
  }
  
  return 0;
}
  
//
// display driver
//
void screenUpdate() {
  Link * ptr;
  ptr = pTail;
  
  clrscr();
  while(ptr!=NULL) {
    led[ptr->y-1] = led[ptr->y-1] | (1<<(8-ptr->x));
    ptr = ptr->next;
  }
  
  led[appleY-1] = led[appleY-1] | (1<<(8-appleX));
}  
  
void screenDisplay() {
  byte row = B10000000;

  for (byte k = 0; k < 8; k++) {
    // Open up the latch ready to receive data
    digitalWrite(latchPin, LOW);
    
    shiftData(~row); // if use PNP transitor
    // shiftData(row);
    shiftData(~led[k]);
    // Close the latch, sending the data in the registers out to the matrix
    digitalWrite(latchPin, HIGH);
    row = row >> 1;
  }
}

void clrscr()
{
  for(int i=0; i<8; i++) {
    led[i] = B00000000;
  }
}

void shiftData(byte data) {
  // Shift out 8 bits LSB first,
  // on rising edge of clock
  boolean pinState;

  //clear shift register read for sending data
  digitalWrite(dataPin, LOW);

  // for each bit in dataOut send out a bit
  for (int i=0; i<8; i++) 
  {
    //set clockPin to LOW prior to sending bit
    digitalWrite(clockPin, LOW);

    // if the value of data and (logical AND) a bitmask
    // are true, set pinState to 1 (HIGH)
    if (data & (1<<i)) 
    {
      pinState = HIGH;
    }
    else 
    {
      pinState = LOW;
    }

    //sets dataPin to HIGH or LOW depending on pinState
    digitalWrite(dataPin, pinState);

    //send bit out on rising edge of clock
    digitalWrite(clockPin, HIGH);
    digitalWrite(dataPin, LOW);
  }

  //stop shifting
  digitalWrite(clockPin, LOW);
}
