#include <RCSwitch.h> //   управление розетками SC2260/2262 http://code.google.com/p/rc-switch/
#include <livolo.h> // управление выключателями Livolo
#define  txPin  8 // пин передатчика
Livolo livolo(8); // объект Livolo
RCSwitch mySwitch = RCSwitch(); // объект RC-Switch
int weatherData = 0;
unsigned long dimmerDelay, timerStart, timerStop, timerSwitchStart, timerSwitchStop, timerRange; // таймеры
int prevRange, nowRange, tempRange, switchRange, vectorRange, initRange, rangeFinder, tempC;
byte nightLightLevel, rainbow, delta, redB, greenB, blueB, targetL, targetR, targetG, targetB, lastValueRGB;
byte lightMode = 0; // режим светильника (0 - ночник, 1 - светильник, 2 - термометр, 3 - радуга ручная, 4 - радуга автоматическая)
byte lastMode = 1; // режим для первого включения всегда светильник
byte valueRGB = 0; // яркость или оттенок (нормализованное значение nowRange)
boolean rainbowUp, followMeLight; // направление радуги
boolean timing = false; // признак работающего таймера (рука над лампой)
boolean switchLock = false; // признак запрета на переключение режимов до снятия руки с лампы
boolean tempLock = false; // признак запрета на индикацию температуры до получения нового значения
boolean backLight = false; // признак работающего фонового света
boolean mainLight = false; // признак работающего верхнего света
boolean nightLight = false; // признак режима ночника
boolean tempRcvd = false; // признак полученной температуры
#define nightLightLimit 65 // яркость ночника
#define rangePin A0 // датчик препятствия
#define redPin 9 // красный
#define greenPin 10 // зеленый
#define bluePin 11 // синий
#define bottom 630 // низ зоны регулировки
#define top 135 // верх зоны регулировки
#define delta 20 // допустимое отклонение при определении дистанции
#define shortDelay 350 // задержка для переключения режимов
#define longDelay 3000
#define timeOutRange 100 // интервал между вычислением дистанции нужен так как при "скольжении" руки дистанция меняется, а значит меняется яркость или цвет
#define rainbowStep 75 // интервал смены оттенка радуги
#define dimmerStep 35 // скорость приглушения ночника
void setup() {
	//  Serial.begin(9600);
	pinMode(rangePin, INPUT);
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	mySwitch.enableTransmit(txPin); // разрешена передача
	mySwitch.enableReceive(0);  // разрешен прием (прерывание 0 -> пин 2)
	rgbLight(0, 0, 0); // светильник включается выключенным
	lastMode = 1; // первое включение всегда в режиме света
	timerRange = millis(); // старт таймера на периодическое считывание дистанции
	nowRange = getRange(); // первое вычисление дистанции
}
void loop() {
	// ПЕРИОДИЧЕСКОЕ ВЫЧИСЛЕНИЕ ДИСТАНЦИИ
	if ((millis() - timerRange) > timeOutRange) { // вычисление дистанции раз в timeOutRange
		prevRange = nowRange; // память предыдущего значения дистанции
		nowRange = getRange(); // текущее значение дистанции
		timerRange = millis(); // сброс таймера интервала вычисления дистанции
		// Serial.print("Range: ");
		// Serial.println(nowRange);
	}
	// ПРОВЕЛИ РУКОЙ -- запуск таймеров и переключение режимов (если допустимо), задержали руку - сервисные функции (если допустимо)
	if ((nowRange < bottom + delta) && (nowRange > top)) { // если дистанция больше порога и меньше верхней границы
		if (nowRange > bottom - delta) { // выключение света раньше "дна" как индикатор для удержания руки при полном выключении (2*delta)
			valueRGB = 0;
		} else {
			valueRGB = 254 - map(nowRange, top, bottom, 0, 254); // приведение текущего значения к диапазону 0 - 254 и "переворот"
		}
		// Serial.print("valueRGB: ");
		// Serial.println(valueRGB);
		// Цветовая подсказка для переключения фонового и верхнего света
		if (lightMode == 1) { // если режим светильника
			if (valueRGB > 70 && valueRGB < 110) { // если рука в середине нижней половины
				followMeLight = true; // флаг блокировки регулировки света белого свечения, разрешен только оттенок индикатора
				lastValueRGB = valueRGB; // память последнего значения яркости
				rgbLight(0, 255, 0); // зеленый оттенок как индикатор возможности переключения "ближнего" света
				// Serial.println("Set Green to 255");
			} else {
				if (valueRGB > 175 && valueRGB < 215) { // если рука в середине верхней половины
					followMeLight = true;
					lastValueRGB = valueRGB; // память последнего значения яркости
					rgbLight(0, 0, 255); // синий оттенок как индикатор возможности переключения "дальнего" света
					// Serial.println("Set Blue to 255");
				} else {
					if (valueRGB > 235) { // если рука у верхней границы
						followMeLight = true;
						lastValueRGB = valueRGB; // память последнего значения яркости
						rgbLight(valueRGB, 0, 0); // красный оттенок как индикатор предела регулировки яркости
						// Serial.println("Set RED");
					} else {
						lastValueRGB = valueRGB;
						followMeLight = false;
					}
				}
			}
		}
		// И так далее
		if (timing == false) { // если таймер не работает
			timerStart = millis(); // запуск таймера на удержание руки
			timerSwitchStart = millis(); // запуск таймера на взмах руки
			timing = true; // признак работающего таймера
		}
		if (timing == true) { // если запущен таймер коротких и длинных задержек (рука над лампой)
			// СБРОС ТАЙМЕРОВ ЕСЛИ РУКА НАД ЛАМПОЙ ДВИГАЕТСЯ
			tempRange = nowRange - prevRange; // вычисление смещения руки при длительном удержании над лампой
			if (abs(tempRange) > delta) { // если смещение больше допустимого
				timerStart = millis(); // перезапуск таймера длинной задержки
			}
			if ((millis() - timerStart) > longDelay) { // если руку задержали над лампой на время переключения режимов
				// ВКЛЮЧЕНИЕ СВЕТИЛЬНИКА
				if (lightMode == 0 && switchLock == false) { // если был режим ночника - переключение в последний активный режим
					lightMode = lastMode;
					switchLock = true; // переключение блокируется до отвода руки от лампы
				}
				// СЕРВИСНЫЕ ФУНКЦИИ (включение и выключение дополнительного освещения в режиме обычного светильника)
				if (lightMode == 1 && switchLock == false) { // если режим света
					// Переключение фонового света
					if ((nowRange > (bottom / 2)) && (nowRange < (bottom - 2 * delta))) { // если руку задержали в нижней половине, но выше нижнего порога +4*delta
						//        // Serial.print("Backlight mode (nowRange): ");
						//        // Serial.println(nowRange);
						//        // Serial.print("backLight:");
						//        // Serial.println(backLight);
						if (backLight == false) { // если фоновый свет выключен
							// включаем фоновый свет
							mySwitch.send(863029, 24);  // фоновый свет
							backLight = true;
							switchLock = true;
							//           // Serial.println("Backlight ON");
							//        // Serial.print("backLight:");
							//        // Serial.println(backLight);
						} else { // если фоновый свет включен
							// выключаем фоновый свет
							mySwitch.send(863028, 24);  // фоновый свет
							backLight = false;
							switchLock = true;
							//           // Serial.println("Backlight OFF");
							//        // Serial.print("backLight:");
							//        // Serial.println(backLight);
						}
					}
					// Переключение верхнего света
					if ((nowRange < (bottom / 2)) && (nowRange > top)) { // если руку задержали в верхней половине, но ниже верхнего порога
						/*
					// Serial.print("Main light mode (nowRange): ");
					// Serial.println(nowRange);
					// Serial.print("mainLight:");
					// Serial.println(mainLight);
			*/
						if (mainLight == false) { // если фоновый свет выключен
							// включаем основной свет
							livolo.sendButton(8500, 0); // весь верхний свет
							livolo.sendButton(8500, 96); // весь верхний свет
							mainLight = true;
							switchLock = true;
							/*
				// Serial.println("Main light ON");
				// Serial.print("mainLight:");
				// Serial.println(mainLight);
			*/
						} else { // если фоновый свет включен
							// выключаем основной свет
							livolo.sendButton(8500, 0); // весь верхний свет
							livolo.sendButton(8500, 96); // весь верхний свет
							mainLight = false;
							switchLock = true;
							/*
					// Serial.println("Main light OFF");
					// Serial.print("mainLight:");
					// Serial.println(mainLight);
			*/
						}
					}
				}
				// ВЫКЛЮЧЕНИЕ светильника и переход в режим ночника
				if (nowRange > (bottom - delta)) { // если рука ниже порога (2*delta)
					// Serial.print("Full off mode (nowRange): ");
					// Serial.println(nowRange);
					if (lightMode > 0 && switchLock == false) { // если не в режиме ночника
						blinkLight(150);
						rgbLight(0, 0, 0); // выключение света
						livolo.sendButton(8500, 106); // весь верхний свет
						mySwitch.send(863028, 24);  // фоновый свет
						mainLight = false;
						backLight = false;
						lastMode = lightMode; // память последнего режима
						lightMode = 0; // режим ночника
						switchLock = true; // блокировка переключений до следующего взмаха
					}
				}
			} // > longDelay
		} // timing = true
		// РЕГУЛИРОВКА яркости светильника
		if (lightMode == 1) {
			if (followMeLight == false) { // флаг блокировки регулировки света белого свечения, разрешен только оттенок индикатора
				// Serial.println("Brightness control");
				rgbLight(valueRGB, valueRGB, valueRGB);
			}
		}
		// РЕГУЛИРОВКА ОТТЕНКА
		if (lightMode == 3) {
			if (valueRGB > 0) { // регулировка оттенка
				colorRGB(valueRGB);
			} else {
				rgbLight(0, 0, 0);  // приглушение света для индикации режима выключения
			}
		}
	} // nowRange > bottom
	// ЕСЛИ НАД ЛАМПОЙ НЕТ руки, но она была
	if ((nowRange < (top + delta))) {
		if (timing == true) { // если провели рукой и включились таймеры
			if (lightMode == 1) {
				rgbLight(lastValueRGB, lastValueRGB, lastValueRGB); // восстановление оттенка и яркости после индикации возможности переключения света
				followMeLight = false;
			}
			unsigned long  millistart = millis() - timerSwitchStart;
			if (millistart < shortDelay) { // если рукой просто провели
				// ВКЛЮЧЕНИЕ и выключение ночника
				if (lightMode == 0 && switchLock == false) { // если режим ночника
					if (nightLight == false) { // если ночник выключен
						nightLightLevel = 0;
						nightLight = true; // ночник включен
					} else {
						nightLightLevel = nightLightLimit;
						nightLight = false; // ночник выключен
					}
					dimmerDelay = millis(); // запуск таймера интервала увеличения яркости
				}
				// ПЕРЕКЛЮЧЕНИЕ режимов
				if (lightMode > 0) { // если режим светильника
					lightMode = lightMode + 1; // переключение режима
					blinkLight(150);
					if (lightMode == 4) { // если режим радуги
						dimmerDelay = millis(); // запуск таймера интервала изменения яркости
						if (nowRange < 255) { // установка стартового оттенка радуги
							rainbow = nowRange;
							rainbowUp = true;
						} else {
							rainbow = 254;
							rainbowUp = false; // направление радуги
						}
					}
					if (lightMode == 3) { // если режим оттенка - сразу включить оттенок
						colorRGB(valueRGB);
					}
					if (lightMode == 2) {
						if (tempRcvd == true) {
							colorRGB(tempC); // инициализация и восстановление индикации температуры при смене режима
						} else {
							noTemp();
						}
					}
					if (lightMode > 4) { // зацикливание переключения режимов
						lightMode = 1;
					}
				}
			} // < longDelay
		} // timing = true
		timing = false;
		switchLock = false; // сброс блокировки на переключение режимов по длительному удержанию руки над лампой
	} // nowRange < bottom + delta

	if (nightLight == true && nightLightLevel < nightLightLimit) { // если ночник включен, а яркость меньше установленной
		if ((millis() - dimmerDelay) > dimmerStep) { // регулировка яркости с интервалом dimmerStep мс
			nightLightLevel++;
			rgbLight(nightLightLevel, nightLightLevel, nightLightLevel);
			dimmerDelay = millis(); // запуск таймера интервала увеличения яркости
		}
	}
	if (nightLight == false && nightLightLevel > 0) { // если ночник включен, а яркость больше нуля
		if ((millis() - dimmerDelay) > dimmerStep) {
			nightLightLevel--;
			rgbLight(nightLightLevel, nightLightLevel, nightLightLevel);
			dimmerDelay = millis(); // запуск таймера интервала увеличения яркости
		}
	}
	// ИНДИКАЦИЯ температуры
	if (lightMode == 2) {
		if (valueRGB > 0) { // если рука на лампе и пора выключаться
			if (tempRcvd == true) {
				if (tempLock == false) { // если не заблокирована смена оттенка температуры
					colorRGB(tempC); // инициализация и восстановление индикации температуры при смене режима
					tempLock = true; // блокировка смены оттенка до следующего значения температуры
				} else {
					noTemp();
				}
			}
		} else {
			rgbLight(0, 0, 0);
			tempLock = false;
		}
	}
	// РАДУГА
	if (lightMode == 4) {
		if (valueRGB > 0) { // если рука на лампе и пора выключаться
			if ((millis() - dimmerDelay) > rainbowStep) {
				// Serial.print("Rainbow value: ");
				// Serial.println(rainbow);
				if (rainbowUp == true) {
					if (rainbow < 254) {
						rainbow++;
					} else {
						rainbowUp = false;
					}
				}
				if (rainbowUp == false) {
					if (rainbow > 1) {
						rainbow--;
					} else {
						rainbowUp = true;
					}
				}
				colorRGB(rainbow);
				dimmerDelay = millis();
			}
		} else {
			rgbLight(0, 0, 0);
		}
	}
	if (mySwitch.available()) { // прием данных и команд
		int value = mySwitch.getReceivedValue();
		if (value != 0) {
			// ВКЛЮЧЕНИЕ ВМЕСТЕ С ФОНОМ
			if (lightMode == 0) {
				if (mySwitch.getReceivedValue() == 863029) {
					lightMode = 1;
					rgbLight(254, 254, 254);
					switchLock = true;
				}
			}
			// ВЫКЛЮЧЕНИЕ ВМЕСТЕ С ФОНОМ
			if (mySwitch.getReceivedValue() == 863028) {
				rgbLight(0, 0, 0); // выключение света
				mainLight = false; // отключение памяти состояния света при выключении светильника
				backLight = false;
				if (lightMode == 0) {
					lastMode = 1;
				} else {
					lastMode = lightMode; // память последнего режима
				}
				lightMode = 0; // режим ночника
				switchLock = true; // блокировка переключений до следующего взмаха
			}
			if (mySwitch.getReceivedValue() / 100000 == 161) {
				weatherData = mySwitch.getReceivedValue() - 16100000;
				if (weatherData < 10000) { // фильтр от влажности
					if (weatherData > 1000) { // минусовая температура
						if (weatherData > 1250) { // температура ниже -25С
							tempC = 0;
						}
						if (weatherData < 1250 && weatherData > 1150) { // температура -25С -- -15C
							tempC = 15;
						}
						if (weatherData < 1150) { // температура -15C -- 0C
							tempC = 50;
						}
					}  else { // плюсовая температура
						if (weatherData > 300) { // если температура выше +30С
							tempC = 254;
						}
						if (weatherData > 250 && weatherData < 300) { // если температура +25C -- +30С
							tempC = 235;
						}
						if (weatherData > 200 && weatherData < 250) { // если температура +25C -- +30С
							tempC = 170;
						}
						if (weatherData > 150 && weatherData < 200) { // если температура +15C -- +25С
							tempC = 100;
						}
						if (weatherData < 150) { // если температура ниже +15
							tempC = 85;
						}
					}
					tempRcvd = true;
					tempLock = false; // получена температура для отображения
					// tempC = tempC*0.363; // трансляция температуры в шкалу от 0 до 255
					// Serial.print("TempC: ");
					// Serial.println(tempC);
				}
			}
			mySwitch.resetAvailable();
		}
	}
} // loop
void rgbLight(byte redL, byte greenL, byte blueL) {
	analogWrite(redPin, redL);
	analogWrite(greenPin, greenL);
	analogWrite(bluePin, blueL);
}
void colorRGB(int colorV) {
	// КРАСНЫЙ:
	if (colorV < 100) {
		targetR = colorV * 0.05;
	} else {
		targetR = 5 + (colorV - 100) * 1.29;
	}
	// ЗЕЛЕНЫЙ
	if (colorV < 75) { // 0 - 180
		targetG = 2.4 * colorV;
	}
	if (colorV > 75 && colorV < 100) { // 180 - 254
		targetG = 180 + (colorV - 75) * 2.96;
	}
	if (colorV > 100 && colorV < 150) { // 254
		targetG = 254;
	}
	if (colorV > 150 && colorV < 200) { // 254 - 154
		targetG =  254 - (colorV - 150) * 2;
	}
	if (colorV > 200) { // 154 - 0
		targetG = 154 - (colorV - 200) * 2.85;
	}
	// СИНИЙ
	if (colorV < 100) {
		targetB = 254 - colorV * 2.54;
	} else {
		targetB = colorV * 0.05;
	}
	rgbLight(targetR, targetG, targetB);
}
void blinkLight(byte spark) {
	for (byte i = 0; i < 3; i++) {
		rgbLight(0, 0, 0);
		delay(75);
		rgbLight(spark, spark, spark);
		delay(75);
	}
}
unsigned int getRange() {
	byte i;
	unsigned int rangeFinder = 0;
	for (i = 0; i < 100; i++) {
		rangeFinder = rangeFinder + analogRead(rangePin);
	}
	rangeFinder = rangeFinder / 100;
	return rangeFinder;
}
// СВЕТОФОР БЕЗ ТЕМПЕРАТУРЫ
void noTemp() {
	rgbLight(254, 0, 0); // Мигаем разными цветами
	delay(500);
	rgbLight(0, 254, 0);
	delay(500);
	rgbLight(0, 0, 254);
	delay(500);
	blinkLight(150); // Мигаем белым
	lightMode = 3; // Переключаемся в следующий режим
}