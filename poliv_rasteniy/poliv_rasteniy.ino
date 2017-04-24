#include "avr/wdt.h"
#include "SPI.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#include "CyberLib.h"
#include "iarduino_RTC.h"

iarduino_RTC RTC(RTC_DS1307);
LiquidCrystal_I2C lcd(0x27,16,2);  // дисплей подключить к А4 А5
DHT dht(7, DHT22);      //датчик темп и влажности воздуха подключается к D7
#define pullPin 8     //pin дергания датчика влажности
#define humPin  A7      //pin датчика влажности земли
#define waterPin 2      //pin насоса
#define airPin 3      //pin вытяжки
#define heatPin 4     //pin обогревателя
#define lightPin 6      //pin освещения PWM
#define ledPin 5      //светодиод работы насоса
#define buttonUp      A3  //кнопки
#define buttonDown    A2
#define buttonLeft    A1
#define buttonRight   A0

unsigned long currentMillis;
long timer1 = -3000; //задержка считывания показаний датчиков температуры 3 сек
byte timer2 = 10; //таймер для считывания влажности земли
long previousMillis = 0; //счетчик прошедшего времени для AutoMainScreen
long interval = 10000; //задержка автовозврата к MainScreen 10 сек
boolean timerbt = true, b1, b2, b3, b4, val1 = true, val2 = true, val3 = true, val4 = true; //кнопки

float hum_atmosphere;
float temp_atmosphere;
int hum_ground;
byte m = 1; //переменная для экранов меню
byte m_qt = 7; //количество меню
byte tlighton, tlightoff; //время вкл и выкл света
byte t1; //переменная для задания температуры воздуха
byte h1; //переменная для задания влажности воздуха
byte h2; //переменная для задания влажности земли
byte VAR_mode_SET = 0;           // режим установки времени: 0-нет 1-сек 2-мин 3-час 4-день 5-мес 6-год 7-день_недели


void setup() {
  pinMode (buttonUp, INPUT_PULLUP); //pin кнопки для смены экрана меню
  pinMode (buttonDown, INPUT_PULLUP);
  pinMode (buttonLeft, INPUT_PULLUP);
  pinMode (buttonRight, INPUT_PULLUP);

  pinMode (humPin, INPUT);  //pin считывания влажности земли
  pinMode(pullPin, OUTPUT);   //pin дергания датчика влажности
  pinMode(heatPin, OUTPUT); //pin обогревателя
  pinMode(waterPin, OUTPUT);  //pin насоса
  pinMode(ledPin, OUTPUT);  //светодиод работы насоса
  pinMode(lightPin, OUTPUT); //освещение
  pinMode(airPin, OUTPUT);  //вытяжка
  Serial.begin(115200);
  t1 = ReadEEPROM_Byte(0);
  h1 = ReadEEPROM_Byte(1);
  h2 = ReadEEPROM_Byte(2);
  tlighton = ReadEEPROM_Byte(3);
  tlightoff = ReadEEPROM_Byte(4);
  RTC.begin();
  dht.begin(); //датчик темп и влажности воздуха подключается к D7
  lcd.init(); //дисплей
  lcd.setCursor(4, 0);
  lcd.print("START");
  Serial.println("START");
  delay_ms (1000);//Задержка приветствия
  lcd.clear();
  wdt_enable (WDTO_8S);   //вкл авторесета через 8 сек.
}

