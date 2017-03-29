
#include "LiquidCrystal.h" // дисплей подключить к А4 А5
#include "MenuBackend.h"    //MenuBackend library - copyright by Alexander Brevig
#include "InfraredReceiver.h"
#include "Wire.h"

const int buttonLeft = 31365;	// влево
const int buttonRight = 28815;	// вправо
const int buttonUp = 32002;		// вверх
const int buttonEnter = 24480;	// ок
const int buttonDown = 32385;	//вниз
int key;//pressed key

LiquidCrystal lcd(0);	// LiquidCrystal display with:

InfraredReceiver infraredReceiver; //IRF

//Menu variables
MenuBackend menu = MenuBackend(menuUsed,menuChanged);
//initialize menuitems
//инициализации пунктов меню
MenuItem menu1Item1 = MenuItem("menu1_1");	//меню температуры
MenuItem menu1Item2 = MenuItem("menu1_2");	//меню влажности
MenuItem menu1Item3 = MenuItem("menu1_3");	//настройки
MenuItem menuItem3SubItem1 = MenuItem("podmenu1_3_1");	//влажность земли
MenuItem menuItem3SubItem1SubItem1 = MenuItem("podmenu1_3_1_1");	//настройа влажности

MenuItem menuItem3SubItem2 = MenuItem("podmenu1_3_2");	//темепратура воздуха
MenuItem menuItem3SubItem2SubItem2 = MenuItem("podmenu1_3_2_1");	//настройка температуры

void setup()
{

  lcd.begin(16, 2);
  infraredReceiver.begin();

  //configure menu
  //настроить меню
  menu.getRoot().add(menu1Item1);
  menu1Item1.addRight(menu1Item2).addRight(menu1Item3);
  menu1Item3.add(menuItem3SubItem1).addRight(menuItem3SubItem2);
  //menu1Item3.add(menuItem2SubItem1).addRight(menuItem2SubItem2).addRight(menuItem3SubItem3);
  menu.toRoot();
  lcd.setCursor(0,0);  
  lcd.print("www.coagula.org");

}  // setup()...


void loop()
{

  //readButtons();  //I splitted button reading and navigation in two procedures because 
  //Я разделил чтения и навигации кнопки в двух процедур, поскольку
  navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)
  //В некоторых ситуациях я хочу использовать кнопки для других целей (например, для изменения настроек)

}


void menuChanged(MenuChangeEvent changed){

  MenuItem newMenuItem=changed.to; //get the destination menu

  lcd.setCursor(0,1); //set the start position for lcd printing to the second row

  if(newMenuItem.getName()==menu.getRoot()){
    lcd.print("Root menu      ");
  }
  else if(newMenuItem.getName()=="menu1_1"){
    lcd.print("menu1_1          ");
  }
  else if(newMenuItem.getName()=="menu1_2"){
    lcd.print("menu1_2        ");
  }
  else if(newMenuItem.getName()=="menu1_3"){
    lcd.print("menu1_3        ");
  }
  else if(newMenuItem.getName()=="podmenu1_3_1"){
    lcd.print("podmenu1_3_1          ");
  }
  else if(newMenuItem.getName()=="podmenu1_3_2"){
    lcd.print("podmenu1_3_2   ");
  }
  else if(newMenuItem.getName()=="podmenu1_3_1_1"){
    lcd.print("podmenu1_3_1_1   ");
  }
  else if(newMenuItem.getName()=="podmenu1_3_2_1"){
    lcd.print("podmenu1_3_2_1   ");
  }

}

void menuUsed(MenuUseEvent used){
  lcd.setCursor(0,0);  
  lcd.print("Vy ispolzovali        ");
  lcd.setCursor(0,1); 
  lcd.print(used.item.getName());
  delay(3000);  //delay to allow message reading - чтобы задержать чтении сообщение
  lcd.setCursor(0,0);  
  lcd.print("www.coagula.org");
  menu.toRoot();  //back to Main
}



void navigateMenus() {
  key = infraredReceiver.read();
    MenuItem currentMenu=menu.getCurrent();

  switch (key){
  case buttonEnter:
    if(!(currentMenu.moveDown())){  //if the current menu has a child and has been pressed enter then menu navigate to item below
      //Если в текущем меню есть подменю, и была нажата, то введите меню перейдите к пункту ниже
      menu.use();
    }
    else{  //otherwise, if menu has no child and has been pressed enter the current menu is used
      //Иначе, если меню не имеет подменю и была нажата Enter текущее меню используется
      menu.moveDown();
    } 
    break;
  case buttonUp:
    menu.toRoot();  //back to main - Вернуться на главную
    break;
  case buttonRight:
    menu.moveRight();
    break;      
  case buttonLeft:
    menu.moveLeft();
    break;      
  }

  //lastButtonPushed=0; //reset the lastButtonPushed variable
  //сбросить переменную lastButtonPushed
}

