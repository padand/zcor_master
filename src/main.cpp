#include <Arduino.h>
#include <SPI.h>

#define BAUD 9600

// Macros for bit masks
#define CBI(n,b) (n &= ~_BV(b))
#define TEST(n,b) !!((n)&_BV(b))

#define OUT_WRITE(IO,V)       do{ pinMode(IO, OUTPUT); digitalWrite(IO,V); }while(0)
#define SET_INPUT(IO)       do{ pinMode(IO, INPUT); }while(0)
#define WRITE(IO, V)          do{ digitalWrite(IO, V); }while(0)

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
  digitalWrite(SS, LOW);
}

inline uint8_t receive() {
  SPDR = 0;
  asm volatile("nop");
  while (!TEST(SPSR, SPIF)) { /* nada */ }
  Serial.print(SPDR);
  return SPDR;
}

char req;
char res;
char decoded[100];

void spiDecode() {
  // decode data
  unsigned int i=0;
  do {
    res = receive();
  } while(res!='^');
  do {
    res = receive();
    decoded[i] = res;
    i++;
  } while(res!='$');
  decoded[i-1] = '\0';
  Serial.println("HERE");
  Serial.println(decoded);
}

void loop() {
  // res = receive();
  // Serial.print(res);
  spiDecode();
  delay(500);
}