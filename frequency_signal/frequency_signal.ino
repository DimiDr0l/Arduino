/* Генератор, частотометр, вольтметр на Ардуино Уно v 2.4
  Распиновка: Энкодер A0,A1 (in)
  Кнопка энкодера A2 (in)
  Кнопка "режим" A3 (in)
  Подсветка 8 (out) (вывод BL дисплея)
  Дисплей nokia 5110 - 13,12,11,10 (out)  
  Выход генератора 0..7 + 9 (out)
  Вход частотометра - 5 (in)
  Входы вольтметра -А4,А5 (in)
  http://arduino.ru/forum/proekty/generator-s-reguliruemoei-chastotoi-na-arduino?page=4#comment-204507
  https://img-fotki.yandex.ru/get/31286/118551841.8/0_dbaf6_2a797110_orig.jpg
*/ 


const PROGMEM uint8_t  sinewave[]= // массив синуса
{
0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9c,0x9f,0xa2,0xa5,0xa8,0xab,0xae,
0xb0,0xb3,0xb6,0xb9,0xbc,0xbf,0xc1,0xc4,0xc7,0xc9,0xcc,0xce,0xd1,0xd3,0xd5,0xd8,
0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xed,0xef,0xf0,0xf2,0xf3,0xf5,
0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfc,0xfd,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfe,0xfd,0xfc,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,
0xf6,0xf5,0xf3,0xf2,0xf0,0xef,0xed,0xec,0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
0xda,0xd8,0xd5,0xd3,0xd1,0xce,0xcc,0xc9,0xc7,0xc4,0xc1,0xbf,0xbc,0xb9,0xb6,0xb3,
0xb0,0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,0x67,0x63,0x60,0x5d,0x5a,0x57,0x54,0x51,
0x4f,0x4c,0x49,0x46,0x43,0x40,0x3e,0x3b,0x38,0x36,0x33,0x31,0x2e,0x2c,0x2a,0x27,
0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x12,0x10,0x0f,0x0d,0x0c,0x0a,
0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x0a,0x0c,0x0d,0x0f,0x10,0x12,0x13,0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
0x25,0x27,0x2a,0x2c,0x2e,0x31,0x33,0x36,0x38,0x3b,0x3e,0x40,0x43,0x46,0x49,0x4c,
0x4f,0x51,0x54,0x57,0x5a,0x5d,0x60,0x63,0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c
};

const PROGMEM uint8_t squarewave[]= //массив меандра
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

const PROGMEM uint8_t trianglewave[]= //массив треугольника
{
0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf1,0xef,0xef,0xeb,0xe9,0xe7,0xe5,0xe3,0xe1,
0xdf,0xdd,0xdb,0xd9,0xd7,0xd5,0xd3,0xd1,0xcf,0xcf,0xcb,0xc9,0xc7,0xc5,0xc3,0xc1,
0xbf,0xbd,0xbb,0xb9,0xb7,0xb5,0xb3,0xb1,0xaf,0xaf,0xab,0xa9,0xa7,0xa5,0xa3,0xa1,
0x9f,0x9d,0x9b,0x99,0x97,0x95,0x93,0x91,0x8f,0x8f,0x8b,0x89,0x87,0x85,0x83,0x81,
0x7f,0x7d,0x7b,0x79,0x77,0x75,0x73,0x71,0x6f,0x6f,0x6b,0x69,0x67,0x65,0x63,0x61,
0x5f,0x5d,0x5b,0x59,0x57,0x55,0x53,0x51,0x4f,0x4f,0x4b,0x49,0x47,0x45,0x43,0x41,
0x3f,0x3d,0x3b,0x39,0x37,0x35,0x33,0x31,0x2f,0x2f,0x2b,0x29,0x27,0x25,0x23,0x21,
0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x11,0x0f,0x0f,0x0b,0x09,0x07,0x05,0x03,0x01
};

const PROGMEM uint8_t sawtoothwave[]= //массив пила1
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

