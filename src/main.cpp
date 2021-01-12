#include <Arduino.h>
#include <SPI.h>
#include <axis_value_parser.h>
#include <zcor_protocol.h>

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

// holds the transfer response
volatile uint8_t res;

// polls a reuest waiting for an expected response
// return the response
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

//================================================= MAIN LOOP

AxisValueParser avp = AxisValueParser();

void loop() {
  Serial.println("Request reset");
  if(!waitResponse(REQUEST_RESET,RESPONSE_RESET,1000)) {
    Serial.println("Reset timeout");
    return;
  }
  Serial.println("Reset ok");
  // Request position pos continuously
  uint8_t pos = 1;
  while(true) {
    Serial.println("Request position");
    transfer(REQUEST_POSITION_READ(pos));
    if(!waitResponse(REQUEST_POSITION_STATUS,RESPONSE_POSITION_STATUS_OK(pos),5000)) {
      Serial.println("Position aquire timeout");
      continue;
    }
    Serial.println("Position aquired");
    avp.init();
    while(!avp.verify()) {
      res = transfer(REQUEST_POSITION_DIGIT);
      //Serial.println(res);
      if(RESPONSE_IS_POSITION_DIGIT(res)) {
        avp.add(RESPONSE_POSITION_DIGIT(res));
      }
      // delay(50);
    }
    float value = avp.pos();
    Serial.println(value);
    delay(2000);
  }
}