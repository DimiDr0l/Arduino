long startMillis, finishMillis;

void setup() {
  //char crc8;
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay (100);
  startMillis = millis();
  Serial.println (millis());
  byte crc = Crc8("YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012YOUR_AP_PASSWORD90YOUR_AP_PASSWORD90YOUR_AP_PASSWORD9012");
  Serial.println (millis());
  finishMillis =  millis();
  Serial.print ("time remaning: ");
  Serial.println (finishMillis - startMillis);
  Serial.println (crc);
}

void loop() {
}

unsigned char Crc8(char* pcBlock) {
  int len = strlen(pcBlock);
  unsigned char crc = 0;//0xFF;
  unsigned int i;

  while (len--)
  {
    crc ^= *pcBlock++;

    for (i = 0; i < 8; i++)
      crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
  }

  return crc;
}
