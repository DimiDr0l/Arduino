#include <SoftwareSerial.h>
SoftwareSerial sSerial(10, 9); //rx tx
#define PZEM_VOLTAGE (uint8_t)0xB0
#define RESP_VOLTAGE (uint8_t)0xA0

#define PZEM_CURRENT (uint8_t)0xB1
#define RESP_CURRENT (uint8_t)0xA1

#define PZEM_POWER   (uint8_t)0xB2
#define RESP_POWER   (uint8_t)0xA2

#define PZEM_ENERGY  (uint8_t)0xB3
#define RESP_ENERGY  (uint8_t)0xA3

#define RESPONSE_SIZE sizeof(PZEMCommand)
#define PZEM_DEFAULT_READ_TIMEOUT 1000
#define RESPONSE_DATA_SIZE RESPONSE_SIZE - 2
#define PZEM_ERROR_VALUE - 1.0

struct PZEMCommand {
  uint8_t command;
  const uint8_t addr[4] = {0x00, 0x00, 0x00, 0x00};
  const uint8_t data = 0x00;
  uint8_t crc;
};

void send(uint8_t cmd) {
  PZEMCommand pzem;
  pzem.command = cmd;
  pzem.crc = pzem.command;
  uint8_t *bytes = (uint8_t*)&pzem;
  sSerial.write (bytes, sizeof(pzem));
}

bool recieve(uint8_t resp, uint8_t *data) {
  uint8_t buffer[RESPONSE_SIZE];

  unsigned long startTime = millis();
  uint8_t len = 0;
  while ((len < RESPONSE_SIZE) && (millis() - startTime < PZEM_DEFAULT_READ_TIMEOUT)) {
    if (sSerial.available() > 0) {
      uint8_t c = uint8_t(sSerial.read());
      if (!c && !len)
      continue; // skip 0 at startup
      buffer[len++] = c;
    }
  }

  if (len != RESPONSE_SIZE)
  return false;

  if (buffer[6] != crc(buffer, len - 1))
  return false;

  if (buffer[0] != resp)
  return false;

  if (data){
    for (int i = 0; i < RESPONSE_DATA_SIZE; i++)
    data[i] = buffer[1 + i];
  }
  return true;
}

uint8_t crc(uint8_t *data, uint8_t sz){
  uint16_t crc = 0;
  for(uint8_t i=0; i<sz; i++)
  crc += *data++;
  return (uint8_t)(crc & 0xFF);
}

float voltage() {
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_VOLTAGE);
  if (!recieve(RESP_VOLTAGE, data))
  return PZEM_ERROR_VALUE;
  return (data[0] << 8) + data[1] + (data[2] / 10.0);
}

float current(){
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_CURRENT);
  if(!recieve(RESP_CURRENT, data))
  return PZEM_ERROR_VALUE;
  return (data[0] << 8) + data[1] + (data[2] / 100.0);
}

float power(){
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_POWER);
  if(!recieve(RESP_POWER, data))
  return PZEM_ERROR_VALUE;
  return (data[0] << 8) + data[1];
}

float energy(){
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_ENERGY);
  if(!recieve(RESP_ENERGY, data))
  return PZEM_ERROR_VALUE;
  return ((uint32_t)data[0] << 16) + ((uint16_t)data[1] << 8) + data[2];
}

void setup() {
  Serial.begin (115200);
  sSerial.begin(9600);
  Serial.println ("start");
}

void loop() {
  Serial.println (voltage());
  Serial.println (current());
  Serial.println (power());
  Serial.println (energy());
  delay (1500);
}
