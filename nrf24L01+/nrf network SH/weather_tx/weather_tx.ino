#include <LowPower.h>
#include <SPI.h>
#include <RF24Network.h>
#include <RF24.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define DHTPIN 2
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
const float pm = 0.007500637554192; //1 паскаль = 0.007500637554192 миллиметр ртутного столба

#define pinPowerDHT 3	//Питание dht
#define pinPowerBMP 4	//Питание bmp
#define pinPowerNrf 9	//Питание nrf

RF24 radio(/*ce*/ 7, /*cs*/ 8);
RF24Network network(radio);
const uint8_t this_node = 02;  // Адрес нашего узла 
const uint8_t main_node = 00;  // Адрес, куда по умолчанию сливаем показания
const uint8_t chanel = 100;    // Номер канала
uint8_t WaitCount = 75;   // Количество циклов сна по 8 сек. (10 мин)
boolean allowSend = false; //признак разрешения на отправку
//////////////////////////////////////
#define DEBUG_MODE 0
////////////////////////////////////

struct wst_sensor{
	char    ID[5];     //ИД устойства
	uint32_t UpTime;
	uint16_t count_err;
	int16_t	T;	//Температура
	//long	L;	//Освещенность  (Lx)
	uint16_t	H;	//влажность
	uint16_t	P;	//давление
	uint16_t	V;	//уровень заряда батарейки (mV)
};
struct wst_sensor sensor;
RF24NetworkHeader header(main_node);
uint16_t qt = 0;
uint16_t tt = 0;
uint16_t hh = 0;
uint16_t pp = 0;
uint16_t vv = 0;

void setup(void) {
	pinMode(pinPowerDHT, OUTPUT);
	pinMode(pinPowerBMP, OUTPUT);
	pinMode (pinPowerNrf, OUTPUT);
	SPI.begin();
	if (DEBUG_MODE) {
		Serial.begin(115200);
		Serial.println("-- START DEBUG MODE --");
		WaitCount = 0;
	}
	char ID[5] = "LTS1";
	sensor.count_err = 0;
	memcpy(sensor.ID, ID, 5);
}

void loop(void) {
	//считываем значения датчиков
	if (DEBUG_MODE) Serial.println("data read...");
	digitalWrite(pinPowerDHT, HIGH); // Подаем питание на датчики
	digitalWrite(pinPowerBMP, HIGH);
	dht.begin();
	delay(800);
	bmp.begin();
	int16_t T = dht.readTemperature()*100;
	uint16_t H = dht.readHumidity()*100;
	uint16_t P = bmp.readPressure() * pm;
	digitalWrite(pinPowerDHT, LOW); // Отключаем питание с датчиков
	digitalWrite(pinPowerBMP, LOW);
	uint16_t V = readVcc();
	/////////////////////////////////////////////////////////////
	if (DEBUG_MODE) {
		allowSend = true;     //Есть что отправить - Взведем флаг
		Serial.print("T= ");
		Serial.print(T);
		Serial.print(" tt= ");
		Serial.print(tt);
		Serial.print("; H= ");
		Serial.print(H);
		Serial.print("; hh= ");
		Serial.print(hh);
		Serial.print("; P= ");
		Serial.print(P);
		Serial.print("; pp= ");
		Serial.print(pp);
		Serial.print("; V= ");
		Serial.println(V);
	}
	qt++;
	/////////////////////////////////////////////////////////////
	if (T < tt - 100/*1с*/ || T > tt + 100 || H < hh - 200 || H > hh + 200 || P < pp - 1 || P > pp + 1 || V < vv - 100/*0.1V*/) { // уменьшим кол-во передач
		tt = T;
		hh = H;
		vv = V;
		pp = P;
		allowSend = true;	//Есть что отправить - Взведем флаг
	}
	if (qt >= 12 ) allowSend = true;
	// Отправка данных на базовый узел
	if (allowSend) {
		allowSend = false;
		qt = 0; //обнуление счетчика
		sensor.T = T;
		sensor.H = H;
		sensor.P = P;
		sensor.V = V;
		sensor.UpTime = millis() / 1000;
		digitalWrite (pinPowerNrf, HIGH);
		delay(50);
		radio.begin();
		network.begin( chanel, this_node );
		delay(50);
		if (DEBUG_MODE) Serial.println("Send Data");
		if (!network.write(header, &sensor, sizeof(sensor))) {
		sensor.count_err++;
		}
		digitalWrite (pinPowerNrf, LOW);
	}
	sleep();
}
// Переводим устройство в режим низкого энергопотребления
void sleep() {
	if (DEBUG_MODE) {
		Serial.println("----- SLEEP -----");
		Serial.flush();
	}
	for (int i = 0; i < WaitCount; i++) {  //впадаем в спячку
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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