const PROGMEM uint8_t rewsawtoothwave[]= //массив пила2
{
0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4,0xf3,0xf2,0xf1,0xf0,
0xef,0xee,0xed,0xec,0xeb,0xea,0xe9,0xe8,0xe7,0xe6,0xe5,0xe4,0xe3,0xe2,0xe1,0xe0,
0xdf,0xde,0xdd,0xdc,0xdb,0xda,0xd9,0xd8,0xd7,0xd6,0xd5,0xd4,0xd3,0xd2,0xd1,0xd0,
0xcf,0xce,0xcd,0xcc,0xcb,0xca,0xc9,0xc8,0xc7,0xc6,0xc5,0xc4,0xc3,0xc2,0xc1,0xc0,
0xbf,0xbe,0xbd,0xbc,0xbb,0xba,0xb9,0xb8,0xb7,0xb6,0xb5,0xb4,0xb3,0xb2,0xb1,0xb0,
0xaf,0xae,0xad,0xac,0xab,0xaa,0xa9,0xa8,0xa7,0xa6,0xa5,0xa4,0xa3,0xa2,0xa1,0xa0,
0x9f,0x9e,0x9d,0x9c,0x9b,0x9a,0x99,0x98,0x97,0x96,0x95,0x94,0x93,0x92,0x91,0x90,
0x8f,0x8e,0x8d,0x8c,0x8b,0x8a,0x89,0x88,0x87,0x86,0x85,0x84,0x83,0x82,0x81,0x80,
0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x77,0x76,0x75,0x74,0x73,0x72,0x71,0x70,
0x6f,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,0x68,0x67,0x66,0x65,0x64,0x63,0x62,0x61,0x60,
0x5f,0x5e,0x5d,0x5c,0x5b,0x5a,0x59,0x58,0x57,0x56,0x55,0x54,0x53,0x52,0x51,0x50,
0x4f,0x4e,0x4d,0x4c,0x4b,0x4a,0x49,0x48,0x47,0x46,0x45,0x44,0x43,0x42,0x41,0x40,
0x3f,0x3e,0x3d,0x3c,0x3b,0x3a,0x39,0x38,0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x30,
0x2f,0x2e,0x2d,0x2c,0x2b,0x2a,0x29,0x28,0x27,0x26,0x25,0x24,0x23,0x22,0x21,0x20,
0x1f,0x1e,0x1d,0x1c,0x1b,0x1a,0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,
0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,
};


const PROGMEM char musor_mass[]= //массив для подгонки адреса в флэш-памяти
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


volatile int32_t freq=1000;// частота по умолчанию
volatile uint32_t icr=0;  //переменная для управления регистром сравнения таймера1
volatile uint16_t shag=100; //шаг энкодера по умолчанию
volatile uint16_t int_tic=0; 
volatile uint8_t regim=5; //режим генератора по умолчанию
volatile uint8_t monitor_flag; // флаг для вывода на дисплей
volatile uint8_t ad2,ad1,ad0; //служебные байты для функции на ассемблере
volatile uint8_t duty=50; //скважность
volatile uint8_t main_flag=0;//флаг работа в режиме генератора или нет

#define int_on() PCMSK1=(1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11); PCICR=(1<<PCIE1);
//включить прерывание PCINT1, выбор пинов прерывания A1,A2,A3
#define int_off() PCMSK1=0; PCICR=0; //отключить PCINT1
#include <LCD5110_SSVS.h>
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
LCD5110 lcd(10,11,12,13);

// DDS algorithm 
static inline void signalOUT(const uint8_t *signal){ 
   asm volatile(  "eor r18, r18   ;r18<-0"  "\n\t"
    "eor r19, r19   ;r19<-0"  "\n\t"
     "1:"            "\n\t"
      "add r18, %0  ;1 cycle"     "\n\t"
       "adc r19, %1  ;1 cycle"     "\n\t"  
        "adc %A3, %2  ;1 cycle"     "\n\t"
         "lpm      ;3 cycles"  "\n\t"
         "out %4, __tmp_reg__  ;1 cycle" "\n\t"
        "sbis %5, 7   ;1 cycle if no skip" "\n\t"     
       "rjmp 1b    ;2 cycles. Total 10 cycles" "\n\t" 
      :
     :"r" (ad0),"r" (ad1),"r" (ad2),"e" (signal),"I" (_SFR_IO_ADDR(PORTD)), "I" (_SFR_IO_ADDR(PORTB))
    :"r18", "r19" );
    }

