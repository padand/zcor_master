#include <Arduino.h>
#include <SPI.h>

#define BAUD 9600

#define ECHO_PAIR(A,B) do{ Serial.print(A); Serial.print(": "); Serial.println(B); }while(false)

void setup() {
  Serial.begin(BAUD);
  SPI.begin();
  digitalWrite(SS, LOW);
  Serial.println("start");
}

char req;
char res;

void loop() {
  if(Serial.available()) {
    Serial.readBytes(&req, 1);
    ECHO_PAIR("Read byte", req);
    
    res = SPI.transfer(req);
    ECHO_PAIR("Got response", res);
  }
}