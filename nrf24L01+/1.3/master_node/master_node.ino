#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


#define ON 1
#define OFF 0
RF24 radio(7, 8);        // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);   // Network uses that radio
const uint16_t pws_node = 01; //выключатель 01
const uint16_t wst_node = 02; //погодная станция
const uint16_t this_node = 00;  // адррес этой ячейки (main node)
const uint16_t channel = 100; //выбор канала
const unsigned long interval = 1000; // ms       // Delay manager to send pings regularly.
unsigned long last_time_sent;
//////////////////////////////////////
#define DEBUG_MODE 1
////////////////////////////////////
struct wst_get {  //структура получения данных от погодной станции
	char  ID[5];  //ИД устойства
	unsigned long UpTime;
	int count_err;
	float T;    //Температура
	//long  L;  //Освещенность  (Lx)
	float H;    //влажность
	int P;    //давление
	int V;    //уровень заряда батарейки (mV)
};
wst_get wst_payload;

struct pws_get {  //структура получения данных от выключателя
	char  ID[5];  //ИД устойства
	unsigned long UpTime;
	int count_err;
	boolean sw;   //состояние реле
	int I;      //ток
	int vcc;
};
pws_get pws_get_payload;

struct pws_set {  //структура отправки данных на выкл.
	char  ID[5];  //ИД устойства
	boolean sw;   //состояние реле
};
pws_set pws_set_payload;

void setup(void) {
	if (DEBUG_MODE) {
		Serial.begin(115200);
		Serial.println("-- DEBUG ENABLED --");
		Serial.println("-Start master node-");
	}

	radio.begin();
	radio.setPALevel(RF24_PA_HIGH);
	network.begin(channel, this_node);
}

void loop(void) {
	network.update();
	if ( network.available() ) {
		RF24NetworkHeader header;
		network.peek(header);
		int _ID = header.from_node;
		if (DEBUG_MODE) {
			Serial.print("_ID ");
			Serial.println(_ID);
		}

		switch (_ID) {
		case pws_node:  //PWS
			network.read(header, &pws_get_payload, sizeof(pws_get_payload));
			Serial.print("ID: ");
			Serial.print(pws_get_payload.ID);
			Serial.print("; UpTime: ");
			Serial.print(pws_get_payload.UpTime);
			Serial.print("; count_err: ");
			Serial.print(pws_get_payload.count_err);
			Serial.print("; sw: ");
			Serial.print(pws_get_payload.sw);
			Serial.print("; I: ");
			Serial.print(pws_get_payload.I);
			Serial.print("; vcc: ");
			Serial.println(pws_get_payload.vcc);
			break;

		case wst_node:  //wst
			network.read(header, &wst_payload, sizeof(wst_payload));
			Serial.print("ID: ");
			Serial.print(wst_payload.ID);
			Serial.print("; UpTime: ");
			Serial.print(wst_payload.UpTime);
			Serial.print("; count_err: ");
			Serial.print(pws_get_payload.count_err);
			Serial.print("; Temp: ");
			Serial.print(wst_payload.T);
			Serial.print("; Hum: ");
			Serial.print(wst_payload.H);
			Serial.print("; P: ");
			Serial.print(wst_payload.P);
			Serial.print("; volts: ");
			Serial.println(wst_payload.V);
			break;

		default:
			Serial.print("*** WARNING *** Unknown ID node ");
			Serial.println(_ID);
			break;
		};
	}
/*
	unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
	if ( now - last_time_sent >= interval ) {
		last_time_sent = now;
		pws_send(pws_node, "PWS1", ON);
	}
 */
}

void pws_send(byte node, char id[5], boolean sw_set) {
	memcpy(pws_set_payload.ID, id, 5);
	pws_set_payload.sw = sw_set;
	RF24NetworkHeader header(node);
	if (network.write(header, &pws_set_payload, sizeof(pws_set_payload))) {
		;
		Serial.println("Send OK");
	} else Serial.println("Send FAIL");
}

