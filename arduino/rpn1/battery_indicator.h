
#define BATT_LEVEL_PIN A3
#define BATT_CHARGING_PIN 10
#define BATT_FULL_PIN 11

#define BATT_FULL 1
#define BATT_CHARGING 2
#define BATT_LOW 3
#define BATT_OK 4

// see end of this file to generate this
byte dischargeCurve[] = {86,81,78,76,74,73,72,71,70,69,68,66,65,64,63,61,60,59,58,57,56,55,55,
                          54,53,52,51,50,50,49,48,46,45,44,43,41,40,39,38,37,36,35,33,33,32,31,
                          30,29,28,28,27,26,25,25,24,23,22,21,20,19,18,17,15,14,12,10,7,1,0};


class BatteryIndicator{
  public:
  
  bool updateNeeded;
  int battIndicator;
  static const int lowBattThreshold = 680; // 3.30v
  byte currentBattPercent;

  BatteryIndicator(){
    updateNeeded = true;
    battIndicator = BATT_OK;
    currentBattPercent = battPercent();
  }
  
  // return true if battery indicator needs updating
  bool checkBattery(){
    bool ok = true;
    if (battCharging()){
      ok = false;
      if (battIndicator != BATT_CHARGING){
        //Serial.println("batt charging");
        battIndicator = BATT_CHARGING;
        updateNeeded = true;
      }
    }
    else if (battFull()){
      ok = false;
      if (battIndicator != BATT_FULL){
        //Serial.println("batt full");
        battIndicator = BATT_OK;
        updateNeeded = true;
      }
    }
    else if (battLow()){
      ok = false;
      if (battIndicator != BATT_LOW){ 
        //Serial.println("batt low");
        battIndicator = BATT_LOW; 
        updateNeeded = true;
      }
    }
    else if (ok && battIndicator != BATT_OK){
      //Serial.println("batt ok");
      battIndicator = BATT_OK;
      updateNeeded = true;
    }
    else if ( abs(battPercent() - currentBattPercent) > 4 ){
      currentBattPercent = battPercent();
      updateNeeded = true;
    }

    return updateNeeded;
  }

  bool battCharging(){
    return !digitalRead(BATT_CHARGING_PIN);
  }

  bool battFull(){
    return !digitalRead(BATT_FULL_PIN);
  }

  bool battLow(){
    return analogRead(BATT_LEVEL_PIN) <= lowBattThreshold;
  }

  byte battPercent(){    
    // (analog pin reading - threshold) / 2    
    int reading = (analogRead (BATT_LEVEL_PIN) - lowBattThreshold) / 2;
    int curveIdx = 0;
    for(curveIdx = 0; curveIdx < sizeof(dischargeCurve); curveIdx++) if (dischargeCurve[curveIdx] < reading) break;
    int percent = map (curveIdx, 0, sizeof(dischargeCurve), 100, 0);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
  }

  void show(unsigned long t){
    updateNeeded = true;
  }

};


  /*
  
  // battery discharge curve recording:
  // ( w/ 50 Ohm resistor across batt )
  
  int i = 0;
  while (!statusBar.battLow()){
    byte batt_level_record = (analogRead(BATT_LEVEL_PIN) - statusBar.lowBattThreshold) / 2;
    EEPROM.write (i,  batt_level_record);
    stack[1] = fp64_sd(i);
    stack[0] = fp64_sd(batt_level_record);
    updateX = true;
    updateY = true;
    editing = false;
    updateDisplay();
    delay ((long)1000 * 60 * 5); // 5 minutes
    i++;
  }

  // dump discarge curve from EEPROM:
  for(int i = 0; i < EEPROM.length(); i++){
    Serial.print (EEPROM.read(i));
    Serial.print (",");
  }
  Serial.println();

  */
