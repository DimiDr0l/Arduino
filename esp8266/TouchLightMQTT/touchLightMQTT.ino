#include "ESP8266WiFi.h"
//#include "ESP8266mDNS.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include "PubSubClient.h"
#include "EEPROM.h"

ESP8266WebServer server(80);

#define version "2.4.1"
#define pinLed1 4 //pwm Led1
#define pinLed2 5 //pwm Led2
#define pinOut 2 //out pin
#define pinSCS 15 //SCS  10
#define pinSCK 14 //SCLK 13
#define pinSDI 13 //MOSI 11 (Master Out Slave In) - линия для передачи данных от ведущего устройства (Master) к ведомым (Slave)
#define pinSDO 12 //MISO 12 (Master In Slave Out) - линия для передачи данных от ведомого устройства (Slave) к ведущему (Master)
byte delayDimmer = 2, key = 0, m = 0;
int16_t pwmLed1 = 500, pwmLed2 = 500, pwmlastLed1 = 500, pwmlastLed2 = 500, retries = 0;
const int16_t maxrange = 1023; //max range PWM (max 1023)
boolean onStateAll = false, stateM = false, stateAP = false, stateOTA = false, onState = false;
unsigned long timer1 = 0, timerwificonnect = 0, currentMillis;

const char* apssid = "IoT SSID";
const char* appass = "12345678";

String st;
String content;
String essid = "";
String epass = "";
IPAddress eipmqt;
String eportmqt = "";
String eloginmqt = "";
String epassmqt = "";
int16_t statusCode;

WiFiClient wclient;
PubSubClient client(wclient);

String prefix   = "/IoTmanager";     // global prefix for all topics - must be some as mobile device
String deviceID = "dev01-bedroom";   // thing ID - unique device id in our project

const String stat1 = "{\"status\":\"1\"}";
const String stat0 = "{\"status\":\"0\"}";

const int nWidgets = 3;
String stat        [nWidgets];
String sTopic      [nWidgets];
String color       [nWidgets];
String style       [nWidgets];
String badge       [nWidgets];
String widget      [nWidgets];
String descr       [nWidgets];
String page        [nWidgets];
String thing_config[nWidgets];
String id          [nWidgets];

String setStatus ( int s ) {
  String stat = "{\"status\":\"" + String(s) + "\"}";
  return stat;
}

void initVar() {
  id    [0] = "0";
  page  [0] = "Bedroom";
  descr [0] = "Свет в комнате";
  widget[0] = "toggle";
  sTopic[0]   = prefix + "/" + deviceID + "/light0";
  color[0]   = "\"color\":\"green\"";            // black, blue, green, orange, red, white, yellow (off - grey)
  stat[0] = stat0;                      // defaultVal status Off

  id    [1] = "1";
  page  [1] = "Bedroom";
  descr [1] = "Zone1";
  widget[1] = "range";
  sTopic[1]   = prefix + "/" + deviceID + "/dim-light1";
  style[1]   = "\"style\":\"range-calm\"";          // see http://ionicframework.com/docs/components/#colors
  badge[1]   = "\"badge\":\"badge-assertive\"";       // see http://ionicframework.com/docs/components/#colors
  stat[1] = setStatus(pwmLed1);               // defaultVal status light1

  id    [2] = "2";
  page  [2] = "Bedroom";
  descr [2] = "Zone2";
  widget[2] = "range";
  sTopic[2]   = prefix + "/" + deviceID + "/dim-light0";
  style[2]   = "\"style\":\"range-calm\"";                 // see http://ionicframework.com/docs/components/#colors
  badge[2]   = "\"badge\":\"badge-assertive\"";            // see http://ionicframework.com/docs/components/#colors
  stat[2] = setStatus(pwmLed2);             // defaultVal status light2

  thing_config[0] = "{\"id\":\"" + id[0] + "\",\"page\":\"" + page[0] + "\",\"descr\":\"" + descr[0] + "\",\"widget\":\"" + widget[0] + "\",\"topic\":\"" + sTopic[0] + "\"," + color[0] + "}"; // GPIO switched On/Off by mobile widget toggle
  thing_config[1] = "{\"id\":\"" + id[1] + "\",\"page\":\"" + page[1] + "\",\"descr\":\"" + descr[1] + "\",\"widget\":\"" + widget[1] + "\",\"topic\":\"" + sTopic[1] + "\"," + style[1] + "," + badge[1] + "}";
  thing_config[2] = "{\"id\":\"" + id[2] + "\",\"page\":\"" + page[2] + "\",\"descr\":\"" + descr[2] + "\",\"widget\":\"" + widget[2] + "\",\"topic\":\"" + sTopic[2] + "\"," + style[2] + "," + badge[2] + "}";

}
// send confirmation
void pubStatus(String t, String payload) {
  if (client.publish((t + "/status").c_str(), payload.c_str())) {
    Serial.println("Publish new status for " + t + ", value: " + payload);
  } else {
    Serial.println("Publish new status for " + t + " FAIL!");
  }
}
void pubConfig() {
  client.publish((prefix + "/" + deviceID + "/config/Version").c_str(), version);
  for (int i = 0; i < nWidgets; i = i + 1) {
    if (client.publish((prefix + "/" + deviceID + "/config").c_str(), thing_config[i].c_str())) {
      Serial.println("Publish config: Success (" + thing_config[i] + ")");
    } else {
      Serial.println("Publish config FAIL! ("    + thing_config[i] + ")");
    }
    delay(50);
  }
  for (int i = 0; i < nWidgets; i = i + 1) {
    pubStatus(sTopic[i], stat[i]);
    delay(50);
  }
}