void menu() {
  //Вывод меню
  //Описание экранов меню
  switch (m) {
  case 1: //переменная m=1
    lcd.setCursor(0, 0);
    lcd.print("T1=");     //*******************
    lcd.print(temp_atmosphere, 1);
    lcd.print("c");
    lcd.setCursor(0, 1);      //*T1=temp_atmosphere H1=hum_atmosphere H2=hum_ground;*
    lcd.print("H1=");
    lcd.print(hum_atmosphere, 1);
    lcd.print("% ");
    lcd.print("H2=");
    lcd.print(hum_ground);
    lcd.print("%");
    break;
  case 2:                 //переменная m=2
    lcd.setCursor(0, 0);      //***********************
    lcd.print("Temp atmos");  //*Температура воздуха  *
    lcd.setCursor(0, 1);      //*T1=t1              *
    lcd.print("T1 = ");       //***********************
    lcd.print(t1);
    lcd.print("c");
    break;
  case 3:                 //переменная m=3
    lcd.setCursor(0, 0);      //*******************
    lcd.print("Hum atmos");   //*Влажность воздуха  *
    lcd.setCursor(0, 1);      //*H1=h1            *
    lcd.print("H1 = ");       //*******************
    lcd.print(h1);
    lcd.print("%");
    break;
  case 4:                 //переменная m=4
    lcd.setCursor(0, 0);      //*******************
    lcd.print("Hum ground");  //*Влажность земли  *
    lcd.setCursor(0, 1);      //*H2=h2            *
    lcd.print("H2 = ");       //*******************
    lcd.print(h2);
    lcd.print("%");
    break;
  case 5:
    lcd.setCursor(1, 0);
    lcd.print("Light-ON Time");
    lcd.setCursor(6, 1);
    lcd.print(tlighton);
    lcd.print(":00");
    break;
  case 6:
    lcd.setCursor(1, 0);
    lcd.print("Light-OFF Time");
    lcd.setCursor(6, 1);
    lcd.print(tlightoff);
    lcd.print(":00");
    break;
  case 7:                 //переменная m=5
    RTC.blinktime(VAR_mode_SET);
    lcd.setCursor (3, 0);
    lcd.print(RTC.gettime("d-m-Y   D"));
    lcd.setCursor (4, 1);
    lcd.print(RTC.gettime("H:i:s"));
    break;
  }

  //Проверка автовозврата
  if (currentMillis - previousMillis > interval && m != 1) {
    previousMillis = currentMillis;
    m = 1;
    lcd.clear();
  }
  else if (currentMillis - previousMillis > 15000) lcd.setBacklight(LOW); //отключение подсветки через 15 сек
  else lcd.setBacklight(HIGH);
}

void readSensors() {
  timer2++;
  if (timer2 >= 10) { //проверка таймера считывания датчика влажности каждые 30 сек (3сек*10)
    digitalWrite(pullPin, HIGH); //дергание ногой датчика влажности земли
    digitalWrite(ledPin, HIGH); //индикация считывания с датчика влажности
    delay (100);
    hum_ground = analogRead(humPin);
    digitalWrite(pullPin, LOW);
    digitalWrite(ledPin, LOW); //отключение индикации считывания с датчиков
    hum_ground = map(hum_ground, 100, 1000, 100, 0);
    if (hum_ground < h2) {
      pump_on();
      Serial.println("Water ON");
    } else {
      pump_off();
      Serial.println("Water OFF");
    }
    timer2 = 0;
  }

  hum_atmosphere = dht.readHumidity();
  temp_atmosphere = dht.readTemperature();
  if (isnan(hum_atmosphere) || isnan(temp_atmosphere)) { //проверка датчика DHT22 УБРАТЬ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    digitalWrite(heatPin, LOW); //выключение обогревателя
    digitalWrite(airPin, LOW); //выключение вентилятора
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("DHT ERROR!");
    lcd.setCursor(4, 1);
    lcd.print("ACHTUNG!");
    digitalWrite(ledPin, HIGH);
    lcd.setBacklight(HIGH);
    delay_ms (300);
    digitalWrite(ledPin, LOW);
    lcd.setBacklight(LOW);
    delay_ms (300);
    //lcd.clear();
    wdt_reset();
    readSensors();
  }
}

