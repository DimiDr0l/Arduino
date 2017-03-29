#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

const char* apssid = "Io SSID";
const char* passphrase = "12345678";
String st;
String content;
String essid = "";
String epass = "";
IPAddress eipmqt;
String eportmqt = "";
String eloginmqt = "";
String epassmqt = "";
int statusCode;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  EEPROM.begin(512);
  delay(10);
  // read eeprom
  EEPROM.commit();
  for (int i = 0; i < 32; ++i) essid += char(EEPROM.read(i));
  for (int i = 32; i < 64; ++i) epass += char(EEPROM.read(i));
  //for (int i = 64; i < 96; ++i) eipmqt += char(EEPROM.read(i));
  for (int i = 64; i < 68; ++i) eipmqt[i - 64] = EEPROM.read(i);

  for (int i = 96; i < 101; ++i) eportmqt += char(EEPROM.read(i));
  for (int i = 101; i < 133; ++i) eloginmqt += char(EEPROM.read(i));
  for (int i = 133; i < 165; ++i) epassmqt += char(EEPROM.read(i));

  Serial.print("SSID: ");
  Serial.println(essid);
  Serial.print("PASS: ");
  Serial.println(epass);

  if (essid.length() > 1 && eipmqt[0] > 1) {
    WiFi.begin(essid.c_str(), epass.c_str());
    byte retries = 0;
    while ((WiFi.status() != WL_CONNECTED) && (retries < 20)) {
      retries++;
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      WiFi.printDiag(Serial);
    } else setupAP();
  } else setupAP();
  launchWeb();
}

void launchWeb() {
  createWebServer();
  server.begin();
}

void setupAP() {
  Serial.print("Start AP ");
  Serial.println(apssid);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
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
  delay(100);
  WiFi.softAP(apssid, passphrase, 6);
}

void createWebServer() {

  server.on("/", []() {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
    content += ipStr;
    content += "<p>";
    content += st;
    content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32 value='";
    content += essid.c_str() + String("'>");
    content += "<label>Pass: </label><input name = 'pass' length = 32 value ='";
    content += epass.c_str() + String("'>");
    content += "</P><label>IP MQT: </label><input name='ipmqt0' length=3 value='";
    content += String(eipmqt[0]) + String("'>");
    content += "<input name='ipmqt1' length=3 value='";
    content += String(eipmqt[1]) + String("'>");
    content += "<input name='ipmqt2' length=3 value='";
    content += String(eipmqt[2]) + String("'>");
    content += "<input name='ipmqt3' length=3 value='";
    content += String(eipmqt[3]) + String("'>");

    content += "<label>Port MQT: </label><input name='portmqt' length=5 value='";
    content += eportmqt.c_str() + String("'>");
    content += "<label>Login MQT: </label><input name='loginmqt' length=32 value='";
    content += eloginmqt.c_str() + String("'>");
    content += "<label>Password MQT: </label><input name='passmqt' length=32 value='";
    content += epassmqt.c_str() + String("'>");
    content += "<input type='submit'></form>";
    content += "</html>";
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
      for (int i = 0; i < 165; ++i) EEPROM.write(i, 0); //стирание памяти SSID и pass
      for (int i = 0; i < qssid.length(); ++i) EEPROM.write(i, qssid[i]); //запись SSID
      for (int i = 0; i < qpass.length(); ++i) EEPROM.write(32 + i, qpass[i]); //запись пароля wifi
      //for (int i = 0; i < qipmqt.length(); ++i) EEPROM.write(64 + i, qipmqt[i]); //запись ip MQT
      for (int i = 0; i < 4; ++i) EEPROM.write(64 + i, qipmqt[i]); //запись ip MQT

      for (int i = 0; i < qportmqt.length(); ++i) EEPROM.write(96 + i, qportmqt[i]); //запись порта MQT
      for (int i = 0; i < qloginmqt.length(); ++i) EEPROM.write(101 + i, qloginmqt[i]); //запись логина MQT
      for (int i = 0; i < qpassmqt.length(); ++i) EEPROM.write(133 + i, qpassmqt[i]); //запись пароля MQT
      EEPROM.commit();
      content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
      statusCode = 200;
    } else {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
    }
    server.send(statusCode, "application/json", content);

  });

  server.on("/cleareeprom", []() {
    content = "<!DOCTYPE HTML>\r\n<html>";
    content += "<p>Clearing the EEPROM</p></html>";
    server.send(200, "text/html", content);
    for (int i = 0; i < 165; ++i) EEPROM.write(i, 0); //стирание настроек
    EEPROM.commit();
  });
  server.on("/reboot", []() {
    content = "<!DOCTYPE HTML>\r\n<html>";
    content += "<p>Reboot...</p></html>";
    server.send(200, "text/html", content);
    ESP.reset();
  });

}

void loop() {
  server.handleClient();
}

