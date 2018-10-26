#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "4fd007e398764ce0bef5b91257511444";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "/n %10 Sky Net";
char pass[] = "1qaz2wsx3edc4";

void setup()
{
  ledcAttachPin(17,1); //назад
  ledcAttachPin(18,2); //вперёд
  ledcAttachPin(19,3); //вправо
  ledcAttachPin(21,4); //влево

  ledcSetup(1, 12000, 10);
  ledcSetup(2, 12000, 10);
  ledcSetup(3, 12000, 10);
  ledcSetup(4, 12000, 10);
  
  // Debug console
  Serial.begin(115200);

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  Blynk.begin(auth, ssid, pass, IPAddress(77,37,180,36), 8442);
}

void loop()
{
  Blynk.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}

BLYNK_WRITE(V0) //вперёд - назад
{
  int x;
  int rawX = param.asInt();
  if (rawX == 512) {
    ledcWrite(1, 0);
    ledcWrite(2, 0);
  }
  else if (rawX>512) {
    x = map (rawX, 513, 1023, 0, 1023);
    ledcWrite(2, x);
    ledcWrite(1, 0);
  }
    else if (rawX<512) {
    x = map (rawX, 511, 0, 0, 1023);
    ledcWrite(2, 0);
    ledcWrite(1, x);
  }
}

BLYNK_WRITE(V1) //влево - вправо
{
  int y;
  int rawY = param.asInt();
  if (rawY == 512) {
    ledcWrite(3, 0);
    ledcWrite(4, 0);
  }
  else if (rawY>512) {
    y = map (rawY, 513, 1023, 0, 1023);
    ledcWrite(3, y);
    ledcWrite(4, 0);
  }
    else if (rawY<512) {
    y = map (rawY, 511, 0, 0, 1023);
    ledcWrite(3, 0);
    ledcWrite(4, y);
  }
  
}