void checkSensors() {
RTC.gettime();
  if (temp_atmosphere < t1 - 0.5) {
    digitalWrite(airPin, HIGH);   //включение вентилятора
    digitalWrite(heatPin, HIGH);  //включение обогревателя
    Serial.println("Heat ON  Air ON");
  }
  else if (temp_atmosphere >= t1 + 1 || hum_atmosphere > h1) {
    digitalWrite(heatPin, LOW);   //выключение обогревателя
    digitalWrite(airPin, HIGH);   //включение вентилятора
    Serial.println("Heat OFF  Air ON");
  }
  else if ((temp_atmosphere <= t1 + 2 && temp_atmosphere >= t1) && hum_atmosphere <= h1) {
    digitalWrite(heatPin, LOW);   //выключение обогревателя
    digitalWrite(airPin, LOW);    //выключение вентилятора
    Serial.println("Heat OFF  Air OFF");
  }
  if (RTC.Hours == tlighton && RTC.minutes == 0) {
  digitalWrite (lightPin, HIGH);
  Serial.println("Light ON");
  }
  
  if (RTC.Hours == tlightoff && RTC.minutes == 0) {
  digitalWrite (lightPin, LOW);
  Serial.println("Light OFF");
  }
}

void pump_on() {
  digitalWrite(waterPin, HIGH); //включение насоса
  //digitalWrite(ledPin, HIGH);
}
void pump_off() {
  digitalWrite(waterPin, LOW); //отключение
  //digitalWrite(ledPin, LOW);
}

