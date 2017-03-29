#include "Servo.h"
#include "LiquidCrystal.h" //библиотека дисплея
#include "Ultrasonic.h" //библиотека ультразвукового дальномера
#include "Wire.h" 
#include "CyberLib.h" //библиотека управления пинами


#define LED_init {D8_Out; D9_Out;}
#define robot_go {D8_High; D9_High;}
#define robot_stop {D8_Low; D9_Low;}
#define robot_left {D8_Low; D9_High;}
#define robot_right {D8_High; D9_Low;}

Servo myservo; 
boolean mn_con = false; //параметр задания автоматического режима
int dist; //объявление переменной дистанции

LiquidCrystal lcd(0); //инициализация дисплея
Ultrasonic ultrasonic(11,10); // инициализация дальномера

void setup() {
  myservo.attach(7)
  LED_init; //инициализация светодиодов
  Serial.begin(9600); //инициализация com порта
  lcd.setBacklight(HIGH);
  lcd.begin(16, 2);
  lcd.setCursor(0,1);
  lcd.print("Auto Control");
}

void loop() {

  dist = ultrasonic.Ranging(CM);
  int distsr=dist;

  switch (mn_con){
  case false:
    auto_control();
    break;
  default:
    manual_control(); 
  }

  dist = ultrasonic.Ranging(CM);
  if (dist>=distsr+3 || dist<=distsr-3) {
    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.setCursor(0, 0);
    lcd.print(dist);
  }
}


void auto_control(){

  if (dist<10) {
    robot_stop;
  } 
  else 
    if (dist<20) {
    robot_left;
  } 
  else
    if (dist<40) {
      robot_right;
    } 
    else 
  {
    robot_go;
  }

  if(Serial.available()>1) {
    char c = Serial.read();
    if (c=='*') {
      char c = Serial.read();
      if (c=='~') {
        mn_con = true;
        lcd.setCursor(0,1);
        lcd.print("Manual Control");
        robot_stop;
      } 
    }
  }
}             

void manual_control(){


  if(Serial.available()) {
    char c = Serial.read();
    if (c=='*') {
      delay (10);
      char c = Serial.read();
      if (c=='`') {
        mn_con = false; //включение автоматического режима
        lcd.setCursor(0,1);
        lcd.print("              ");
        lcd.setCursor(0,1);
        lcd.print("Auto Control");
      }
    }
    else 
      if (c=='w' || c=='W') { //вперёд
      robot_go;
      myservo.write(90)
      delay (100);
    }
    else 
      if (c=='s' || c=='S') { //стоп
      robot_stop; 
      myservo.write(90)
      delay (100);
    }
    else 
      if (c=='a' || c=='A') { //влево
      robot_left; 
      myservo.write(0)
      delay (100);
    }
    else 
      if (c=='d' || c=='D') { //вправо
      robot_right; 
      myservo.write(180)
      delay (100);
    }
  }
  
}







