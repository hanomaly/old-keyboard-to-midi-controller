// midi channels
const int CHANNEL1 = 0;
const int CHANNEL2 = 1;

// pitch bend
const int PITCHBEND_CMD = 224;
const int PB_LSB = 0;
int PB_VALUE = 64;
bool PBisOn = false;

// control change 
const int CONTROLCHANGE1_CMD = 176;
const int CC_LSB = 1;
int CC_VALUE = 64;
bool CCisOn = false;

// notes
const int NOTE_ON_CMD = 144;
const int NOTE_OFF_CMD = 128;
const int NOTE_VELOCITY = 127;

// drums
int DRUM_VELOCITY =  0;
int DRUM_NOTE[] = {31,32,33,34,35,36}; 
const int DRUM_TOL = 19;
bool drumOn[] = {false,false,false,false,false,false};

// joystick analog pins
const int X_pin = 0;
const int Y_pin = 1; 

// drum pins
const int drumPin[] = {2,3,4,5,6,7};

// keyboard matrix
const int NUM_ROWS = 8;
const int NUM_COLS = 7;

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

// MIDI baud rate
const int SERIAL_RATE = 31250; // needs to be 31250 when going through midi port, can be 9600 for hairless

void setup()
{
  
  int note = 28; 
  
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

  Serial.begin(SERIAL_RATE);
}

void loop()
{
  
  // keyboard
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
        sendMidiMessage(NOTE_ON_CMD, CHANNEL1, keyToMidiMap[rowCtr][colCtr], NOTE_VELOCITY);
      }
    }

    // process keys released
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] == 0 && keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = false;
        sendMidiMessage(NOTE_OFF_CMD, CHANNEL1, keyToMidiMap[rowCtr][colCtr], NOTE_VELOCITY);
      }
    }
  }

  // joystick X-axis - pitch bend
  PB_VALUE = (analogRead(X_pin) / 1024.0) * 127;
  if (PB_VALUE < 53 || PB_VALUE > 56) {
    sendMidiMessage(PITCHBEND_CMD,CHANNEL1,PB_LSB,PB_VALUE);
    PBisOn=true;
  }
  else if (PBisOn==true && PB_VALUE > 53 && PB_VALUE < 56) {
    sendMidiMessage(PITCHBEND_CMD,CHANNEL1,PB_LSB,PB_VALUE);
    PBisOn=false;
  }

  // joystick Y-axis - control change
  CC_VALUE = (analogRead(Y_pin) / 1024.0) * 127;
  if (CC_VALUE < 53 || CC_VALUE > 56) {
    sendMidiMessage(CONTROLCHANGE1_CMD,CHANNEL1,CC_LSB,CC_VALUE);
    CCisOn=true;
  }
  else if ( CCisOn == true && CC_VALUE > 53 && CC_VALUE < 56) {
    sendMidiMessage(CONTROLCHANGE1_CMD,CHANNEL1,CC_LSB,CC_VALUE);
    CCisOn=false;
  }

  // drums
  for(int drumNo = 0; drumNo < 6; ++drumNo){
    DRUM_VELOCITY = (analogRead(drumPin[drumNo]) / 1023.0) * 127;
    if( drumOn[drumNo]==false && DRUM_VELOCITY > DRUM_TOL ){
      sendMidiMessage(NOTE_ON_CMD,CHANNEL2,DRUM_NOTE[drumNo],DRUM_VELOCITY);
      drumOn[drumNo]=true;
    }
    else if ( drumOn[drumNo]==true && DRUM_VELOCITY <= DRUM_TOL ) {
      sendMidiMessage(NOTE_OFF_CMD,CHANNEL2,DRUM_NOTE[drumNo],DRUM_VELOCITY);
      drumOn[drumNo]=false;
    }
  }
}

// functions

void sendMidiMessage(int cmd, int channel, int lsb, int msb) {
  Serial.write(cmd + channel); // send command plus the channel number
  Serial.write(lsb); // least significant bit 
  Serial.write(msb); // most significant bit
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
