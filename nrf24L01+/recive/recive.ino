#include <SPI.h>
#include "RF24.h"

//Контакты от радиомодуля NRF24L01 подключаем к пинамнам -> Arduino (nano)
//SCK -> 13
//MISO -> 12
//MOSI -> 11
//CSN -> 10
//CE -> 9
#define ch_len 32

RF24 radio(9, 10);

//пины куда подключены светодиоды или другая слаботочка

char comand[8];
char datahg[6], dataha[6], datata[6], Y[5], M[3], D[3], H[3], Mn[3];
char data[ch_len];

const uint64_t pipe = 0xF0F0F0F000LL;// адреса каналов приема и передачи

void setup() {
  Serial.begin(115200);

  radio.begin();
  delay(2);
  radio.setDataRate(RF24_250KBPS); // Скорость передачи
  radio.setChannel(0x66); // Номер канала от 0 до 127
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15); // Кол-во попыток и время между попытками
  radio.openReadingPipe(1, pipe); // Открываем один из 6-ти каналов приема
  radio.startListening(); // Начинаем слушать эфир
  Serial.print("start recive");

}

void loop() {
  if (radio.available()) {

    radio.read(data, sizeof(data)); //принимаем пакет с Arduino Mega
    Serial.println(data);

    sscanf(data, "%[^',']", &comand);
    if (String(comand) == "sndpar") {
      sscanf(data, "%[^','],%[^','],%[^','],%s", &comand, &datahg, &dataha, &datata);
     /* Serial.println("-----------------------");
      Serial.print("comand ");
      Serial.println(comand);
      Serial.print("datahg ");
      Serial.println(datahg);
      Serial.print("dataha ");
      Serial.println(dataha);
      Serial.print("datata ");
      Serial.println(datata);
      Serial.println("-----------------------");
      */
    }
    else if (String(comand) == "sndtime") {
      sscanf(data, "%[^','],%[^','],%[^','],%[^','],%[^','],%s", &comand, &Y, &M, &D, &H, &Mn);
      //atoi(Y)); char* to int(byte)
      Serial.println("-----------------------");
      Serial.print("comand ");
      Serial.println(comand);
      Serial.print(atoi(Y));
      Serial.print("-");
      Serial.print(atoi(M));
      Serial.print("-");
      Serial.print(atoi(D));
      Serial.print(" ");
      Serial.print(atoi(H));
      Serial.print(":");
      Serial.println(atoi(Mn));
      Serial.println("-----------------------");
    }

    radio.stopListening();
    radio.openWritingPipe(pipe);
    //const char *buffer = "";
    //String buf = "";
    //buf = "ok";
    //buffer = buf.c_str();
    //radio.flush_tx();
    radio.write("ok", ch_len);
    radio.startListening();
    //Serial.print("data ");


  }
}
