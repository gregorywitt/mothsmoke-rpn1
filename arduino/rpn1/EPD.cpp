// This is for driving a GDEY0213B74 2.13 Inch 250x122 Eink Display w/ SSD1680 chipset
// the best clues are at https://github.com/ZinggJM/GxEPD2
// with some more clues at https://www.good-display.com/product/391.html

#include "EPD.h"

EPD::EPD(byte _dc_pin, byte _reset_pin, byte _busy_pin, bool (*_poll)()) {
  dc_pin = _dc_pin;
  reset_pin = _reset_pin;
  busy_pin = _busy_pin;

  //abort_flag = _abort_flag;
  poll = _poll;
}

EPD::~EPD() {
}

void EPD::begin() {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  
  pinMode(dc_pin, OUTPUT);
  pinMode(busy_pin, INPUT);

  SPI.begin();

  pinMode(reset_pin, OUTPUT);
  hardwareReset();  
  waitWhileBusy();

  powerDown();

  // fill ram with white
  powerUp();

  write_command(SSD1680_WRITE_RAM1);
  for(int i = 0; i < MEM_SIZE; i++) write_data(0xFF);

  write_command(SSD1680_WRITE_RAM2);
  for(int i = 0; i < MEM_SIZE; i++) write_data(0xFF);

  powerDown();
}

void EPD::fillScreen(byte d) {
  powerUp();

  //write_command(SSD1680_WRITE_BORDER);
  //write_data(0x05);  

  write_command(SSD1680_WRITE_RAM1);
  for(int i = 0; i < MEM_SIZE; i++) write_data(d);

  write_command(SSD1680_WRITE_RAM2);
  for(int i = 0; i < MEM_SIZE; i++) write_data(d);

  fullUpdate();
}

void EPD::fillScreenFast(byte d) {
  unsigned long startTime= millis();
  powerUp();

  write_command(SSD1680_WRITE_RAM1);
  for(int i = 0; i < MEM_SIZE; i++) write_data(d);
  
  updatePartial();

  write_command(SSD1680_WRITE_RAM2);
  for(int i = 0; i < MEM_SIZE; i++) write_data(d);
  write_command(SSD1680_WRITE_RAM1);
  for(int i = 0; i < MEM_SIZE; i++) write_data(d);

  //Serial.print("full screen write time: ");
  //Serial.println(millis() - startTime);
}

void EPD::writeBlock(byte bytes[], int numBytes, byte xPos, byte yPos, byte h, byte which_ram){

  byte w = (numBytes * 8) / h;

  // determine addresses
  int xAddrStart, xAddrEnd, yAddrStart, yAddrEnd;
  xAddrStart = SCREEN_WIDTH - xPos;
  yAddrStart = yPos / 8;
  yAddrEnd = yAddrStart + h / 8 - 1;
  xAddrStart = xAddrStart - 1;
  xAddrEnd = xAddrStart + w - 1;
  
  // write addresses
  write_command(SSD1680_SET_RAMXPOS);
  write_data(yAddrStart);
  write_data(yAddrEnd);

  write_command(SSD1680_SET_RAMYPOS);
  write_data(xAddrStart % 256);
  write_data(xAddrStart / 256);
  write_data(xAddrEnd % 256);
  write_data(xAddrEnd / 256);

  write_command(SSD1680_SET_RAMXCOUNT);
  write_data(yAddrStart);

  write_command(SSD1680_SET_RAMYCOUNT);
  write_data(xAddrStart % 256);
  write_data(xAddrStart / 256);

  // write image to ram
  write_command(which_ram);
  for (int i = 0; i < numBytes; i++) write_data(pgm_read_byte_near(bytes + i));
}

