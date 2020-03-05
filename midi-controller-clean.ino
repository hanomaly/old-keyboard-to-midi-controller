#define NUM_ROWS 8
#define NUM_COLS 7

#define NOTE_ON_CMD 144
#define NOTE_OFF_CMD 128
#define NOTE_VELOCITY 127

//MIDI baud rate
#define SERIAL_RATE 31250 //31250 //when going through midi port

// Pin Definitions

// Row input pins
const int row1Pin = 5;
const int row2Pin = 6;
const int row3Pin = 7;
const int row4Pin = 8;
const int row5Pin = 9;
const int row6Pin = 10;
const int row7Pin = 11;
const int row8Pin = 12;

// 74HC595 pins
const int dataPin = 4;
const int latchPin = 3;
const int clockPin = 2;

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

// bitmasks for scanning columns
int bits[] =
{ 
  B10000000,
  B01000000,
  B00100000,
  B00010000,
  B00001000,
  B00000100,
  B00000010,
  B00000001
};

// for the joystick
int lsb = 0;
int msb = 0;
int msb1 = 0;
const int pitchbend = 224;
const int controlchange = 176;
//const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output
bool pb = false;
bool cc = false;

// drums
const int dp1 = 2;
int hitavg = 0;
int vel =  0;
bool drumon = false;
int drumtol = 150;

void setup()
{
  int note = 31;

  for(int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr)
    {
      keyPressed[rowCtr][colCtr] = false;
      keyToMidiMap[rowCtr][colCtr] = note;
      note++;
    }
  }

  // setup pins output/input mode
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  pinMode(row1Pin, INPUT);
  pinMode(row2Pin, INPUT);
  pinMode(row3Pin, INPUT);
  pinMode(row4Pin, INPUT);
  pinMode(row5Pin, INPUT);
  pinMode(row6Pin, INPUT);
  pinMode(row7Pin, INPUT);
  pinMode(row8Pin, INPUT);

  // for joystick switch
  //pinMode(SW_pin, INPUT);
  //digitalWrite(SW_pin, HIGH);

  Serial.begin(SERIAL_RATE);
}

void loop()
{

  for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    //scan next column
    scanColumn(colCtr);

    //get row values at this column
    int rowValue[NUM_ROWS];
    rowValue[0] = digitalRead(row1Pin);
    rowValue[1] = digitalRead(row2Pin);
    rowValue[2] = digitalRead(row3Pin);
    rowValue[3] = digitalRead(row4Pin);
    rowValue[4] = digitalRead(row5Pin);
    rowValue[5] = digitalRead(row6Pin);
    rowValue[6] = digitalRead(row7Pin);
    rowValue[7] = digitalRead(row8Pin);

    // process keys pressed
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] != 0 && !keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = true;
        noteOn(rowCtr,colCtr);
      }
    }

    // process keys released
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] == 0 && keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = false;
        noteOff(rowCtr,colCtr);
      }
    }
  }

  // joystick - pitchbend
  msb = (analogRead(X_pin) / 1024.0) * 127;
  if (msb<62 || msb>66) {
    Serial.write(pitchbend);//send command byte
    Serial.write(lsb);//send data byte #1
    Serial.write(msb);//send data byte #2
    pb=true;
  }
  else if (pb==true && msb>62 && msb<66) {
    Serial.write(pitchbend);//send command byte
    Serial.write(lsb);//send data byte #1
    Serial.write(msb);//send data byte #2
    pb=false;
  }

  // look up changeable midi things for y axis
  msb1 = (analogRead(Y_pin) / 1024.0) * 127;
  if (msb1<62 || msb1>66) {
    Serial.write(controlchange);//send command byte
    Serial.write(1);//send data byte #1
    Serial.write(msb1);//send data byte #2
    cc=true;
  }
  else if (cc==true && msb1>62 && msb1<66) {
    Serial.write(controlchange);//send command byte
    Serial.write(1);//send data byte #1
    Serial.write(msb1);//send data byte #2
    cc=false;
  }

  // drums
  hitavg = analogRead(dp1);
  if(drumon==false && hitavg>drumtol){
    // turn note on with velocity, then off
    vel = (hitavg / 1023.0) * 127;
    Serial.write(NOTE_ON_CMD + 1); // +1 for midichannel 1 (default 0)
    Serial.write(31);
    Serial.write(vel);
    drumon=true;
  }
  else if (drumon==true && hitavg<=drumtol) {
    Serial.write(NOTE_OFF_CMD + 1);
    Serial.write(31);
    Serial.write(vel);
    drumon=false;
  }
  
}

// functions

void sendMidiMessage(int cmd, int channel, int lsb, int msb) {
  Serial.write(cmd + channel); // +1 for midichannel 1 (default 0)
  Serial.write(lsb);
  Serial.write(msb);
}

void scanColumn(int colNum)
{
  digitalWrite(latchPin, LOW);

  if(0 <= colNum && colNum <= 7)
  {
    shiftOut(dataPin, clockPin, MSBFIRST, B00000000); //right sr
    shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]); //left sr
  }
  else
  {
    shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum-8]); //right sr
    shiftOut(dataPin, clockPin, MSBFIRST, B00000000); //left sr
  }
  digitalWrite(latchPin, HIGH);
}

void noteOn(int row, int col)
{
  Serial.write(NOTE_ON_CMD);
  Serial.write(keyToMidiMap[row][col]);
  Serial.write(NOTE_VELOCITY);
}

void noteOff(int row, int col)
{
  Serial.write(NOTE_OFF_CMD);
  Serial.write(keyToMidiMap[row][col]);
  Serial.write(NOTE_VELOCITY);
}
