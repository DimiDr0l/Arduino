#include "EEPROM.h"

#define s1 3 //пины клавиатуры
#define s2 4
#define s3 5
#define s4 6
#define s5 7
#define s6 8 //режим записи кода
#define ledEeprom 12 //светодиод записи
#define ledRelay 13 //светодиод открытия замка
#define relayPin 11 //контакт реле
long pressButtonEEpromTimer, debounceDelay;
boolean timer = true, relay, b1, b2, b3, b4, b5, b6, val1 = true, val2 = true, val3 = true, val4 = true, val5 = true, val6 = true, valMode = true, eepromMode = false;
byte key, i, passLength = 3;
byte password[3], passPres[3];

void setup() {
  Serial.begin (115200);
  pinMode (s1, INPUT_PULLUP);
  pinMode (s2, INPUT_PULLUP);
  pinMode (s3, INPUT_PULLUP);
  pinMode (s4, INPUT_PULLUP);
  pinMode (s5, INPUT_PULLUP);
  pinMode (s6, INPUT_PULLUP);
  pinMode (ledEeprom, OUTPUT);
  pinMode (ledRelay, OUTPUT);
  pinMode (relayPin, OUTPUT);
  close ();
 for (i = 1; i <= passLength; i++){
  password[i] = EEPROM.read(i);
  }
}


void loop() {
  readButtons();
  if (key != 0 ) {
	if (eepromMode) {
	i++;
	EEPROM.write(key,i);
	password[i] = key;
	Serial.println (password[i]);
	if (i == passLength) {
		eepromMode = !eepromMode;
		digitalWrite (ledEeprom, eepromMode);
		i=0;
	}
  }
 else {
 i++;
 passPres[i] = key;
 if (i == passLength) {
 for (i = 1; i <= passLength; i++)
{
if (password[i] != passPres[i]) {
i=0;
blinkErr();
break;
} else if (password[i] == passPres[passLength]) {
open();
}
}
 }
 }
 }
}

void open(){
digitalWrite (relayPin, LOW);
digitalWrite (ledRelay, HIGH);
digitalWrite (ledEeprom, HIGH);
Serial.println ("Open");
delay (3000);
}

void close(){
delay (1000);
digitalWrite (relayPin, HIGH);
 blinkErr();
Serial.println ("Close");

}

void blink() {
  digitalWrite (ledRelay, HIGH);
  delay (100);
  digitalWrite (ledRelay, LOW);
}
void blinkErr() {
  digitalWrite (ledRelay, HIGH);
  digitalWrite (ledEeprom, HIGH);
  delay (100);
  digitalWrite (ledRelay, LOW);
  digitalWrite (ledEeprom, LOW);
}
void readButtons() {
  b1 = digitalRead (s1);
  b2 = digitalRead (s2);
  b3 = digitalRead (s3);
  b4 = digitalRead (s4);
  b5 = digitalRead (s5);
  b6 = digitalRead (s6);

  if ((!b1 || !b2 || !b3 || !b4 || !b5 || !b6) && timer) {
    debounceDelay = millis();
    timer = false;
  }
  if (millis() - debounceDelay > 100) {


    if (!b5 && !b6 && valMode) {
      pressButtonEEpromTimer = millis();
      valMode = false;
    }
    else if (!b5 && !b6 && !valMode) {
      if (millis() - pressButtonEEpromTimer > 3000) {
        valMode = true;
		i = 0;
        eepromMode = !eepromMode;
        digitalWrite (ledEeprom, eepromMode);
        delay (2000);
        readButtons();
      }
    }
    else if (!b1 && val1) {
      val1 = false;
    }
    else if (b1 && !val1) {
      val1 = true;
      key = 1;
      blink ();
    }
    else if (!b2 && val2) {
      val2 = false;
    }
    else if (b2 && !val2) {
      val2 = true;
      key = 2;
      blink ();
    }
    else  if (!b3 && val3) {
      val3 = false;
    }
    else if (b3 && !val3) {
      val3 = true;
      key = 3;
      blink ();
    }
    else  if (!b4 && val4) {
      val4 = false;
    }
    else if (b4 && !val4) {
      val4 = true;
      key = 4;
      blink ();
    }
    else  if (!b5 && val5) {
      val5 = false;
    }
    else if (b5 && !val5) {
      val5 = true;
      key = 5;
      blink ();
    }
    else  if (!b6 && val6) {
      val6 = false;
    }
    else if (b6 && !val6) {
      val6 = true;
      key = 6;
      blink ();
    }
    else {
      key = 0;
      valMode = true;
    }
    timer = true;
  }
}