void callbackmqt(char* topic, byte* payload, unsigned int length) {
  String topics = topic;
  String p = "";
  for (int16_t i = 0; i < length; i++) {
    p += char(payload[i]);
  }
  Serial.print("Get data from subscribed topic ");
  Serial.print(topics);
  Serial.print(" => ");
  Serial.println(p);

  if (topics == sTopic[0] + "/control") {
    if (p == "0" && onStateAll) {
      AllOff();
    } else if (p == "1" && !onStateAll) {
      AllOn();
    }

  } else if (topics == sTopic[0] + "/control/OTA") {
    String statap = "{\"status\":";
    statap += "\"start OTA\"}";
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    pubStatus(sTopic[0], statap);
    pubStatus(sTopic[0], ipStr);
    stateOTA = true;
    setupAP();

  } else if (topics == sTopic[0] + "/control/reboot") {
    String statap = "{\"status\":";
    statap += "\"reboot esp\"}";
    pubStatus(sTopic[0], statap);
    restart();

  } else if (topics == sTopic[1] + "/control") {
    if (onStateAll) {
      pwmLed1 = constrain(p.toInt(), 0, maxrange);
      analogWrite(pinLed1, pwmLed1);
      stat[1] = setStatus(pwmLed1);
    }
    pubStatus(sTopic[1], stat[1]);

  } else if (topics == sTopic[2] + "/control" ) {
    if (onStateAll) {
      pwmLed2 = constrain(p.toInt(), 0, maxrange);
      analogWrite(pinLed2, pwmLed2);
      stat[2] = setStatus(pwmLed2);
    }
    pubStatus(sTopic[2], stat[2]);
  }
  if (topics == prefix) {
    if (p == "HELLO") {
      pubConfig();
    }
  }
}

void setup() {
  //Serial.begin(115200);
  //ESP.wdtDisable();
  //ESP.wdtFeed();
  //ESP.wdtEnable(WDTO_8S);
  pinMode (pinLed1, OUTPUT);
  pinMode (pinLed2, OUTPUT);
  pinMode (pinOut, INPUT);
  pinMode (pinSCS, OUTPUT);
  pinMode (pinSCK, OUTPUT);
  pinMode (pinSDI, OUTPUT);
  pinMode (pinSDO, INPUT);
  digitalWrite(pinSDI, HIGH);
  digitalWrite(pinLed1, LOW);
  digitalWrite(pinLed2, LOW);

  analogWriteFreq(500); //задание частоты pwm Гц
  analogWriteRange(maxrange); //диапазон pwm
  set_Subtle_SPI(25); //установка чувствительности
  initVar(); //инициализация переменных

  WiFi.mode(WIFI_STA);
  WiFi.hostname(deviceID);
  EEPROM.begin(512);
  delay(10);
  // read eeprom
  EEPROM.commit();
  for (int16_t i = 0; i < 32; ++i) essid += char(EEPROM.read(i));
  for (int16_t i = 32; i < 64; ++i) epass += char(EEPROM.read(i));
  for (int16_t i = 64; i < 68; ++i) eipmqt[i - 64] = EEPROM.read(i); //чтение 4 ячеек памяти для ип адреса
  for (int16_t i = 96; i < 101; ++i) eportmqt += char(EEPROM.read(i));
  for (int16_t i = 101; i < 133; ++i) eloginmqt += char(EEPROM.read(i));
  for (int16_t i = 133; i < 165; ++i) epassmqt += char(EEPROM.read(i));

  client.setServer(eipmqt, eportmqt.toInt());
  client.setCallback(callbackmqt);

  Serial.println("");
  Serial.print("real chip size: ");
  Serial.print(ESP.getFlashChipRealSize());
  Serial.println(" Bytes");
  Serial.print("SSID: ");
  Serial.println(essid);
  Serial.print("PASS: ");
  Serial.println(epass);
  Serial.println (ESP.getFreeHeap());

}

