#include <SPI.h>
#include "RF24.h"
RF24 radio(9, 10);
const uint64_t pipe = 0xE8E8F0F0E1LL;
#define speedPWM A0
#define pl 4 //свет
#define pw 5
#define ps 6
#define pa 7
#define pd 8
#define led 13
boolean v1, v2, v3 , v4, v5, lightbooton = false;
int speed;
String msg;
char char_array[5];

void setup(void) {
  pinMode(pw, INPUT_PULLUP);
  pinMode(ps, INPUT_PULLUP);
  pinMode(pa, INPUT_PULLUP);
  pinMode(pd, INPUT_PULLUP);
  pinMode(pl, INPUT_PULLUP); //light
  pinMode (led, OUTPUT);
  radio.begin();
  radio.openWritingPipe(pipe);
  Serial.begin (115200);
  Serial.println ("Start");
}

void loop() {
  speed = 255; //map(analogRead(speedPWM), 0, 1023, 0, 255) + 100;
  v1 = digitalRead (pw);
  v2 = digitalRead (ps);
  v3 = digitalRead (pa);
  v4 = digitalRead (pd);
  v5 = digitalRead (pl);
  if (!v1) {
    msg = "w" + String(speed);  ;
  } else if (!v2) {
    msg = "s" + String(speed);  ;
  } else if (!v3) {
    msg = "a" + String(speed);  ;
  } else if (!v4) {
    msg = "d" + String(speed);  ;
  } else if (!v5) {
    lightbooton = true;
  } else if (v5 && lightbooton) {
    msg = "l";
    lightbooton = false;
  } else msg = "";
  msg.toCharArray(char_array, sizeof(msg) + 1);
  if (msg != "") {
    radio.write(char_array, sizeof(char_array));
    //Serial.println(char_array);
  }
}

