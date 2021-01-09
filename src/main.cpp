#include <Arduino.h>
#include <SPI.h>

#define BAUD 9600

//================================================= SPI DEFINITIONS

// Macros for bit masks
#define CBI(n,b) (n &= ~_BV(b))
#define TEST(n,b) !!((n)&_BV(b))

#define OUT_WRITE(IO,V)     do{ pinMode(IO, OUTPUT); digitalWrite(IO,V); }while(0)
#define SET_INPUT(IO)       do{ pinMode(IO, INPUT); }while(0)
#define WRITE(IO, V)        do{ digitalWrite(IO, V); }while(0)

inline uint8_t transfer(uint8_t data) {
  SPDR = data;
  asm volatile("nop");
  while (!TEST(SPSR, SPIF)) { /* nada */ }
  return SPDR;
}

//================================================= SETUP

void setup() {
  Serial.begin(BAUD);
  SPI.begin();
  OUT_WRITE(SCK, LOW);
  OUT_WRITE(MOSI, HIGH);
  SET_INPUT(MISO);
  WRITE(MISO, HIGH);

  CBI(
    #ifdef PRR
      PRR
    #elif defined(PRR0)
      PRR0
    #endif
      , PRSPI);
  SPCR = _BV(MSTR) | _BV(SPE) | _BV(SPR0);
  digitalWrite(SS, LOW);
  Serial.println("start");
}

//================================================= SPI PROTOCOL

// holds the transfer response
volatile uint8_t res;

#define AXIS_VALUE_SIZE 6
#define AXIS_VALUE_DECIMALS 2
#define AXIS_VALUE_VERIFY_CYCLES 2
// current axis value (eg: 101.43)
// negative values not allowed!
// padded with zeroes
char axisValue[AXIS_VALUE_SIZE];
// checks if the axisValue was filled correctly
// axisValue is correct when it passes a few
// cycles (AXIS_VALUE_VERIFY_CYCLES) without changing
unsigned int axisValueVerify[AXIS_VALUE_SIZE];
// fills axisValue array with 0;
// fills axisValueVerify array with 0;
void initAxisValue();
// current index for the axis value
volatile unsigned int axisValueIndex = 0;
// increments the index for the axis value,
// making shure to start over if the end was reached
void incrementAxisValueIndex();
// adds a value and shifts the index
void addAxisValue(char value);
// checks if axisValue elements have matched for
// at least AXIS_VALUE_VERIFY_CYCLES cycles
bool verifyAxisValue();
// shifts the values while the dot is in the correct position
void shiftAxisValue();
// polls a reuest waiting for an expected response
// return the response
uint8_t waitResponse(uint8_t poll, uint8_t expected, unsigned long timeout);

//================================================= MAIN LOOP

void loop() {
  Serial.println("Request reset");
  transfer(0);
  if(waitResponse(0,0,1000)) {
    Serial.println("Reset ok");
    uint8_t requestPosition = 1;
    while(true) {
      Serial.println("Request position");
      transfer(requestPosition);
      if(waitResponse(10,10 + requestPosition,5000)) {
        Serial.println("Position aquired");
        initAxisValue();
        while(!verifyAxisValue()) {
          res = transfer(100);
          //Serial.println(res);
          if(res>100) {
            addAxisValue(res%100);
          }
          // delay(50);
        }
        shiftAxisValue();
        float value = atof(axisValue);
        Serial.println(value);
        delay(2000);
      } else {
        Serial.println("Position aquire timeout");
      }
    }
  } else {
    Serial.println("Reset timeout");
  }
}

//================================================= IMPLEMENTATIONS

void initAxisValue() {
  for(unsigned int i=0; i<AXIS_VALUE_SIZE; i++) {
    axisValue[i] = 0;
    axisValueVerify[i] = 0;
  }
}

void incrementAxisValueIndex() {
  if(axisValueIndex < AXIS_VALUE_SIZE-1){
    axisValueIndex++;
  } else {
    axisValueIndex=0;
  }
}

void addAxisValue(char value) {
  if(axisValue[axisValueIndex] != value) {
    axisValue[axisValueIndex] = value;
    axisValueVerify[axisValueIndex] = 0;
  } else {
    axisValueVerify[axisValueIndex] ++;
  }
  incrementAxisValueIndex();
}

bool verifyAxisValue() {
  bool hasCycles = false;
  for(unsigned int i=0; i<AXIS_VALUE_SIZE && !hasCycles; i++) {
    if(axisValueVerify[i]<AXIS_VALUE_VERIFY_CYCLES) hasCycles = true;
  }
  return !hasCycles;
}

void shiftAxisValue(){
  while(axisValue[AXIS_VALUE_SIZE-AXIS_VALUE_DECIMALS-1] != '.'){
    const char temp = axisValue[0];
    for(unsigned int i=1; i<AXIS_VALUE_SIZE; i++) {
      axisValue[i-1] = axisValue[i];
    }
    axisValue[AXIS_VALUE_SIZE-1] = temp;
  }
}

uint8_t waitResponse(uint8_t poll, uint8_t expected, unsigned long timeout) {
  timeout += millis();
  while(timeout > millis()) {
    res = transfer(poll);
    // Serial.println(res);
    if(res==expected) {
      return true;
    } else {
      delay(50);
    }
  }
  return false;
}