void setup(){
lcd.InitLCD();
for (int n=0; n <sizeof(musor_mass); n++ ) {PORTD=musor_mass[n]; }   
PORTD=0; DDRD=0; TCCR1A=0; TCCR1B=0; TIMSK1=0; 
PORTC|=(1<<PC2)|(1<<PC3); //подтяжка кнопок на A2 A3
DDRB|=1<<PB0; // 8 пин -включение подсветки дисплея = выход
main_screen(); //стартовое сообщение и выбор задач
//далее загрузка генератора по умолчанию
TCCR0B=0;  check_regim(); 
int_on();//включить прерывание PCINT1
main_flag=1;
} //end setup


void up_down(boolean x){ // управление регулировками
// static boolean n=0;  if (n=!n){return;} // снять ремарку для энкодеров с двойным щелчком
if (TCCR1B==17 && ICR1<2800 && regim==5){ 
if(x) {if (icr<1000 && shag > 100) shag=100; 
       if (icr<100 && shag > 10) shag=10;
       if (icr<10 && shag > 1) shag=1;          
       icr-=shag;
         if (icr<2) icr=2;    }
         else {  if (icr > 1800 && shag >100) shag =100; icr+=shag ;  } 
  return;   }
if (regim==6){if (ICR1>100){ 
                if (x){if(duty<100) {duty++; }  }
                if (!x){ if(duty>0) {duty--; }} }
              else{ 
                    if (x){if(OCR1A<ICR1) {OCR1A++; }  }         
                    else {if(OCR1A>0) {OCR1A--; }  }         
                    if(OCR1A>ICR1) OCR1A=ICR1-1;
                    duty=(uint32_t)100*OCR1A/ICR1;  }
         return;
  } 
x? freq+=shag : freq-=shag ;  
if (freq < 1) freq=1;
}

        
////////////////////////////////////////////////////////////////        
//****** ПРЕРЫВАНИЕ от кнопок и энкодера*******/////////////////         
 ISR (PCINT1_vect){ 
 int_off();  PORTD=0;
 ///блок для обработки событий не в режиме генератора///
if (main_flag==0) { if ((PINC&(1<<3))==0){ while ((PINC&(1<<3))==0); //вкл. подсветки
                  if ((PINC&(1<<2))==0){ PINB|=1<<PB0; while ((PINC&(1<<2))==0);}
                  int_on(); return; //выходить если не в режиме генератора 
                  }}
 //далее всё в режиме генаратора
 // если нажата кнопка энкодера     
 if ((PINC&(1<<3))==0){ 
        while ((PINC&(1<<3))==0);// подождать до тех пор, когда кнопку отпустят
      //блок вкл/отк подсветки дисплея (8 пин ардуино)
        if ((PINC&(1<<2))==0){ //если после этого нажата кнопка режимов,
         PINB|=1<<PB0; //менять состояние 8 пина
          while ((PINC&(1<<2))==0);// теперь подождать пока отпустят кнопку режим 
          int_on(); return;
          }
     if (regim==6) { regim=5; check_regim(); int_on(); return; }
          switch (shag){ 
                   case 1: shag=10;     break;
                   case 10: shag=100;    break;
                   case 100: shag=1000;   break;
                   case 1000: shag=1; break; 
                  } check_regim(); int_on(); return;
            } //конец блока *если нажата кнопка энкодера*
if ((PINC&(1<<2))==0){ // если нажата кнопка режимов
   PORTD=0;
    while ((PINC&(1<<2))==0);
     regim++; if (regim==7) regim=0; 
     check_regim(); int_on();  return;
      } //конец блока *если нажата кнопка режимов*
// если кнопки не нажимались -значит крутили энкодер:
        up_down( ! (PINC&1)^((PINC&2)>>1) ); //отправить считанное энкодеров в другую функцию
        // если при вращении счёт идёт не в нужную сторону, то (вставить/убрать) символ '!'   up_down(! 
        check_regim(); int_on();
         } //конец функции обработки прерываний от кнопок PCINT1_vect 
