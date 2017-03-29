#include "fix_fft.h"
#include "CyberLib.h"

char im[128], data[128];
int i = 0, val;
int out = 0;

int redPin = 5;          // pins that the LED are attached to
int greenPin = 6;
int bluePin = 3;

void setup() {
  Serial.begin(115200);
  D5_Out; //red
  D2_Out; //green
  D8_Out; //blue
}

void loop() {
  for (i = 0; i < 128; i++) {
    val = A0_Read;
    data[i] = val;
    im[i] = 0;
  }

  fix_fft(data, im, 7, 0);

  for (i = 1; i < 64; i++) {
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);

    if (i == 2) {
      out = data[i];
      Serial.print("Low - ");
      Serial.println(out);
      Serial.println(val);
      if (out <= 6) D5_Low;
      if (out >= 8) D5_High;
    }

    if (i == 30) {
      out = data[i];
      Serial.print("Mid - ");
      Serial.println(out);
      Serial.println(val);
      if (out <= 2) D8_Low;
      if (out >= 4) D8_High;
    }

    if (i == 58) {
      out = data[i];
      Serial.print("Hi - ");
      Serial.println(out);
      Serial.println(val);
      if (out <= 3) D2_Low;
      if (out >= 5) D2_High;
    }
  }
}