void ReadButtons() {
  uint8_t i=0;
  b1 = digitalRead (buttonUp);
  b2 = digitalRead (buttonDown);
  b3 = digitalRead (buttonLeft);
  b4 = digitalRead (buttonRight);
  if (!b1 && val1) {
    val1 = false;
  }
  else if (b1 && !val1) {
    val1 = true;
    previousMillis = currentMillis;
    if (m == 2) {
      t1++;
      lcd.clear();
      WriteEEPROM_Byte(0, t1);
      if (t1 > 40) t1 = 20; //устанавливаем предел изменения переменной = 40
    }
    //UP для влажности H1 воздуха
    if (m == 3) {
      h1 = h1 + 5;
      lcd.clear();
      if (h1 > 100) h1 = 20;
      WriteEEPROM_Byte(1, h1);
    }
    //UP для влажности H2 земли
    if (m == 4) {
      h2 = h2 + 5;
      lcd.clear();
      if (h2 > 100) h2 = 10;
      WriteEEPROM_Byte(2, h2);
    }
    
    if (m == 5) {
      tlighton = tlighton + 1;
      lcd.clear();
      if (tlighton > 23) tlighton = 0;
      WriteEEPROM_Byte (3, tlighton);
    }
    
    if (m == 6) {
      tlightoff = tlightoff + 1;
      lcd.clear();
      if (tlightoff > 23) tlightoff = 0;
      WriteEEPROM_Byte (4, tlightoff);
    }
    
    if (m == 7) {
      switch (VAR_mode_SET){  // инкремент (увеличение) устанавливаемого значения
        /* сек */ case 1: RTC.settime(0,                                   -1, -1, -1, -1, -1, -1); break;
        /* мин */ case 2: RTC.settime(-1, (RTC.minutes==59?0:RTC.minutes+1), -1, -1, -1, -1, -1); break;
        /* час */ case 3: RTC.settime(-1, -1, (RTC.Hours==23?0:RTC.Hours+1),     -1, -1, -1, -1); break;
        /* дни */ case 4: RTC.settime(-1, -1, -1, (RTC.day==31?1:RTC.day+1),         -1, -1, -1); break;
        /* мес */ case 5: RTC.settime(-1, -1, -1, -1, (RTC.month==12?1:RTC.month+1),     -1, -1); break;
        /* год */ case 6: RTC.settime(-1, -1, -1, -1, -1, (RTC.year==99?0:RTC.year+1),       -1); break;
        /* д.н.*/ case 7: RTC.settime(-1, -1, -1, -1, -1, -1, (RTC.weekday==6?0:RTC.weekday+1) ); break;
      }
    }
  }
  else if (!b2 && val2) {
    val2 = false;
  }
  else if (b2 && !val2) {
    val2 = true;
    previousMillis = currentMillis;
    if (m == 2) {     //если находимся на экране с переменной t1
      t1--;
      lcd.clear();
      if (t1 < 20) t1 = 40; //устанавливаем предел изменения переменной = 20
      WriteEEPROM_Byte(0, t1);
    }
    //down для H1
    if (m == 3) {
      h1 = h1 - 5;

      lcd.clear();
      if (h1 < 20) h1 = 100;
      WriteEEPROM_Byte(1, h1);
    }
    //down для H2
    if (m == 4) {
      h2 = h2 - 5;
      lcd.clear();
      if (h2 < 10) h2 = 100;
      WriteEEPROM_Byte(2, h2);
    }
    
    if (m == 5) {
      tlighton--;
      lcd.clear();
      if (tlighton < 0) tlighton = 23;
      WriteEEPROM_Byte (3, tlighton);
    }
    
    if (m == 6) {
      tlightoff--;
      lcd.clear();
      if (tlightoff < 0) tlightoff = 23;
      WriteEEPROM_Byte (4, tlightoff);
    }
    
    if (m == 7) {
      switch (VAR_mode_SET){  // декремент (уменьшение) устанавливаемого значения
        /* сек */ case 1: RTC.settime(0,                                   -1, -1, -1, -1, -1, -1); break;
        /* мин */ case 2: RTC.settime(-1, (RTC.minutes==0?59:RTC.minutes-1), -1, -1, -1, -1, -1); break;
        /* час */ case 3: RTC.settime(-1, -1, (RTC.Hours==0?23:RTC.Hours-1),     -1, -1, -1, -1); break;
        /* дни */ case 4: RTC.settime(-1, -1, -1, (RTC.day==1?31:RTC.day-1),         -1, -1, -1); break;
        /* мес */ case 5: RTC.settime(-1, -1, -1, -1, (RTC.month==1?12:RTC.month-1),     -1, -1); break;
        /* год */ case 6: RTC.settime(-1, -1, -1, -1, -1, (RTC.year==0?99:RTC.year-1),       -1); break;
        /* д.н.*/ case 7: RTC.settime(-1, -1, -1, -1, -1, -1, (RTC.weekday==0?6:RTC.weekday-1) ); break;
      }
    }
  }
  else  if (!b3 && val3) {
    val3 = false;
  }
  else if (b3 && !val3) {
    val3 = true;
    m--;
    previousMillis = currentMillis;
    lcd.clear();
    if (m < 1) m = m_qt;
  }
  else  if (!b4 && val4) {
    val4 = false;
    previousMillis = currentMillis; //если кнопка была нажата сбросить счетчик автовозврата к главному экрану
    
  }
  else if (!b4 && m == 7) {
    while(!digitalRead (buttonRight)){
      delay(10);
      i++;
      if(i>200){digitalWrite(ledPin, HIGH);}
    }
    digitalWrite(ledPin, LOW);
    if(i>200){                //если кнопка buttonRight удерживалась больше 2 секунд
      val4 = true;
      VAR_mode_SET++;           // переходим к следующему устанавливаемому параметру
      if(VAR_mode_SET>7){VAR_mode_SET=1;} // возвращаемся к первому устанавливаемому параметру
    }else{                  // если кнопка buttonRight удерживалась дольше 2 секунд, то требуется выйти из режима установки даты/времени
      VAR_mode_SET=0;           // выходим из режима установки даты/времени
    }
  }
  else if (b4 && !val4) {
    val4 = true;
    m++;
    lcd.clear();
    if (m > m_qt) m = 1; //если уровень меню больше m_qt
  }
}

void loop() {
  currentMillis = millis();
  ReadButtons();
  menu();
  if (currentMillis - timer1 > 3000) //считывание датчиков каждые 3 сек
  {
    readSensors();
    checkSensors();
    timer1 = currentMillis;
  }
  wdt_reset();
}


