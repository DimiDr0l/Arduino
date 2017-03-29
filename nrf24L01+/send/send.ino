#include <SPI.h>
#include "RF24.h"

#define ch_len 30
//Контакты от радиомодуля NRF24L01 подключаем к пинамнам -> Arduino (Mega)
//SCK -> 52
//MISO -> 50
//MOSI -> 51
//CSN -> 53
//CE -> 9

RF24 radio(9, 10);
const uint64_t pipe = 0xF0F0F0F000LL;// адреса каналов приема и передачи

long timer = 0;
byte ii = 0;
int ji = 0;
char data[ch_len];

void setup()
{
  Serial.begin(115200);      // для дебага
  radio.begin();
  delay(2);
  radio.setDataRate(RF24_250KBPS); // Скорость передачи
  radio.setChannel(0x66); // Номер канала от 0 до 127
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15); // Кол-во попыток и время между попытками
  radio.openReadingPipe(1, pipe); // Открываем один из 6-ти каналов приема
  radio.startListening(); // Начинаем слушать эфир
  Serial.println("start");
}


void loop()
{
  if (millis() - timer > 1500) {
    timer = millis();
    if (ii >= 2) {
      SendData ("sndpar");
      ii = 0;
    }
    else if (ji >= 7) {
      SendData ("sndtime");
      ji = 0;
    }
    ii++;
    ji++;
  }
  if (radio.available()) {
    radio.read(data, sizeof(data));
    Serial.print("recive ");
    Serial.println(data);
  }
}

void SendData(String cmd) {
  radio.stopListening();
  radio.openWritingPipe(pipe);
  const char *buffer = "";
  String buf = "";
  if (cmd == "sndpar") {
    buf = "sndpar," + String(73.4) + "," + String(50.0) + "," + String(24.6);
    buffer = buf.c_str();
    radio.write(buffer, ch_len);
    Serial.print("sndpar: ");
    Serial.println(buffer);
  }
  else if (cmd == "sndtime") {
    buf = "sndtime," + String(2016) + "," + String(02) + "," + String(20) + "," + String(12) + "," + String(57);
    buffer = buf.c_str();
    radio.write(buffer, ch_len);

    Serial.print("Date Time: ");
    Serial.println(buffer);
  }
  radio.startListening();
  //delay(40);
}