void loop() {
  dRead();
  reconnect();
  yield();
  //ESP.wdtFeed();
}

void reconnect() {
  if (millis() >= timerwificonnect && !stateOTA) {
    timerwificonnect = millis() + 3000;
    if (WiFi.status() != WL_CONNECTED) {
      if (essid.length() > 0 && eipmqt[0] > 0) {
        WiFi.begin(essid.c_str(), epass.c_str());
        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
          Serial.print(".");
          if (!stateAP) {
            retries ++;
            if (retries > 10) setupAP();
            //retries = 0;
          }
          return;
        } else retries = 0;
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        WiFi.printDiag(Serial);
      } else if (!stateAP) setupAP();
    } else if (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      //Attempt to connect
      if (client.connect(deviceID.c_str(), eloginmqt.c_str(), epassmqt.c_str())) {
        if (stateAP) {
          WiFi.mode(WIFI_STA);
          stateAP = false;
        }
        Serial.println("connected");
        retries = 0;
        pubConfig();
        client.subscribe(prefix.c_str());           // for receiving HELLO messages
        client.subscribe((prefix + "/+/+/control/#").c_str());  // for receiving GPIO control messages
        Serial.println("Subscribe: Success");
      } else {
        Serial.print("Connect to MQTT server: FAIL ");
        Serial.println(client.state());
        Serial.println("Try again...");
        if (!stateAP) {
          retries ++;
          if (retries > 30) setupAP();
        }
      }
    }
  }
  client.loop();
  server.handleClient();
}

