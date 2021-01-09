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
// current axis value (eg: 101.43)
// negative values not allowed!
char axisValue[AXIS_VALUE_SIZE];
// current index for the axis value
volatile unsigned int axisValueIndex = 0;
// fills axisValue array with 0;
void initAxisValue();
// checks if axisValue array has no more zeroes left in it
bool isAxisValueFilled();

//================================================= MAIN LOOP

void loop() {
  transfer(1);
  bool isReady = false;
  while(!isReady) {
    res = transfer(10);
    if(res==11) {
      isReady = true;
    }
    delay(100);
  }
  initAxisValue();
  while(!isAxisValueFilled()) {
    res = transfer(100);
    if(res>100) {
      axisValue[res/100] = res%100;
    }
    delay(50);
  }
  Serial.println(axisValue);
  delay(500);
}

//================================================= IMPLEMENTATIONS

void initAxisValue() {
  for(unsigned int i=0; i<AXIS_VALUE_SIZE; i++) {
    axisValue[i] = 0;
  }
}

bool isAxisValueFilled() {
  bool hasZero = false;
  for(unsigned int i=0; i<AXIS_VALUE_SIZE && !hasZero; i++) {
    if(axisValue[i]==0) hasZero = true;
  }
  return hasZero;
}