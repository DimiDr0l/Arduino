#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
// #include <ArduinoJson.h>
// StaticJsonBuffer<256> jsonBuffer;
//JsonObject& js_out = jsonBuffer.createObject();
String js_out;


RF24 radio(7, 8);        // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);   // Network uses that radio
const uint8_t wst_node = 02; //погодная станция
const uint8_t this_node = 00;  // адррес этой ячейки (main node)
const uint8_t channel = 100; //выбор канала
//const uint32_t interval = 1000; // ms       // Delay manager to send pings regularly.
//unsigned uint32_t last_time_sent;
//////////////////////////////////////
#define DEBUG_MODE 0
////////////////////////////////////
struct wst_get {  //структура получения данных от погодной станции
  char  ID[5];  //ИД устойства
  uint32_t UpTime;
  uint16_t count_err;
  int16_t T;    //Температура
  //long  L;  //Освещенность  (Lx)
  uint16_t H;    //влажность
  uint16_t P;    //давление
  uint16_t mV;    //уровень заряда батарейки (mV)
};
wst_get wst_payload;

void setup(void) {
  Serial.begin(115200);
  Serial.println("-Start master node-");

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
/////////////////////////////////////"{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}"
    switch (_ID) {
    /*case pws_node:  //PWS
       network.read(header, &pws_get_payload, sizeof(pws_get_payload));
    js_out = "{\"ID\":\"" + String(pws_get_payload.ID);
    js_out += "\",\"UpTime\":" + String(pws_get_payload.UpTime);
    js_out += ",\"count_err\":" + String(pws_get_payload.count_err);
    js_out += ",\"sw\":" + String(pws_get_payload.sw);
    js_out += ",\"I\":" + String(pws_get_payload.I);
    js_out += ",\"V\":" + String(pws_get_payload.V);
    js_out += ",\"W\":" + String(pws_get_payload.W);
  js_out += ",\"E\":" + String(pws_get_payload.E) + String("}");
        
      //js_out["ID"] = pws_get_payload.ID;
      //js_out["UpTime"] = pws_get_payload.UpTime;
      //js_out["count_err"] = pws_get_payload.count_err;
      //js_out["sw"] = pws_get_payload.sw;
      //js_out["I"] = pws_get_payload.I;
      //js_out["V"] = pws_get_payload.V;
      //js_out["W"] = pws_get_payload.W;
      //js_out.printTo(Serial);
        Serial.println(js_out);
      
      if (DEBUG_MODE) {
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
        Serial.print("; V: ");
        Serial.println(pws_get_payload.V);
      }
      break;
*/
    case wst_node:  //wst
      network.read(header, &wst_payload, sizeof(wst_payload));
    js_out = "{\"ID\":\"" + String(wst_payload.ID);
    js_out += "\",\"UpTime\":" + String(wst_payload.UpTime);
    js_out += ",\"count_err\":" + String(wst_payload.count_err);
    js_out += ",\"T\":" + String(wst_payload.T);
    js_out += ",\"H\":" + String(wst_payload.H);
    js_out += ",\"P\":" + String(wst_payload.P);
    js_out += ",\"V\":" + String(wst_payload.mV) + String("}");
    
      //js_out["ID"] = wst_payload.ID;
      //js_out["UpTime"] = wst_payload.UpTime;
      //js_out["count_err"] = wst_payload.count_err;
      //js_out["T"] = wst_payload.T;
      //js_out["H"] = wst_payload.H;
      //js_out["P"] = wst_payload.P;
      //js_out["V"] = wst_payload.mV;
      //js_out.printTo(Serial);
        Serial.println(js_out);
      
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
        Serial.print("; volts: ");
        Serial.println(wst_payload.mV);
      }
      break;

    default:
      Serial.print("*** WARNING *** Unknown ID node ");
      Serial.println(_ID);
      break;
    };
  }
}

/*   while (Serial.available()) {
    String S;
    S = Serial.readString();
    JsonObject& js_in = jsonBuffer.parseObject(S);
    // Test if parsing succeeds.
    if (!js_in.success()) {
      Serial.println("parseObject() failed");
      return;
    } 
    String id_j = js_in["ID"];
    boolean sw_j;
    if (id_j == "PWS1") {
      sw_j = js_in["sw"];
      pws_send(pws_node, "PWS1", sw_j);
    }
    if (DEBUG_MODE) {
      Serial.print(id_j);
      Serial.print(" : ");
      Serial.println(sw_j);
    }
  }

   
unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
if ( now - last_time_sent >= interval ) {
  last_time_sent = now;
  pws_send(pws_node, "PWS1", ON);
}

}

void pws_send(byte node, char id[5], boolean sw_set) {
  memcpy(pws_set_payload.ID, id, 5);
  pws_set_payload.sw = sw_set;
  RF24NetworkHeader header(node);
  boolean ok = network.write(header, &pws_set_payload, sizeof(pws_set_payload));
//  if (DEBUG_MODE){
    if (ok) {
      Serial.println("Send OK");
    } else Serial.println("Send FAIL");
//  }
}
 */