void EPD::filledRect(bool white, int numBytes, byte xPos, byte yPos, byte h, byte which_ram){
  
  byte w = (numBytes * 8) / h;

  // determine addresses
  int xAddrStart, xAddrEnd, yAddrStart, yAddrEnd;
  xAddrStart = SCREEN_WIDTH - xPos;
  yAddrStart = yPos / 8;
  yAddrEnd = yAddrStart + h / 8 - 1;
  xAddrStart = xAddrStart - 1;
  xAddrEnd = xAddrStart + w - 1;
  
  // write addresses
  write_command(SSD1680_SET_RAMXPOS);
  write_data(yAddrStart);
  write_data(yAddrEnd);

  write_command(SSD1680_SET_RAMYPOS);
  write_data(xAddrStart % 256);
  write_data(xAddrStart / 256);
  write_data(xAddrEnd % 256);
  write_data(xAddrEnd / 256);

  write_command(SSD1680_SET_RAMXCOUNT);
  write_data(yAddrStart);

  write_command(SSD1680_SET_RAMYCOUNT);
  write_data(xAddrStart % 256);
  write_data(xAddrStart / 256);

  // write image to ram
  write_command(which_ram);
  byte data = 0;
  if (white) data = 255;
  for (int i = 0; i < numBytes; i++) write_data(data);
}

void EPD::fullUpdate() {
  // display update sequence
  write_command(SSD1680_DISP_CTRL2);
  write_data(0xF7);

  //unsigned long start_time = millis();
  write_command(SSD1680_MASTER_ACTIVATE);
  waitWhileBusy();
  //Serial.print("update ms: ");
  //Serial.println(millis() - start_time);
}

void EPD::updatePartial() {
  write_command (SSD1680_DISP_CTRL2);
  write_data (0xFC); // 0xFC is faster than 0xFF
  
  unsigned long start_time = millis();
  write_command (SSD1680_MASTER_ACTIVATE);
  bool abort = false;
  while (millis() < start_time + MIN_UPDATE_TIME) if (poll()) abort = true;
  if (abort){
    hardwareReset();
    //Serial.println("abort a");
    return;
  }
  while (digitalRead(busy_pin)){
    if (poll()){
      hardwareReset();
      //Serial.println("abort w");
      return;
    }
  }
  //Serial.print("update ms: ");
  //Serial.println(millis() - start_time);
  return;
}

void EPD::hardwareReset() {
  unsigned long st = millis();
  digitalWrite (reset_pin, LOW);
  while (millis() < st + 10) poll();

  // bring out of reset
  st = millis();
  digitalWrite (reset_pin, HIGH);
  while(millis() < st + 10) poll();
}

void EPD::powerUp() {
  hardwareReset();

  waitWhileBusy();

  // init_codes
  write_command (SSD1680_SW_RESET);
  waitWhileBusy();
  
  // Set display size and driver output control
  write_command (SSD1680_DRIVER_CONTROL);
  write_data ((SCREEN_WIDTH - 1) % 256);
  write_data ((SCREEN_WIDTH - 1) / 256);
  write_data (0x00);

  write_command(SSD1680_DATA_MODE);     // Ram data entry mode
  write_data(0x01); // was 0x03

  // set ram x start/end position
  write_command(SSD1680_SET_RAMXPOS);
  write_data(0x00);
  write_data(SCREEN_HEIGHT / 8 - 1);

  // Set ram Y start/end postion
  write_command(SSD1680_SET_RAMYPOS);
  write_data((SCREEN_WIDTH - 1) % 256);
  write_data((SCREEN_WIDTH - 1) / 256);
  write_data(0x00);
  write_data(0x00);

  write_command(SSD1680_WRITE_BORDER);  // border color
  write_data(0x05);

  write_command(SSD1680_DISP_CTRL1);
  write_data(0x00);
  write_data(0x80);

  write_command(SSD1680_TEMP_CONTROL);
  write_data(0x80);

  write_command(SSD1680_SET_RAMXCOUNT);
  write_data(0x00);
  
  write_command(SSD1680_SET_RAMYCOUNT);
  write_data((SCREEN_WIDTH - 1) % 256);
  write_data((SCREEN_WIDTH - 1) / 256);

  waitWhileBusy();
}

void EPD::powerDown() {
  write_command(SSD1680_DEEP_SLEEP);
  write_data(0x01);
}

void EPD::waitWhileBusy() {
  while (digitalRead(busy_pin)){
    //pollButton();
  }
}

void EPD::write_command(uint8_t c) {
  digitalWrite(dc_pin, LOW);
  SPI.transfer(c);
}

void EPD::write_data(uint8_t d) {
  digitalWrite(dc_pin, HIGH);
  SPI.transfer(d);
}
