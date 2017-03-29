#include "avr/wdt.h"
#include "SPI.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#include "CyberLib.h"
#include "RTClib.h"
#include "RF24.h"

RF24 radio(8, 9);
const uint64_t pipe = 0xF0F0F0F000LL;
RTC_DS1307 RTC;
LiquidCrystal_I2C lcd(0x27,16,2);  // дисплей подключить к А4 А5
DHT dht(7, DHT22);    // датчик темп и влажности воздуха подключается к D7
#define pullPin 8   //нога дергания датчика влажности
#define humPin  A7    // нога датчика влажности земли
#define waterPin 2    //нога насоса
#define airPin 3    //нога вытяжки
#define heatPin 4   //нога обогревателя
#define lightPin 6    //нога освещения PWM
#define ledPin 5    //светодиод работы насоса
#define buttonUp      A3  //кнопки
#define buttonDown    A2
#define buttonLeft    A1
#define buttonRight   A0
#define ch_len 32 //буфер 32 байта

unsigned long currentMillis;
long timer1 = -3000; //задержка считывания показаний датчиков температуры 3 сек
byte timer2 = 10; //таймер для считывания влажности земли
long previousMillis = 0; //счетчик прошедшего времени для AutoMainScreen
long interval = 10000; //задержка автовозврата к MainScreen 10 сек
long debounceDelay;
boolean timerbt = true, b1, b2, b3, b4, val1 = true, val2 = true, val3 = true, val4 = true; //кнопки

float hum_atmosphere;
float temp_atmosphere;
int hum_ground;
byte m = 1; //переменная для экранов меню
byte t1; //переменная для задания температуры воздуха
byte h1; //переменная для задания влажности воздуха
byte h2; //переменная для задания влажности земли
//byte i = 10;
//int j = 28790;
int Year = 2017;
byte Mounth = 3, Day = 1, Hour = 19, Minute = 45, Second = 0; //установка даты

char comand[8], datahg[6], dataha[6], datata[6], Y[5], M[3], D[3], H[3], Mn[3];
char data[ch_len];

void setup() {
  Serial.begin(115200);
  pinMode (buttonUp, INPUT_PULLUP); //нога кнопки для смены экрана меню
  pinMode (buttonDown, INPUT_PULLUP);
  pinMode (buttonLeft, INPUT_PULLUP);
  pinMode (buttonRight, INPUT_PULLUP);

  pinMode (humPin, INPUT);  //нога считывания влажности земли
  pinMode(pullPin, OUTPUT);   //нога дергания датчика влажности
  pinMode(heatPin, OUTPUT); //нога обогревателя
  pinMode(waterPin, OUTPUT);  //нога насоса
  pinMode(ledPin, OUTPUT);  //светодиод работы насоса
  pinMode(lightPin, OUTPUT); //освещение
  pinMode(airPin, OUTPUT);  //вытяжка
  t1 = ReadEEPROM_Byte(0);
  h1 = ReadEEPROM_Byte(1);
  h2 = ReadEEPROM_Byte(2);
  //  radio.begin(); //инициализация NRF24L01+
  //  delay(2);
  //  radio.setChannel(0x66); // Номер канала от 0 до 127
  //  radio.setPALevel(RF24_PA_MAX);
  //  radio.setDataRate(RF24_250KBPS);
  //  radio.setRetries(15, 15); // Кол-во попыток и время между попытками
  //  radio.openReadingPipe(1, pipe);
  //  radio.startListening();
  //radio.powerUp();
  dht.begin(); //датчик темп и влажности воздуха подключается к D7
  lcd.init(); //дисплей

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(Year, Mounth, Day, Hour, Minute, Second)); //установка даты и время
  }

  lcd.setCursor(4, 0);
  lcd.print("START");
  Serial.println("START");
  delay_ms (1000);//Задержка приветствия
  lcd.clear();
  //wdt_enable (WDTO_8S);   //вкл авторесета через 8 сек.
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
  case 5:                 //переменная m=5
    DateTime now = RTC.now(); //отображение времени
    lcd.setCursor (3, 0);
    lcd.print (now.day(), DEC);
    lcd.print ('/');
    lcd.print (now.month(), DEC);
    lcd.print ('/');
    lcd.print (now.year(), DEC);
    lcd.setCursor (4, 1);
    lcd.print (now.hour(), DEC);
    lcd.print (':');
    lcd.print (now.minute(), DEC);
    lcd.print (':');
    lcd.print (now.second(), DEC);
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
    delay (50);
    hum_ground = map(hum_ground, 100, 1000, 100, 0);
    digitalWrite(pullPin, LOW);
    digitalWrite(ledPin, LOW); //отключение индикации считывания с датчиков
    if (hum_ground < h2) {
      pump_on();
      Serial.println("poliv BKJI");
    } else {
      pump_off();
      Serial.println("poliv BbIK");
    }
    timer2 = 0;
  }

  hum_atmosphere = dht.readHumidity();
  temp_atmosphere = dht.readTemperature();
  if (temp_atmosphere == 0.00 || hum_atmosphere == NAN) { //проверка датчика DHT22
    digitalWrite(heatPin, LOW); //выключение обогревателя
    digitalWrite(airPin, LOW); //выключение вентилятора
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("DHT ERROR!!!");
    digitalWrite(ledPin, HIGH);
    lcd.setBacklight(HIGH);
    delay_ms (500);
    digitalWrite(ledPin, LOW);
    lcd.setBacklight(LOW);
    delay_ms (500);
    lcd.clear();
    wdt_reset();
    readSensors();
  }
}

