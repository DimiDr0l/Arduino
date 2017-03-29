#include <AFMotor.h>
boolean button1State;
boolean button2State, forward;
int i;

long timer;
long timerPushButton;
AF_Stepper motor(48, 2);

void setup() {
  Serial.begin(115200);
  pinMode (A0, INPUT_PULLUP);
  pinMode (A1, INPUT_PULLUP);
  motor.setSpeed(2000);
}

void loop() {
  if (button1State == true && digitalRead(A0) == false){ 
   forward = false;
   button1State = false;
   Serial.println ("HIGH1");
  }
  else if (button1State == false && digitalRead(A0) == true){
   button1State = true;
   Serial.println ("LOW1");
  }
   
  if (button2State == true && digitalRead(A1) == false){ 
   forward = true;
   button2State = false;
   Serial.println ("HIGH2");
  }
  else if (button2State == false && digitalRead(A1) == true){
   button2State = true;
   Serial.println ("LOW2");
  }
  
  if (forward == true) {
    i++;
  motor.step(1, FORWARD, MICROSTEP);
  } else {
    i--;
  motor.step(1, BACKWARD, MICROSTEP);
  }
  delay (10);
  //motor.step(290, FORWARD, MICROSTEP);
  //motor.step(290, BACKWARD, MICROSTEP);
}