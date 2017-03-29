#include "adafruit_gfx.h"
#include "adafruit_pcd8544.h"
 
#define VoltPin A0  // Voltage pin
#define CurrPin A1  // Current pin
 
float kVD = 3;    // Коэффициент делителя напряжения
float kI = 2.4;   // Коэффициент для пересчета тока нагрузки - в нашем случае значение резистора Rg
 
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);
 
int VoltageValue; 
int CurrentValue;
float voltage;
float current;
 
void setup()   {
  display.begin();
  display.setContrast(40);    // установка контрастности дисплея
  delay(1000);
  display.clearDisplay();     // очистка экрана и буфера
  display.setTextSize(1);     // установка размера текста
  display.setTextColor(BLACK);
  delay(1000);
}
 
void loop() {
  VoltageValue = analogRead(VoltPin);               // считываем значение аналогового входа (напряжение)
  CurrentValue = analogRead(CurrPin);               // считываем значение аналогового входа (ток)
  voltage = VoltageValue * (5.0 / 1023.0) * kVD;    // расчет напряжения
  current = (CurrentValue * (5.0 / 1023.0)) / kI;   // расчет тока
   
  display.clearDisplay();              // очистка экрана и буфера
  display.setCursor(0,0);              // установка курсора в позицию 0,0
   
  display.print("Voltage=");           // вывод надписи
  display.println(voltage);            // вывод значения напряжения с переводом строки
  display.println();                   // перевод строки
   
  display.print("Current=");
  display.println(current);
  display.println();  
   
  display.print("Power=");
  display.println(current * voltage);  // расчет мощности
   
  display.display();                   // вывод данных на дисплей
   
  delay(500);                          // задержка 500 мс
}
