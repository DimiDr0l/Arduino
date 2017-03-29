#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


#define ON 1
#define OFF 0
RF24 radio(7, 8);				// nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);		// Network uses that radio
const char pws_header = '@'; //заголовок пакета
const char lts_header = '!'; //заголовок пакета
const uint16_t pws_node = 01;
const uint16_t lts_node = 02;
const uint16_t this_node = 00;	// адррес этой ячейки (main node)
const uint16_t channel = 100;	//выбор канала
const unsigned long interval = 1000; // ms       // Delay manager to send pings regularly.
unsigned long last_time_sent;

struct lts_receive
{
	char	ID[5];	//ИД устойства
	unsigned long UpTime;
	float	T;		//Температура
	//long	L;	//Освещенность  (Lx)
	float	H;		//влажность
	int	P;		//давление
	int	V;		//уровень заряда батарейки (mV)
};


struct pws_get {
	char	ID[5];	//ИД устойства
	unsigned long UpTime;
	boolean	sw;		//состояние реле
	int	I;			//ток
	int	vcc;
};

struct pws_set {
	char	ID[5];	//ИД устойства
	boolean	sw;		//состояние реле
};
pws_set pws_set_payload;

void setup(void){
	Serial.begin(115200);
	Serial.println("Start master node");

	radio.begin();
	radio.setPALevel(RF24_PA_HIGH);
	network.begin(channel, this_node);
}

void loop(void) {
	network.update();                  // Check the network regularly
	if ( network.available() ) {     // Is there anything ready for us?

		RF24NetworkHeader header;        // If so, grab it and print it out
		network.peek(header);
		int _ID = header.from_node;
		Serial.print("_ID ");
        Serial.println(_ID);
		switch (header.type){                              // Dispatch the message to the correct handler.
		case '@':	//PWS
			pws_get pws_get_payload;
			network.read(header, &pws_get_payload, sizeof(pws_get_payload));
			Serial.print("ID: ");
			Serial.print(pws_get_payload.ID);
			Serial.print("; UpTime: ");
			Serial.print(pws_get_payload.UpTime);
			Serial.print("; sw: ");
			Serial.print(pws_get_payload.sw);
			Serial.print("; I: ");
			Serial.print(pws_get_payload.I);
			Serial.print("; vcc: ");
			Serial.println(pws_get_payload.vcc);
			break;
		case '!':	//LTS
			lts_receive lts_payload;
			network.read(header, &lts_payload, sizeof(lts_payload));
			Serial.print("ID: ");
			Serial.print(lts_payload.ID);
			Serial.print("; UpTime: ");
			Serial.print(lts_payload.UpTime);
			Serial.print("; Temp: ");
			Serial.print(lts_payload.T);
			Serial.print("; Hum: ");
			Serial.print(lts_payload.H);
			Serial.print("; P: ");
			Serial.print(lts_payload.P);
			Serial.print("; volts: ");
			Serial.println(lts_payload.V);
			break;
		default:
			Serial.print("*** WARNING *** Unknown message type ");
			Serial.println(header.type);
			break;
		};
	}
	
	unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms  
	if ( now - last_time_sent >= interval ){
		last_time_sent = now;
		pws_send(pws_node, "PWS1", ON);

	}
	
}
void pws_send(byte node, char id[5], boolean sw_set){
		memcpy(pws_set_payload.ID, id, 5);
		pws_set_payload.sw = sw_set;
		RF24NetworkHeader header(node, pws_header);
		if (network.write(header,&pws_set_payload,sizeof(pws_set_payload))) {;
		Serial.println("Send OK");
		} else Serial.println("Send FAIL");
		
		
}
