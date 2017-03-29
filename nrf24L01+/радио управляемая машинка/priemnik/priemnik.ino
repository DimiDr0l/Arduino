// Необходимо установить библиотеку для радиомодуля nRF24
#include <SPI.h>
#include "RF24.h"

#define s1 5
#define s2 6
#define s3 9
#define s4 10
#define ledpin 4
#define rezerv 3

RF24 radio(9, 10);
const uint64_t pipe = 0xE8E8F0F0E1LL;
char *msg;
int speed;
String pwm;
long timestop;

void setup()
{
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(s4, OUTPUT);
  pinMode(ledpin, OUTPUT);
  pinMode(rezerv, OUTPUT);

  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  Serial.begin (115200);
  Serial.println ("Start");
}

void loop()
{
  if (radio.available()) {
    radio.read(msg, 32);
    for (int i = 0; i < 15; i++) {  //Loop through a number of cycles
      pwm = pwm + String(msg[i]);
    }
    Serial.print (msg[0]);
    Serial.print (msg[1]);
    Serial.print (msg[2]);
    Serial.print (msg[3]);
    Serial.println (msg[4]);
  }
}