void setupAP() {
  WiFi.mode(WIFI_AP_STA);
  createWebServer();
  server.begin();
  stateAP = true;
  //WiFi.disconnect();
  delay(100);
  byte n = WiFi.scanNetworks();
  st = "<ol>";
  for (byte i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  WiFi.softAP(apssid, appass); //-------------------------------------------------------
  Serial.print("Start AP: \"");
  Serial.print(apssid);
  Serial.println("\"");
  Serial.print("AP Password: \"");
  Serial.print(appass);
  Serial.println("\"");

}

void createWebServer() {
  server.on("/", []() {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html><title>Setting ESP LED Lamp</title><center>Hello from ESP8266 at ";
    content += ipStr;
    content += "<p>";
    content += "Free memory: ";
    content += ESP.getFreeHeap();
    content += "<p>";
    content += st;
    content += "</p><form action='setting'><label>SSID: </label><input name='ssid' length=32 value='";
    content += essid.c_str() + String("'>");
    content += "<label>Pass: </label><input name = 'pass' length = 32 value ='";
    content += epass.c_str() + String("'>");
    content += "<p><label>IP MQTT: </label><input name='ipmqt0' length=3 size='3' value='";
    content += String(eipmqt[0]) + String("'>");
    content += "<input name='ipmqt1' length=3 size='3' value='";
    content += String(eipmqt[1]) + String("'>");
    content += "<input name='ipmqt2' length=3 size='3' value='";
    content += String(eipmqt[2]) + String("'>");
    content += "<input name='ipmqt3' length=3 size='3' value='";
    content += String(eipmqt[3]) + String("'></p>");

    content += "<p><label>Port MQTT: </label><input name='portmqt' length=5 value='";
    content += eportmqt.c_str() + String("'></p>");
    content += "<p><label>Login MQTT: </label><input name='loginmqt' length=32 value='";
    content += eloginmqt.c_str() + String("'></p>");
    content += "<p><label>Password MQTT: </label><input name='passmqt' length=32 value='";
    content += epassmqt.c_str() + String("'></p>");
    content += "<input type='submit'></form>";
    content += "<form action='reboot'><p><button type='submit'>Reboot ESP</button></p></form>";
    content += "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>"; //update esp firmware
    content += "</center></html>";
    server.send(200, "text/html", content);
  });

  server.on("/setting", []() {
    String qssid = server.arg("ssid");
    String qpass = server.arg("pass");
    IPAddress qipmqt;
    qipmqt[0] = (server.arg("ipmqt0")).toInt();
    qipmqt[1] = (server.arg("ipmqt1")).toInt();
    qipmqt[2] = (server.arg("ipmqt2")).toInt();
    qipmqt[3] = (server.arg("ipmqt3")).toInt();
    String qportmqt = server.arg("portmqt");
    String qloginmqt = server.arg("loginmqt");
    String qpassmqt = server.arg("passmqt");

    if (qssid.length() > 0 && qpass.length() > 0 && qipmqt[0] > 0 && qportmqt.length() > 0) {
      for (int16_t i = 0; i < 165; ++i) EEPROM.write(i, 0); //стирание памяти SSID и pass
      for (int16_t i = 0; i < qssid.length(); ++i) EEPROM.write(i, qssid[i]); //запись SSID
      for (int16_t i = 0; i < qpass.length(); ++i) EEPROM.write(32 + i, qpass[i]); //запись пароля wifi
      //for (int16_t i = 0; i < qipmqt.length(); ++i) EEPROM.write(64 + i, qipmqt[i]); //запись ip MQT
      for (int16_t i = 0; i < 4; ++i) EEPROM.write(64 + i, qipmqt[i]); //запись ip MQT

      for (int16_t i = 0; i < qportmqt.length(); ++i) EEPROM.write(96 + i, qportmqt[i]); //запись порта MQT
      for (int16_t i = 0; i < qloginmqt.length(); ++i) EEPROM.write(101 + i, qloginmqt[i]); //запись логина MQT
      for (int16_t i = 0; i < qpassmqt.length(); ++i) EEPROM.write(133 + i, qpassmqt[i]); //запись пароля MQT
      EEPROM.commit();
      content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
      statusCode = 200;
      restart();
    } else {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
    }
    server.send(statusCode, "application/json", content);

  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
  });
  server.on("/cleareeprom", []() {
    content = "<!DOCTYPE HTML>\r\n<html>";
    content += "<p>Clearing the EEPROM</p></html>";
    server.send(200, "text/html", content);
    for (int16_t i = 0; i < 165; ++i) EEPROM.write(i, 0); //стирание настроек
    EEPROM.commit();
  });
  server.on("/reboot", []() {
    content = "<!DOCTYPE HTML>\r\n<html>";
    content += "<p>Reboot...</p></html>";
    server.send(200, "text/html", content);
    restart();
  });
}


void dRead() {
  if (!digitalRead(pinOut)) {
    //currentMillis = millis();
    key = readKey();
    if (!onStateAll && (millis() >= timer1 + 70) && !onState) { //100мс на удержание вкл
      onState = true;
      AllOn();
      //return;
    }
    else if (onStateAll && !onState) {
      if ((key == 134) || (key >= 1 && key <= 126)) {  //кнопка выкл
        onState = true;
        AllOff();
        //return;
      }
      else if (key == 129 && !stateM) stateM = true; //кнопка меню
      else if (key == 130) { //кнопка вверх
        switch (m) {
          case 0:
            if (pwmLed1 < maxrange) {
              pwmLed1++;
              pwmLed2 = pwmLed1;
              analogWrite(pinLed1, pwmLed1);
              analogWrite(pinLed2, pwmLed2);
            }
            break;
          case 1:
            if (pwmLed1 < maxrange) {
              pwmLed1++;
              analogWrite(pinLed1, pwmLed1);
            }
            break;
          case 2:
            if (pwmLed2 < maxrange) {
              pwmLed2++;
              analogWrite(pinLed2, pwmLed2);
            }
            break;
        }
        delay(delayDimmer);
      }
      else if (key == 128) { //кнопка вниз
        switch (m) {
          case 0:
            if (pwmLed1 > 1) {
              pwmLed1--;
              pwmLed2 = pwmLed1;
              analogWrite(pinLed1, pwmLed1);
              analogWrite(pinLed2, pwmLed2);
            }
            break;
          case 1:
            if (pwmLed1 > 0) {
              pwmLed1--;
              analogWrite(pinLed1, pwmLed1);
            }
            break;
          case 2:
            if (pwmLed2 > 0) {
              pwmLed2--;
              analogWrite(pinLed2, pwmLed2);
            }
            break;
        }
        delay(delayDimmer);
      }
      else if (key == 135) { //полная яркость
        switch (m) {
          case 0:
            pwmLed1 = maxrange;
            pwmLed2 = pwmLed1;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, pwmLed2);
            break;
          case 1:
            pwmLed1 = maxrange;
            analogWrite(pinLed1, pwmLed1);
            break;
          case 2:
            pwmLed2 = maxrange;
            analogWrite(pinLed2, pwmLed2);
            break;
        }
      }
      else if (key == 132) { //1/2 яркость
        switch (m) {
          case 0:
            pwmLed1 = maxrange / 2;
            pwmLed2 = pwmLed1;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, pwmLed2);
            break;
          case 1:
            pwmLed1 = maxrange / 2;
            analogWrite(pinLed1, pwmLed1);
            break;
          case 2:
            pwmLed2 = maxrange / 2;
            analogWrite(pinLed2, pwmLed2);
            break;
        }
      }
      else if (key == 133) { //1/4 яркость
        switch (m) {
          case 0:
            pwmLed1 = maxrange / 4;
            pwmLed2 = pwmLed1;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, pwmLed2);
            break;
          case 1:
            pwmLed1 = maxrange / 4;
            analogWrite(pinLed1, pwmLed1);
            break;
          case 2:
            pwmLed2 = maxrange / 4;
            analogWrite(pinLed2, pwmLed2);
            break;
        }
      }
      else if (key == 131) { //min яркости
        switch (m) {
          case 0:
            pwmLed1 = 1;
            pwmLed2 = pwmLed1;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, pwmLed2);
            break;
          case 1:
            pwmLed1 = 1;
            analogWrite(pinLed1, pwmLed1);
            break;
          case 2:
            pwmLed2 = 1;
            analogWrite(pinLed2, pwmLed2);
            break;
        }
      }

      /* else if (key >= 1 && key <= 126) {
        byte stateLight = map(key, 1, 126, 0, maxrange);
        switch (m) {
        case 0:
        pwmLed1 = stateLight;
        pwmLed2 = stateLight;
        analogWrite(pinLed1, pwmLed1);
        analogWrite(pinLed2, pwmLed2);
        break;
        case 1:
        pwmLed1 = stateLight;
        analogWrite(pinLed1, pwmLed1);
        break;
        case 2:
        pwmLed2 = stateLight;
        analogWrite(pinLed2, pwmLed2);
        break;
        }
        }
      */
    }

  }  //конец if digitalRead
  else {
    if (onStateAll) {
      if (pwmlastLed1 != pwmLed1) {
        stat[1] = setStatus(pwmLed1);
        pubStatus(sTopic[1], stat[1]);
      }
      if (pwmlastLed2 != pwmLed2) {
        stat[2] = setStatus(pwmLed2);
        pubStatus(sTopic[2], stat[2]);
      }
      pwmlastLed1 = pwmLed1;
      pwmlastLed2 = pwmLed2;

      if (stateM) {
        stateM = false;
        m++;
        if (m > 3) m = 0;
        switch (m) {
          case 0:
            pwmLed2 = pwmLed1;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, pwmLed2);
            break;
          case 1:
            //pwmLed2 = 0;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, 0);
            break;
          case 2:
            //pwmLed1 = 0;
            analogWrite(pinLed1, 0);
            analogWrite(pinLed2, pwmLed2);
            break;
          case 3:
            if (pwmLed1 = pwmLed2) m = 0;
            analogWrite(pinLed1, pwmLed1);
            analogWrite(pinLed2, pwmLed2);
            break;
        }
      }
    }
    if (onState) onState = false;
    timer1 = millis();
  }
}

byte readKey() {
  digitalWrite(pinSCS, LOW);
  byte incoming = shiftIn(pinSDO, pinSCK, MSBFIRST); //и слушаем что нам ответят
  digitalWrite(pinSCS, HIGH);
  delay (5); // задержка 5MS
  return (incoming);
}

void set_Subtle_SPI(byte value) {
  digitalWrite(pinSCS, LOW);
  shiftOut(pinSDI, pinSCK, MSBFIRST, value); //послали адрес
  digitalWrite(pinSCS, HIGH);
  delay (5); // задержка 5MS
}

void AllOn() {
  analogWrite (pinLed1, pwmLed1);
  analogWrite (pinLed2, pwmLed2);
  stat[0] = stat1;
  pubStatus(sTopic[0], stat[0]);
  onStateAll = true;
}

void AllOff() {
  analogWrite (pinLed1, 0);
  analogWrite (pinLed2, 0);
  stat[0] = stat0;
  pubStatus(sTopic[0], stat[0]);
  onStateAll = false;
}

void restart() {
  Serial.println("Will reset and try again...");
  ESP.reset();
}

