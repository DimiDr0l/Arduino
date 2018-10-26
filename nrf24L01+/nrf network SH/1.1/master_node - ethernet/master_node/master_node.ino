#include <RF24Network.h>
#include <RF24.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <SPI.h>


#define ON 1
#define OFF 0
RF24 radio(7, 8);        // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);   // Network uses that radio
const uint16_t pws_node = 01; //выключатель 01
const uint16_t wst_node = 02; //погодная станция
const uint16_t this_node = 00;  // адррес этой ячейки (main node)
const uint16_t channel = 100; //выбор канала

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress server(192, 168, 1, 200);
EthernetClient ethClient;
PubSubClient client (ethClient);
//const unsigned long interval = 1000; // ms       // Delay manager to send pings regularly.
//unsigned long last_time_sent;

String prefix   = "/IoTmanager";     // global prefix for all topics - must be some as mobile device
String deviceID = "mainnode";   // thing ID - unique device id in our project
String eloginmqt = "smart";
String epassmqt = "1qaz@WSX";
String sTopic;
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
	int Vcc;    //уровень заряда батарейки (mV)
};
wst_get wst_payload;

struct pws_get {  //структура получения данных от выключателя
	char  ID[5];  //ИД устойства
	unsigned long UpTime;
	int count_err;
	boolean sw;   //состояние реле
	int I;      //ток
	int V;
};
pws_get pws_payload;

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
	client.setServer(server, 1883);
	client.setCallback(callbackmqt);

	radio.begin();
	radio.setPALevel(RF24_PA_HIGH);
	network.begin(channel, this_node);
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		for (;;)
		;
	}
	Serial.print("My IP address: ");
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print(".");
	}
	Serial.println();
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
			network.read(header, &pws_payload, sizeof(pws_payload));
			sTopic = prefix + "/" + deviceID + "/" + pws_payload.ID;
			pubStatus (sTopic + "/uptime", String(pws_payload.UpTime));
			pubStatus (sTopic + "/c_err", String(pws_payload.count_err));
			pubStatus (sTopic + "/switch", String(pws_payload.sw));
			pubStatus (sTopic + "/volts", String(pws_payload.V));
			pubStatus (sTopic + "/amp", String(pws_payload.I));

			if (DEBUG_MODE) {
				Serial.print("ID: ");
				Serial.print(pws_payload.ID);
				Serial.print("; UpTime: ");
				Serial.print(pws_payload.UpTime);
				Serial.print("; count_err: ");
				Serial.print(pws_payload.count_err);
				Serial.print("; sw: ");
				Serial.print(pws_payload.sw);
				Serial.print("; I: ");
				Serial.print(pws_payload.I);
				Serial.print("; V: ");
				Serial.println(pws_payload.V);
			}
			break;

		case wst_node:  //wst
			network.read(header, &wst_payload, sizeof(wst_payload));
			sTopic = prefix + "/" + deviceID + "/" + wst_payload.ID;
			pubStatus (sTopic + "/temp", String(wst_payload.T));
			pubStatus (sTopic + "/hum", String(wst_payload.H));
			pubStatus (sTopic + "/pres", String(wst_payload.P));
			pubStatus (sTopic + "/uptime", String(wst_payload.UpTime));
			pubStatus (sTopic + "/battery", String(wst_payload.Vcc));
			pubStatus (sTopic + "/error", String(wst_payload.count_err));

			if (DEBUG_MODE) {
				Serial.print("ID: ");
				Serial.print(wst_payload.ID);
				Serial.print("; UpTime: ");
				Serial.print(wst_payload.UpTime);
				Serial.print("; count_err: ");
				Serial.print(wst_payload.count_err);
				Serial.print("; Temp: ");
				Serial.print(wst_payload.T);
				Serial.print("; Hum: ");
				Serial.print(wst_payload.H);
				Serial.print("; P: ");
				Serial.print(wst_payload.P);
				Serial.print("; Battery: ");
				Serial.println(wst_payload.Vcc);
			}
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
	reconnect();
	yield();
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

	if (topics == prefix + "/" + deviceID + "/pws1" + "/control") {
		if (p == "0") {
			Serial.println("AllOff");
		} else if (p == "1" ) {
			Serial.println("AllOn");
		}
	}
}

void reconnect() {
	if (!client.connected()) {
		if (client.connect(deviceID.c_str(), eloginmqt.c_str(), epassmqt.c_str())) {
			Serial.println("connected");
			client.subscribe((prefix + "/+/+/control/#").c_str()); // for receiving GPIO control messages
			Serial.println("Subscribe: Success");
		} else {
			Serial.print("Connect to MQTT server: FAIL ");
		}
	} else client.loop();
}

void pubStatus(String t, String payload) {
	if (client.publish((t + "/status").c_str(), payload.c_str())) {
		Serial.println("Publish new status for " + t + ", value: " + payload);
	} else {
		Serial.println("Publish new status for " + t + " FAIL!");
	}
}