////////////////////КОНЕЦ_ПРЕРЫВАНИЕ_от_кнопок_и_энкодера///////////////////////////////


void pwm_gen(){ //настройка таймера1
 uint16_t divider=1; 
  icr = (16000000ul / freq /2 /divider); 
   byte shifts[] = {3,3,2,2};
     for(byte i = 0; i < 4; i++){
        if (icr > 65536) {
           divider <<= shifts[i];
           icr = 16000000ul / freq /2 /divider; }
        else {  TCCR1B = (i+1)|(1<<WGM13);  break;  } }
ICR1=icr-1; 
set_duty();
} //end pwm_gen

    
void loop() {
   if (monitor_flag) { monitor_flag=0; monitor_out(); }
               if (regim <5){ PORTB&= ~(1<<7);
               DDRD=0xFF;//set D port as output
               uint32_t temp=(float)freq /0.095367431640625; 
               ad2=temp>>16; ad1=temp>>8; ad0=temp;
                 switch (regim){ 
                  case 0: signalOUT(sinewave); break;
                   case 1: signalOUT(trianglewave); break;
                    case 2: signalOUT(squarewave); break;
                     case 3: signalOUT(sawtoothwave); break;
                      case 4: signalOUT(rewsawtoothwave);
                       } //end switch
                     DDRD=0; PORTD=0;
              
                } //end if (regim<5)
  }//end loop


void monitor_out(){
String dutystr,stepstr;
dutystr= String("Duty="+ String(duty)+ "%");                
stepstr=String("Step= "+String(shag));
lcd.clrScr();
//Вывод первой строчки
lcd.setFont(SmallFont);
if (freq <10000) {lcd.print("Frequency, Hz",LEFT,0 );  }
if (freq >=10000) {lcd.print("Frequency, kHz",LEFT,0 );  }
//Вывод второй строчки
lcd.setFont(MediumNumbers);
if (freq <10000) { lcd.printNumI(freq, CENTER, 8); }
if (freq >=10000u && freq < 1000000ul ) { lcd.printNumF( ((float)freq/1000),3 ,CENTER, 8);  }
if (freq >=1000000ul ) { lcd.printNumF( ((float)freq/1000),2 ,CENTER, 8);  }
//Вывод третьей строчки
lcd.setFont(SmallFont);
     switch(regim){ 
       case 0: lcd.print("Sinus DDS",CENTER, 32); break;
        case 1: lcd.print("Triangle DDS",CENTER, 32); break;
         case 2: lcd.print("Meandr DDS",CENTER, 32); break;
        case 3: lcd.print("Pila1 DDS",CENTER, 32); break;
       case 4: lcd.print("Pila2 DDS",CENTER, 32); break;
      case 5: lcd.print("PWM Mode",CENTER,32); break;
    case 6: lcd.print("Duty Mode", CENTER, 32);
     }
//Вывод четвёртой строчки
 if (regim==6) lcd.print(dutystr, CENTER, 40);
         else lcd.print(stepstr, CENTER, 40);
}//end monitor out


void set_duty(){
if (regim==6&&ICR1<100) return;
if (regim==5 && ICR1<100){
    OCR1A=ICR1/2; duty=50; 
    return;   }
static uint16_t ocr;
ocr=(uint32_t)ICR1*duty/100;
OCR1A=ocr;
}

void check_regim(){// проверка и установка режимов генератора
if (regim <5){ if (freq > 100000) freq=100000;
    TCCR1B=0;
    TCCR1A=0;  DDRB&=~(1<<DDB1); // 9 pin arduino set Z-mode 
    }// end if regim <5
  if (regim > 4) { 
      TCCR1A=1<<COM1A1; DDRB|=1<<DDB1; // 9 pin set output (pwm out)
      if (TCCR1B==17 && ICR1<2800){ 
              ICR1=icr; freq= (float) 8000000UL/ICR1;  
              set_duty();                      }
       else { pwm_gen(); } 
         }
   PORTB|= 1<<7; 
monitor_flag=1;
}


