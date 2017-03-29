#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266SSDP.h>
#include <aREST.h>
#include <IRremoteESP8266.h>
#include <FS.h>


// Web интерфейс для устройства
ESP8266WebServer HTTP(80);
// aREST и сервер для него
WiFiServer SERVERaREST(8080);
// Определяем aREST
aREST rest = aREST();
// Определяем ИК передачик на 14 ноге
IRsend irsend(14);
// Для файловой системы
File fsUploadFile;


// Подключаем реле к ногам:
int rele1 = 13; //Pin for reley1
int rele2 = 12; //Pin for reley2
int rele3 = 16; //Pin for reley3

void loop() {
  // put your main code here, to run repeatedly:
  HTTP.handleClient();
  delay(1);
  // Handle REST calls
  WiFiClient client = SERVERaREST.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);

}





