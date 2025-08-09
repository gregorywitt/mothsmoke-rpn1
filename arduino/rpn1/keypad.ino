#include "globals.h"

void processKeypress(byte keyCode){
  //Serial.println("processing input");
  //Serial.print("keycode: ");
  //Serial.println(keyCode);
  //Serial.print("cursor pos: ");
  //Serial.println(cursorPos);

  updateX = true;
  updateY = true;

  if (keyCode == 28){ menuBar.currentMenu[0].action(); return; } // SIN KEY
  if (keyCode == 27){ menuBar.currentMenu[1].action(); return; } // COS KEY
  if (keyCode == 26){ menuBar.currentMenu[2].action(); return; } // TAN KEY
  if (keyCode == 25){ menuBar.currentMenu[3].action(); return; } // MENU KEY
  
  if (keyCode == 4){ do_enter(); return; }
  if (keyCode == 0){ do_add(); return; }
  if (keyCode == 5){ do_subtract(); return; }
  if (keyCode == 10){ do_multiply(); return; }
  if (keyCode == 15){ do_divide(); return; }
  if (keyCode == 14){ do_swap(); return; }
  if (keyCode == 19){ do_rotate(); return; }

  updateY= false;

  if (keyCode == 23){ do_inverse(); return; }  
  if (keyCode == 22){ do_square_root(); return; }  
  if (keyCode == 21){ do_square(); return; }
  if (keyCode == 1) { do_negative(); return; }     // +/- KEY
  if (keyCode == 24){ do_pi(); return; }
  if (keyCode == 20){ do_delete(); return; }
  
  // SOME NUMBER
  if(editing == false){
    if(!editing){
      stack[3] = stack[2];
      stack[2] = stack[1];
      stack[1] = stack[0];
    }
    stack[0] = fp64_sd(0.0);
    inputText[0] = '\0';
    cursorPos = 0;
    updateY = true;
  }
  editing = true;
  //Serial.println("number entered");
  if (cursorPos == maxStringLength) return;

  if (keyCode == 3) { inputText[cursorPos] = '0'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 8) { inputText[cursorPos] = '1'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 7) { inputText[cursorPos] = '2'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 6) { inputText[cursorPos] = '3'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 13){ inputText[cursorPos] = '4'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 12){ inputText[cursorPos] = '5'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 11){ inputText[cursorPos] = '6'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 18){ inputText[cursorPos] = '7'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 17){ inputText[cursorPos] = '8'; cursorPos++; inputText[cursorPos] = '\0'; }
  if (keyCode == 16){ inputText[cursorPos] = '9'; cursorPos++; inputText[cursorPos] = '\0'; }
  
  // don't add more than one decimal point
  if (keyCode == 2) {
    bool hasPoint = false;
    if (cursorPos > 0){
      for(byte i = 0; i < cursorPos - 1; i++) if (inputText[i] == '.') hasPoint = true; 
    }
    if (!hasPoint){ inputText[cursorPos] = '.'; cursorPos++; inputText[cursorPos] = '\0'; }
  }

  //Serial.print("input text: ");
  //Serial.println(inputText);
  //Serial.print(" cursor pos: ");
  //Serial.println(cursorPos);
  
  /*
  for (int i = 0; i < 4; i++){
    Serial.print(i);
    Serial.print(": ");
    Serial.println(fp64_to_string(stack[i], 16, 10));
  }
  */
}

bool scan_keypad(){
  for (byte col = 0; col < num_cols; col++){
    for (byte row = 0; row < num_rows; row++){
      // energize one row at a time
      for (byte i = 0; i < num_rows; i++) digitalWrite(row_pins[i], HIGH);
      digitalWrite(row_pins[row], LOW);
      byte key = row * num_cols + col;
      if (!digitalRead (col_pins[col]) && !keyStates[key] && millis() > keyChangeTime + debounceTime){
        // new press detected
        keyStates[key] = true;
        keyChangeTime = millis();
        addKeypress (key);
        numKeypresses++;
        return true;
      }
      if (keyStates[key] && digitalRead (col_pins[col]) && millis() > keyChangeTime + debounceTime){
        // key release detected
        keyStates[key] = false;
        keyChangeTime = millis();
      }
    }
  }
  return false;
}

void addKeypress(byte key){
  keyBuffer[keyBufferIndex] = key;
  keyBufferIndex++;
  if (keyBufferIndex == keyBufferSize) keyBufferIndex = 0;
}