ISR (TIMER1_OVF_vect){ int_tic++; } // прерывание частотомера

void freq_meter(){
   lcd.clrScr();
   lcd.setFont(SmallFont);
   lcd.print("Freq.counter",LEFT, 0);
  int_on();//включить прерывание PCINT1
  TIMSK1 = 1<<TOIE1;// подключить прерывание
  uint32_t freqm=0;// Переменная измерения частоты
  int_tic=0; TCNT1=0; TIFR1=0;//всё обнулить
while(1){
   TCCR1B=7;//тактировани от входа Т1
   delay(2000); TCCR1B=0;
   freqm= (((uint32_t)int_tic<<16) | TCNT1)/2; //сложить что натикало
   int_tic=0; TCNT1 = 0; 
   lcd.clrScr(); lcd.setFont(SmallFont);
   lcd.print("Freq.counter",LEFT, 0);
   lcd.setFont(MediumNumbers);
   if (freqm <10000) { lcd.printNumI(freqm, CENTER, 8); lcd.setFont(SmallFont); lcd.print("Herz",CENTER, 32 ); }
   if (freqm >=10000u && freqm < 1000000ul ) { lcd.printNumF( ((float)freqm/1000),3 ,CENTER, 8);lcd.setFont(SmallFont);  lcd.print("KiloHerz",CENTER, 32 ); }
   if (freqm >=1000000ul ) { lcd.printNumF( ((float)freqm/1000000ul),3 ,CENTER, 8); lcd.setFont(SmallFont); lcd.print("MegaHerz",CENTER, 32 ); }    
   }  
 }

//////////////////////////////////////////////////////////////////////
void volt_meter() {
ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
ADCSRB=0; DIDR0=48; int_on();
float ain,vcc;  String ainstr,vccstr;
lcd.clrScr(); lcd.setFont(SmallFont);
lcd.print("Volt meter",LEFT, 0);
while(1){
ADMUX = (1<<REFS0)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1); //Vcc measure
delay(1); vcc=(float)(1.1*65472) / analog_func();
vccstr=String("Vcc= "+String(vcc,3)+" v  ");
lcd.print(vccstr,LEFT, 40);
ADMUX = (1<<REFS0)|(1<<MUX2); //A4 measure
delay(1); ain= analog_func()*vcc /65472 ; 
ainstr=String("Ain4= "+String(ain,3)+" v  ");
lcd.print(ainstr,LEFT, 16);
ADMUX = (1<<REFS0)|(1<<MUX2)|(1<<MUX0); //A5 measure
delay(1); ain= analog_func() *vcc /65472 ; 
ainstr=String("Ain5= "+String(ain,3)+" v  ");
lcd.print(ainstr,LEFT, 24);
  };//end while
}//end volt meter

uint32_t analog_func(){
uint32_t adc_buff=0;
for (int n=0; n<=4095; n++ ) {
    ADCSRA |= (1<<ADSC);  
    while (bit_is_set(ADCSRA,ADSC));
    adc_buff += ADC;
   } return adc_buff>>=6;
}

/////////////////////////////////////////////////////////////////
void main_screen(){
static boolean flag_exit =0; uint8_t n=84;
lcd.setFont(SmallFont);
lcd.print("Freq.counter >",LEFT, 8 );
lcd.print("**************",LEFT,24);
lcd.print("  Volt meter >",LEFT, 40 );
while(flag_exit==0) { //ждать нажатия кнопки режимов.
delay(350); lcd.clrRow(3,n); n-=6;  
if (n==0) flag_exit=1;
if ((PINC&(1<<2))==0){ // если нажата кнопка режимов
                      while ((PINC&(1<<2))==0);
                      freq_meter();   }
if ((PINC&(1<<3))==0){ // если нажата кнопка энкодера
                      while ((PINC&(1<<3))==0);
                      volt_meter();   }
        }// end while
}// end main_screen