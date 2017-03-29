void SSDP_init(void) {

  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("SMART-ROOM");
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("/");
  SSDP.setModelName("SMART-ROOM");
  SSDP.setModelNumber("000000000001");
  SSDP.setModelURL("http://esp8266-arduinoide.ru/smart-room/");
  SSDP.setManufacturer("Tretyakov Sergey");
  SSDP.setManufacturerURL("http://www.esp8266-arduinoide.ru");
  SSDP.begin();
}
