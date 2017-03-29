#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#define pin_sw 2

RF24 radio(/*ce*/ 7, /*cs*/ 8);
RF24Network network(radio);
uint16_t this_node = 01;  // Адрес нашего узла
uint16_t main_node = 00;  // Адрес, куда по умолчанию сливаем показания
uint16_t channel = 100;    // Номер канала
const char pws_header = '@'; //заголовок пакета
//////////////////////////////////////
#define DEBUG_MODE 1
////////////////////////////////////
const unsigned long interval = 3000; // ms       // Delay manager to send pings regularly.
unsigned long last_time_sent;

struct pws_send { //
  char  ID[5];  //ИД устойства
  unsigned long UpTime;
  int count_err;
  boolean sw;   //состояние реле
  int I;      //ток
  int vcc;
};
pws_send send_data;

struct pws_receive { //struct powersw sensor;
  char  ID[5];  //ИД устойства
  boolean sw;   //состояние реле
};
pws_receive receive_data;

void setup(void) {
  pinMode (pin_sw, OUTPUT);
  digitalWrite (pin_sw, LOW);
  if (DEBUG_MODE) {
    Serial.begin(115200);
    Serial.println("-- START DEBUG MODE --");
  }
  SPI.begin();
  radio.begin();
  char ID[5] = "PWS1";
  send_data.count_err = 0;
  memcpy(send_data.ID, ID, 5);
  radio.setPALevel(RF24_PA_HIGH);
  network.begin(channel, this_node);
}

void loop(void) {
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
    send_data.sw = true;
    send_data.I = 7.3;
    send_data.vcc = readVcc();
    RF24NetworkHeader header(main_node);
    if (network.write(header, &send_data, sizeof(send_data))) {
      Serial.println ("Send OK");
    } 
    else {
      send_data.count_err++;
      Serial.println ("Send FAIL");
    }
  }

}

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

