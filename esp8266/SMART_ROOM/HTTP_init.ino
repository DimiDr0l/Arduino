void HTTP_init(void) {
  // Включаем работу с файловой системой
  FS_init();
  
  // SSDP дескриптор
  HTTP.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(HTTP.client());
  });
  //Создание ответа
  HTTP.on("/Reley", handle_Reley); // обрашение к реле через web интерфейс
  HTTP.on("/irc", irControlweb);   // обрашение к ИК через web интерфейс
  // Добавляем функцию Update для перезаписи прошивки
  update();
  // Запускаем HTTP сервер
  HTTP.begin();
}

void irControlweb(){
  int state = HTTP.arg("volume").toInt();
  irsend.sendNEC(state, 36);
  handleFileRead("/pult.htm");
  }



