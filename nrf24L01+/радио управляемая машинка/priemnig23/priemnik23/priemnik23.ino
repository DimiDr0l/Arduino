// Необходимо установить библиотеку для радиомодуля nRF24
#include <SPI.h>
#include "RF24.h"

#define s1 5
#define s2 6
#define s3 9
#define s4 10
#define ledpin 4
#define rezerv 3

RF24 radio(7, 8);
const uint64_t pipe = 0xE8E8F0F0E1LL;
char *msg;
int speed;
int pwm;
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
    radio.read(msg, 4);
    Serial.print (msg[0]);
    Serial.print ("-");
    pwm = (String(msg[1]) + String(msg[2]) + String(msg[3])).toInt() - 100;
    //Serial.println (pwm);
  }  else msg[0] = 0;

  if (msg[0] != 0) {
    timestop = millis();
    switch (char c = msg[0]) {
      case 'w':
        forward (pwm);
        break;
      case 's':
        backward (pwm);
        break;
      case 'a':
        left (pwm);
        break;
      case 'd':
        right (pwm);
        break;
      case 'l':
        digitalWrite (ledpin, !digitalRead (ledpin));
        break;
    }
  } else if (millis() - timestop > 100 ) stop();
}

void stop()
{
  analogWrite(s1, 0);
  analogWrite(s2, 0);
  analogWrite(s3, 0);
  analogWrite(s4, 0);
}

void forward(int speed)
{
  Serial.println (speed);
  analogWrite(s1, speed);
  analogWrite(s2, 0);
  analogWrite(s3, speed);
  analogWrite(s4, 0);
}

void backward(int speed)
{
  analogWrite(s1, 0);
  analogWrite(s2, speed);
  analogWrite(s3, 0);
  analogWrite(s4, speed);
}

void left(int speed)
{
  analogWrite(s1, speed);
  analogWrite(s2, 0);
  analogWrite(s3, 0);
  analogWrite(s4, speed);
}

void right(int speed)
{
  analogWrite(s1, 0);
  analogWrite(s2, speed);
  analogWrite(s3, speed);
  analogWrite(s4, 0);
}