void checkSensors() {
  if (temp_atmosphere < t1 - 0.5) {
    digitalWrite(airPin, HIGH); //включение вентилятора
    digitalWrite(heatPin, HIGH); //включение обогревателя
    Serial.println("obogrev BKJI");
  }
  else if (temp_atmosphere >= t1 + 1 || hum_atmosphere >= h1) { //если темепература на 2с или влажность воздуха выше h1
    digitalWrite(heatPin, LOW); //выключение обогревателя
    digitalWrite(airPin, HIGH); //включение вентилятора
    Serial.println("obduv BKJI");
  }
  else if ((temp_atmosphere <= t1 + 2 && temp_atmosphere >= t1) && hum_atmosphere <= h1) {
    digitalWrite(heatPin, LOW); //выключение обогревателя
    digitalWrite(airPin, LOW); //выключение вентилятора
    Serial.println("BCE BbIK");
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
  b1 = digitalRead (buttonUp);
  b2 = digitalRead (buttonDown);
  b3 = digitalRead (buttonLeft);
  b4 = digitalRead (buttonRight);
  if ((!b1 || !b2 || !b3 || !b4) && timerbt) {
    debounceDelay = millis();
    timerbt = false;
  }
  if (millis() - debounceDelay > 50) {
    if (!b1 && val1) {
      val1 = false;
    }
    else if (b1 && !val1) {
      val1 = true;
      previousMillis = currentMillis;
      if (m == 2) { //если находимся на экране с переменной t1
        t1++;      //то при нажатии кнопки + увеличиваем переменную t1 на единицу
        delay_ms (50);
        lcd.clear();
        WriteEEPROM_Byte(0, t1);
        if (t1 > 40) t1 = 20; //устанавливаем придел изменения переменной = 40
      }
      //UP для влажности H1
      if (m == 3) {
        h1 = h1 + 5;
        delay_ms (50);
        lcd.clear();
        if (h1 > 100) h1 = 30;
        WriteEEPROM_Byte(1, h1);
      }
      //UP для влажности H2
      if (m == 4) {
        h2 = h2 + 5;
        delay_ms (50);
        lcd.clear();
        if (h2 > 100) h2 = 40;
        WriteEEPROM_Byte(2, h2);
      }
    }
    else if (!b2 && val2) {
      val2 = false;
    }
    else if (b2 && !val2) {
      val2 = true;
      previousMillis = currentMillis;
      if (m == 2) { //если находимся на экране с переменной t1
        t1--;      //то при нажатии кнопки - уменьшаеем переменную t1 на единицу
        delay_ms (50);
        lcd.clear();
        if (t1 < 20) t1 = 40; //устанавливаем придел изменения переменной = 20
        WriteEEPROM_Byte(0, t1);
      }
      //down для H1
      if (m == 3) {
        h1 = h1 - 5;
        delay_ms (50);
        lcd.clear();
        if (h1 < 30) h1 = 100;
        WriteEEPROM_Byte(1, h1);
      }
      //down для H2
      if (m == 4) {
        h2 = h2 - 5;
        delay_ms (50);
        lcd.clear();
        if (h2 < 40) h2 = 100;
        WriteEEPROM_Byte(2, h2);
      }

    }
    else  if (!b3 && val3) {
      val3 = false;
    }
    else if (b3 && !val3) {
      val3 = true;
      m--;
      previousMillis = currentMillis;
      delay_ms (50);
      lcd.clear();
      if (m < 1) m = 5;
    }
    else  if (!b4 && val4) {
      val4 = false;
    }
    else if (b4 && !val4) {
      val4 = true;
      m++;//увеличиваем переменную уровня меню
      previousMillis = currentMillis; //если кнопка была нажата сбросить счетчик автовозврата к главному экрану
      delay_ms (50);
      lcd.clear();
      if (m > 5) m = 1; //если уровень больше 4
    }
    timerbt = true;
  }
}

void loop() {
  currentMillis = millis();
  ReadButtons();
  menu();
  if (currentMillis - timer1 > 3000) //проверка таймера считывания датчика температуры каждые 3 сек
  {
    readSensors();
    checkSensors();
    timer1 = currentMillis;
    /*   if (i > 2) {
  SendData ("sndpar");
  i = 0;
  }
  if (j > 4) {
  SendData ("sndtime");
  j = 0;
  }
  i++;
  j++;
*/
  }
  //wdt_reset();
  //AvailableData();
}

void AvailableData() {
  if (radio.available()) {
    radio.read(data, sizeof(data));
    sscanf(data, "%[^',']", &comand);
    if (String(comand) == "settime") {
      sscanf(data, "%[^','],%[^','],%[^','],%[^','],%[^','],%s", &comand, &Y, &M, &D, &H, &Mn);
      Year = atoi(Y);
      Mounth = atoi(M);
      Day = atoi(D);
      Hour = atoi(H);
      Minute = atoi(Mn);

      if (! RTC.isrunning()) {
        Serial.println("RTC is NOT running!");
        RTC.adjust(DateTime(Year, Mounth, Day, Hour, Minute, Second)); //установка даты и время
      }

    }
  }
}

void SendData(String cmd) {
  radio.stopListening();
  radio.openWritingPipe(pipe);
  const char *buffer = "";
  String buf = "";
  if (cmd == "sndpar") {
    buf = "sndpar," + String(hum_ground) + "," + String(hum_atmosphere) + "," + String(temp_atmosphere);
    buffer = buf.c_str();
    radio.write(buffer, ch_len);
    //sscanf(buffer, "%[^','],%[^','],%[^','],%s", &comand, &datahg, &dataha, &datata);
  }
  else if (cmd == "sndtime") {
    DateTime now = RTC.now(); //отображение времени
    buf = "sndtime," + String(now.year()) + "," + String(now.month()) + "," + String(now.day()) + "," + String(now.hour()) + "," + String(now.minute());
    buffer = buf.c_str();
    radio.write(buffer, ch_len);

    //Serial.print("Date Time: ");
    //Serial.println(buffer);
    //sscanf(buffer, "%[^','],%[^','],%[^','],%[^','],%[^','],%s", &comand, &Y, &M, &D, &H, &Mn);
  }
  radio.startListening();
}


