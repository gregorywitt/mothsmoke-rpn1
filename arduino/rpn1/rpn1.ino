// Mothsmoke RPN-1 Calculator
// use Leonardo bootloader

#include <SPI.h>
#include <EEPROM.h>
#include "src/LowPower/LowPower.h"
#include "src/fp64lib/fp64lib.h"

#include "globals.h"
#include "EPD.h"
#include "bitmaps.h"
#include "battery_indicator.h"
#include "mode_indicator.h"
#include "menu_bar.h"

// EPD calls scan_keypad so it can abort refresh on keypress
EPD epd(EPD_DC_PIN, EPD_RESET_PIN, EPD_BUSY_PIN, &scan_keypad);

ModeIndicator modeIndicator;
BatteryIndicator battIndicator;
MenuBar menuBar;

void setup() {
  pinMode (POWER_ENABLE_PIN, OUTPUT);
  // hold power pin high to keep power on
  digitalWrite (POWER_ENABLE_PIN, HIGH);

  pinMode (POWER_BUTTON_PIN, INPUT);

  pinMode (LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode (BATT_CHARGING_PIN, INPUT_PULLUP);
  pinMode (BATT_FULL_PIN, INPUT_PULLUP);

  for (byte i = 0; i < num_cols; i++) pinMode(col_pins[i], INPUT_PULLUP);
  for (byte i = 0; i < num_rows; i++) pinMode(row_pins[i], OUTPUT);
  for (byte i = 0; i < num_rows; i++) digitalWrite(row_pins[i], LOW);
  
  for (byte i = 0; i < 30; i++) keyStates[i] = false;
  for (byte i = 0; i < keyBufferSize; i++) keyBuffer[i] = 255;
  keyChangeTime = 0;

  analogReference(INTERNAL); // 2.56V internal reference
  
  // ignore power button bounce
  delay(debounceTime);
  while(powerButtonPressed());
  delay(debounceTime); 

  // flash led to indicate restart
  /*
  for(byte i = 0; i < 5; i++){
    digitalWrite(LED_PIN, i % 2 == 0);
    delay(50);
  }
  */
  
  // if usb connected
  if (UDADDR & _BV(ADDEN)){
    Serial.begin(9600);
    // sometimes this gets run even if usb is disconnected?
    // so use a timeout just in case
    while(!Serial && millis() < 3000);
    if(Serial) Serial.println("\nStart.");
    else Serial.end();
  }

  epd.begin();
  //Serial.println("epd initialized");

  stack[0] = fp64_sd(0.0);
  stack[1] = fp64_sd(0.0);
  stack[2] = fp64_sd(0.0);
  stack[3] = fp64_sd(0.0);
  
  inputText[0] = '\0';

  // load mode
  modeIndicator.deg = EEPROM.read(EEPROM_MODE_ADDR);

  menuBar.blankMenu[0].bitmapOffset = -1;
  menuBar.blankMenu[1].bitmapOffset = -1;
  menuBar.blankMenu[2].bitmapOffset = -1;
  menuBar.blankMenu[3].bitmapOffset = -1;
  menuBar.inverseMenu[0].bitmapOffset = asinBitmap;
  menuBar.inverseMenu[1].bitmapOffset = acosBitmap;
  menuBar.inverseMenu[2].bitmapOffset = atanBitmap;
  menuBar.inverseMenu[3].bitmapOffset = moreBitmap;
  menuBar.moreMenu[0].bitmapOffset = inToMmBitmap;
  menuBar.moreMenu[1].bitmapOffset = MmToInBitmap;
  if (modeIndicator.deg) menuBar.moreMenu[2].bitmapOffset = radBitmap;
  else menuBar.moreMenu[2].bitmapOffset = degBitmap;
  menuBar.moreMenu[3].bitmapOffset = exitBitmap;

  menuBar.blankMenu[0].action = do_sin;
  menuBar.blankMenu[1].action = do_cos;
  menuBar.blankMenu[2].action = do_tan;
  menuBar.blankMenu[3].action = inverse;
  menuBar.inverseMenu[0].action = do_asin;
  menuBar.inverseMenu[1].action = do_acos;
  menuBar.inverseMenu[2].action = do_atan;
  menuBar.inverseMenu[3].action = more;
  menuBar.moreMenu[0].action = inToMm;
  menuBar.moreMenu[1].action = mmToIn;
  if (modeIndicator.deg) menuBar.moreMenu[2].action = rad_mode;
  else menuBar.moreMenu[2].action = deg_mode;
  menuBar.moreMenu[3].action = exit_menu;

  // clear screen if reset forced by user or upload
  if(EEPROM.read(EEPROM_SHUTDOWN_NORMAL_ADDR) == false) epd.fillScreen(255);

  // clear normal shutdown flag
  EEPROM.write(EEPROM_SHUTDOWN_NORMAL_ADDR, false);
  
  updateDisplay();

}

void loop(){
  digitalWrite (LED_PIN, HIGH);

  scan_keypad();

  if (numKeypresses > 0 || battIndicator.checkBattery() || battIndicator.updateNeeded || menuBar.updateNeeded || modeIndicator.updateNeeded){
    // rewind buffer to first unprocessed keypress
    for (byte i = 0; i < numKeypresses; i++){
      keyBufferIndex--;
      if (keyBufferIndex == 255) keyBufferIndex = keyBufferSize - 1;
    }

    // process keypresses
    while (numKeypresses > 0){
      byte key = keyBuffer[keyBufferIndex];
      processKeypress (key);
      keyBufferIndex++;
      if (keyBufferIndex == keyBufferSize) keyBufferIndex = 0;
      numKeypresses--;
    }
    //Serial.println (keyCode);
    updateDisplay();
  }

  if (usbPowerConnected()) return;
  if (numKeypresses > 0) return;


  // when running on battery:
  digitalWrite (LED_PIN, LOW);

  // prepare idle loop
  bool idle = true;
  // set up interrupts to enable detect of any button press
  for (byte i = 0; i < num_rows; i++) digitalWrite (row_pins[i], LOW);
  for(byte i = 0; i < num_cols; i++) attachInterrupt (digitalPinToInterrupt(col_pins[i]), doNothing, LOW);
  
  // stay idle if screen doesn't need updating
  while (idle){
    // sleep for one second or until keypress
    // sleep current is ~ 0.85mA
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

    // every second, do:
    numSecsAsleep++;

    // wake if key pressed, and reset sleep timer
    for (byte i = 0; i < num_cols; i++) if (!digitalRead(col_pins[i])){
      idle = false;
      numSecsAsleep = 0;
    }
    
    // wake if battery indicator needs updating
    if (battIndicator.checkBattery()) idle = false;

    // shut down if power button held
    if (powerButtonPressed){
      unsigned long pressStartTime = millis();
      while (powerButtonPressed()) if (millis() > pressStartTime + 1000) shutdown();
    }

    // remove ghosting if slept for a while
    //if (numSecsAsleep == 100 * 60) {
    //  refreshDisplay();
    //  idle = false; // why is this needed?
    //}

    // hide menu bar if timeout
    /*
    if (menuBar.showing && numSecsAsleep > menuBar.timeShown + menuBar.timeout){
      menuBar.hide();
      idle = false;
    }
    */

    // shut down if slept for a long time
    byte hours_before_shutdown = 3;
    if (numSecsAsleep == hours_before_shutdown * 60 * 60) shutdown();

    // or a short time if battery is low
    if (battIndicator.battIndicator == BATT_LOW && numSecsAsleep == 5 * 60) shutdown();

    // wake if usb connected
    if (usbPowerConnected()){
      idle = false;
      battIndicator.show(0);
    }

    // clean up if exiting idle loop
    if (!idle){
      for(byte i = 0; i < num_cols; i++) detachInterrupt(digitalPinToInterrupt(col_pins[i]));
    }
  }
}

bool powerButtonPressed(){
  // externally pulled down
  bool state= digitalRead (POWER_BUTTON_PIN);
  //digitalWrite(LED_PIN, state);
  return state;
}

void shutdown(){
  digitalWrite(LED_PIN, HIGH);
  // set normal shutdown flag
  EEPROM.write(EEPROM_SHUTDOWN_NORMAL_ADDR, true);
  EEPROM.write(EEPROM_MODE_ADDR, modeIndicator.deg);
  epd.fillScreen(0xFF);
  epd.powerDown();
  digitalWrite(POWER_ENABLE_PIN, LOW);
}

bool usbPowerConnected(){
  return ( battIndicator.battCharging() || battIndicator.battFull() );
}

void doNothing(){} // do nothing on interrupt
