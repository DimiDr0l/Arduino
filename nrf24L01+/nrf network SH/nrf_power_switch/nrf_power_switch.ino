#include <RF24Network.h>
#include <RF24.h>

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


#define pin_sw 2
RF24 radio(/*ce*/ 7, /*cs*/ 8);
RF24Network network(radio);
const uint8_t this_node = 01;  // Адрес нашего узла
const uint8_t main_node = 00; // Адрес, куда по умолчанию сливаем показания
const uint8_t channel = 100;  // Номер канала
//////////////////////////////////////
#define DEBUG_MODE 0
////////////////////////////////////
const uint32_t interval = 60000; // ms       // Delay manager to send pings regularly.
uint32_t last_time_sent;

struct pws_send {
  char  ID[5];  //ИД устойства
  uint32_t UpTime;
  uint16_t count_err;
  boolean sw;   //состояние реле
  float I;    //ток
  float V;    //напряжение
  float W;    //мощьность
  float E;    //вт/ч

};
pws_send send_data;

struct pws_receive { //struct powersw sensor;
  char  ID[5];  //ИД устойства
  boolean sw;   //состояние реле
};
pws_receive receive_data;

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
  Serial.write (bytes, sizeof(pzem));
}

bool recieve(uint8_t resp, uint8_t *data) {
  uint8_t buffer[RESPONSE_SIZE];

  unsigned long startTime = millis();
  uint8_t len = 0;
  while ((len < RESPONSE_SIZE) && (millis() - startTime < PZEM_DEFAULT_READ_TIMEOUT)) {
    while (Serial.available()) {
      uint8_t c = uint8_t(Serial.read());
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

  if (data) {
    for (int i = 0; i < RESPONSE_DATA_SIZE; i++)
      data[i] = buffer[1 + i];
  }
  return true;
}

uint8_t crc(uint8_t *data, uint8_t sz) {
  uint16_t crc = 0;
  for (uint8_t i = 0; i < sz; i++)
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

float current() {
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_CURRENT);
  if (!recieve(RESP_CURRENT, data))
    return PZEM_ERROR_VALUE;
  return (data[0] << 8) + data[1] + (data[2] / 100.0);
}

float power() {
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_POWER);
  if (!recieve(RESP_POWER, data))
    return PZEM_ERROR_VALUE;
  return (data[0] << 8) + data[1];
}

float energy() {
  uint8_t data[RESPONSE_DATA_SIZE];
  send(PZEM_ENERGY);
  if (!recieve(RESP_ENERGY, data))
    return PZEM_ERROR_VALUE;
  return ((uint32_t)data[0] << 16) + ((uint16_t)data[1] << 8) + data[2];
}

void setup() {
  pinMode (pin_sw, OUTPUT);
  digitalWrite (pin_sw, LOW);
  Serial.begin(9600);
  if (DEBUG_MODE) {
  Serial.begin(115200);
    Serial.println("-- START DEBUG MODE --");
  }
  radio.begin();
  char ID[5] = "PWS1";
  memcpy(send_data.ID, ID, 5);
  radio.setPALevel(RF24_PA_HIGH);
  network.begin(channel, this_node);
}

void loop() {
  network.update();
  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
    byte _ID = header.from_node;
    if (_ID == main_node) {
      network.read(header, &receive_data, sizeof(receive_data));
      if (DEBUG_MODE) {
        Serial.print("_ID ");
        Serial.println(_ID);
        Serial.print("ID: ");
        Serial.print(receive_data.ID);
        Serial.print("; sw: ");
        Serial.println(receive_data.sw);
      }
      if (receive_data.sw) digitalWrite (pin_sw, HIGH);
      else digitalWrite (pin_sw, LOW);
	  send_data.sw = digitalRead(pin_sw);
	  SendtoNode();
    }
    else {
      Serial.print("*** WARNING *** Unknown ID node ");
      Serial.println(_ID);
    }
  }

  unsigned long now = millis();
  if ( now - last_time_sent >= interval ) {
    last_time_sent = now;
    send_data.UpTime = now / 1000;
    send_data.sw = digitalRead(pin_sw);
    send_data.V = voltage();
	if (send_data.V != PZEM_ERROR_VALUE) {
    send_data.I = current();
    send_data.W = power();
    send_data.E = energy();
	} else {
	send_data.I = 0;
	send_data.W = 0;
	send_data.E = 0;
	}
	SendtoNode();

    if (DEBUG_MODE) {
      Serial.print ("ID: ");
      Serial.print (send_data.ID);
      Serial.print ("; UpTime: ");
      Serial.print (send_data.UpTime);
      Serial.print ("; count_err: ");
      Serial.print (send_data.count_err);
      Serial.print ("; sw: ");
      Serial.print (send_data.sw);
      Serial.print ("; I: ");
      Serial.print (send_data.I);
      Serial.print ("; V: ");
      Serial.print (send_data.V);
      Serial.print ("; W: ");
      Serial.print (send_data.W);
	  Serial.print ("; E: ");
      Serial.println (send_data.E);
    }
  }

}
void SendtoNode()
{
    RF24NetworkHeader header(main_node);
    if (network.write(header, &send_data, sizeof(send_data))) {
      if (DEBUG_MODE) Serial.println ("Send OK");
    }
    else {
      send_data.count_err++;
      if (DEBUG_MODE) Serial.println ("Send FAIL");
    }
}

/*
  // Измерение напряжения питания
  int readVcc() {
  int result;
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(75); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  result = ADCL;
  result |= ADCH << 8;
  result =  1125300L / result; // (Kvcc * 1023.0 * 1000) (in mV);
  return result;
  }
*/


