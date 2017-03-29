#include "Servo.h"
Servo myservo;

int angle0=71;
int BatVoltagePin = 0; // пин напряжения на батареи
int CharVoltagePin = 1; // пин напряжения при зарядке
float BatVoltage, ChargeVoltage;

void setup() {
  Serial.begin(9600);
  myservo.attach(12);
  myservo.write(angle0);
  
}

void loop(){
  BatVoltage = (3.33/1024) * 2 * analogRead(BatVoltagePin);
  ChargeVoltage = (3.33/1024) * 2 * analogRead(CharVoltagePin);
  Serial.print ("BatVoltage: ");
  Serial.println (BatVoltage);
  Serial.print ("ChargeVoltage: ");
  Serial.println (ChargeVoltage);
  delay (2000);
}
