#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(7, 8);				// nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);		// Network uses that radio
const uint16_t this_node = 00;	// адррес этой ячейки (main node)
const uint16_t channel = 100;	//выбор канала

struct StreetMeteoSensor
{
  char	ID[5];	//ИД устойства
  unsigned long UpTime;
  float	T;		//Температура
  //long	L;	//Освещенность  (Lx)
  float	H;		//влажность
  int	P;		//давление
  int	V;		//уровень заряда батарейки (mV)
};


void setup(void){
  Serial.begin(115200);
  Serial.println("Start weather rx");

  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  network.begin(channel, this_node);
}

void loop(void) {

  network.update();                  // Check the network regularly

  while ( network.available() ) {     // Is there anything ready for us?

    RF24NetworkHeader header;        // If so, grab it and print it out
    StreetMeteoSensor payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("ID: ");
    Serial.print(payload.ID);
    Serial.print("; UpTime: ");
    Serial.print(payload.UpTime);
    Serial.print("; Temp: ");
    Serial.print(payload.T);
    Serial.print("; Hum: ");
    Serial.print(payload.H);
    Serial.print("; P: ");
    Serial.print(payload.P);
    Serial.print("; volts: ");
    Serial.println(payload.V);
  }
}
