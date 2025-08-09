#include "globals.h"

void updateDisplay(){
  //Serial.print("display updating: ");
  //if (editing) Serial.print("edit ");
  //if (updateX) Serial.print("X ");
  //if (updateY) Serial.print("Y");
  //Serial.println();

  // copy everything that might get updated so nothing changes mid-update

  char xStr[maxStringLength];
  strcpy( xStr, format_string( fp64_to_string(stack[0], 16, 10) ) );
  char yStr[maxStringLength];
  strcpy( yStr, format_string( fp64_to_string(stack[1], 16, 10) ) );
  
  bool updateMenu = menuBar.updateNeeded;
  int menuOffsets[4];
  for (byte i = 0; i < 4; i++) menuOffsets[i] = menuBar.currentMenu[i].bitmapOffset;

  if (fp64_isinf(stack[0]) || fp64_isnan(stack[0])) xStr[0] = 'N';
  if (fp64_isinf(stack[1]) || fp64_isnan(stack[1])) yStr[0] = 'N';

  byte xx = 10;
  byte xy = xRegPos;
  byte yx = 10;
  byte yy = yRegPos;
  
  // need to write data three times because of EPD details
  // EPD functions call scan_keypad() and abort asap on keypress
  
  epd.hardwareReset();
  
  if (editing) writeStringToEPD( format_string(inputText), xx, xy, SSD1680_WRITE_RAM1 );
  else if (updateX) writeStringToEPD( xStr, xx, xy, SSD1680_WRITE_RAM1 );
  if (updateY) writeStringToEPD(      yStr, yx, yy, SSD1680_WRITE_RAM1 );
  if (battIndicator.updateNeeded || modeIndicator.updateNeeded){
    writeBattIndicatorToEPD(  SSD1680_WRITE_RAM1 );
    writeModeIndicatorToEPD(SSD1680_WRITE_RAM1);
  }
  if (updateMenu) writeMenuBarToEPD(menuOffsets, SSD1680_WRITE_RAM1);
  
  epd.updatePartial();
  
  if (editing) writeStringToEPD( format_string(inputText), xx, xy, SSD1680_WRITE_RAM2 );
  else if (updateX) writeStringToEPD( xStr, xx, xy, SSD1680_WRITE_RAM2 );
  if (updateY) writeStringToEPD(      yStr, yx, yy, SSD1680_WRITE_RAM2 );
  if (battIndicator.updateNeeded || modeIndicator.updateNeeded){
    writeBattIndicatorToEPD(  SSD1680_WRITE_RAM2 );
    writeModeIndicatorToEPD(SSD1680_WRITE_RAM2);
  }
  if (updateMenu) writeMenuBarToEPD(menuOffsets, SSD1680_WRITE_RAM2);

  if (editing) writeStringToEPD( format_string(inputText), xx, xy, SSD1680_WRITE_RAM1 );
  else if (updateX) writeStringToEPD( xStr, xx, xy, SSD1680_WRITE_RAM1 );
  if (updateY) writeStringToEPD(      yStr, yx, yy, SSD1680_WRITE_RAM1 );
  if (battIndicator.updateNeeded || modeIndicator.updateNeeded){
    writeBattIndicatorToEPD(  SSD1680_WRITE_RAM1 );
    writeModeIndicatorToEPD(SSD1680_WRITE_RAM1);
  }
  if (updateMenu) writeMenuBarToEPD(menuOffsets, SSD1680_WRITE_RAM1);

  epd.powerDown();

  updateX = false;
  updateY = false;
  battIndicator.updateNeeded = false;
  menuBar.updateNeeded = false;
  modeIndicator.updateNeeded = false;
}

void writeBattIndicatorToEPD(byte ram){
  byte xPos = SCREEN_WIDTH - battBlankBitmap_size / (battIndicatorHeight / 8);

  if (battIndicator.battIndicator == BATT_LOW){
    epd.writeBlock(battLowBitmap, battBlankBitmap_size, xPos, 0, battIndicatorHeight, ram);
    return;
  }

  epd.filledRect (WHITE, battBlankBitmap_size, xPos, 0, battIndicatorHeight, ram);

  int offset = battBlankBitmap;
  if (battIndicator.battIndicator == BATT_CHARGING) offset = battChargingBitmap;
  if (battIndicator.battIndicator == BATT_FULL) offset = battBlankBitmap;
  if (battIndicator.battIndicator == BATT_LOW) offset = battLowBitmap;
  if (battIndicator.battIndicator == BATT_OK) offset = battBlankBitmap;
  
  epd.writeBlock (offset, battBlankBitmap_size, xPos, 0, battIndicatorHeight, ram);
  if (battIndicator.battIndicator == BATT_OK){
    offset = battFillSegment;
    byte xStart = xPos + 3;
    byte width = map(battIndicator.currentBattPercent, 0, 100, 1, 25);
    for (byte i = 0; i < width; i++) epd.writeBlock(offset, 2, xStart + i, 0, battIndicatorHeight, ram);
  } 
}

void writeModeIndicatorToEPD (byte ram){
  byte xPos = modeIndicator.xPos;
  byte height = modeIndicator.height;
  int offset = radModeBitmap;
  if (modeIndicator.deg) offset = degModeBitmap;
  epd.writeBlock (offset, radModeBitmap_size, xPos, 0, height, ram);
}

void writeMenuBarToEPD (int* offsets, byte ram){
  for (byte i = 0; i < 4; i++){
    byte xPos = i * 62 + 1;
    int offset = offsets[i]; //menuBar.currentMenu[i].bitmapOffset;
    if (offset == -1) epd.filledRect(WHITE, 62 * 24 / 8 + 1, i * 62, 104, 24, ram);
    else epd.writeBlock (offset, menuBar.bitmapSize, xPos, 104, 24, ram);
  }
}

void writeStringToEPD (char* str, byte xStart, byte yPos, byte ram){
  // clear entire line
  epd.filledRect(WHITE, SCREEN_WIDTH * digitHeight / 8, 0, yPos, digitHeight, ram);

  byte xPos = xStart;
  byte height = digitHeight;
  byte y = yPos;

  // count digits to see if we should use small font
  bool small = false; 
  
  byte numDigits = 0;
  for(byte i = 0; str[i] != '\0' && str[i] != '.' && str[i] != ' '; i++) numDigits++;
  if (numDigits > 9) small = true;
  if (str[0] == 'N') small = false;

  // write text
  byte i = 0;
  bool nope = false;
  bool afterDecimal = false;
  byte decimalPlace = 1;

  // don't display zero if editing
  if (editing && yPos == xRegPos && str[0] == '0' && str[1] == ' ') i = 1;

  while (str[i] != '\0' && str[i] != ' ' && !nope){
    char c = str[i];
    
    if (c == '.') afterDecimal = true;

    if (decimalPlace > 4) small = true;
    
    unsigned int offset, numBytes;
    if (c == 'N'){
      offset = nopeBitmap;
      numBytes = nopeBitmap_size;
      nope = true;
    } else {
      bool s = small;
      switch (str[i]){
        case '.': if (s){ offset = decimalPointBitmap_small; numBytes = decimalPointBitmap_small_size; }else{ offset = decimalPointBitmap; numBytes = decimalPointBitmap_size; } break;
        case '-': if (s){ offset = negativeSignBitmap_small; numBytes = negativeSignBitmap_small_size; }else{ offset = negativeSignBitmap; numBytes = negativeSignBitmap_size; } break;
        case ',': if (s){ offset = commaBitmap_small; numBytes = commaBitmap_small_size; }else{ offset = commaBitmap; numBytes = commaBitmap_size; } break;
        case '0': if (s){ offset = _0_small; numBytes = _0_small_size; }else{ offset = _0; numBytes = _0_size; } break;
        case '1': if (s){ offset = _1_small; numBytes = _1_small_size; }else{ offset = _1; numBytes = _1_size; } break;
        case '2': if (s){ offset = _2_small; numBytes = _2_small_size; }else{ offset = _2; numBytes = _2_size; } break;
        case '3': if (s){ offset = _3_small; numBytes = _3_small_size; }else{ offset = _3; numBytes = _3_size; } break;
        case '4': if (s){ offset = _4_small; numBytes = _4_small_size; }else{ offset = _4; numBytes = _4_size; } break;
        case '5': if (s){ offset = _5_small; numBytes = _5_small_size; }else{ offset = _5; numBytes = _5_size; } break;
        case '6': if (s){ offset = _6_small; numBytes = _6_small_size; }else{ offset = _6; numBytes = _6_size; } break;
        case '7': if (s){ offset = _7_small; numBytes = _7_small_size; }else{ offset = _7; numBytes = _7_size; } break;
        case '8': if (s){ offset = _8_small; numBytes = _8_small_size; }else{ offset = _8; numBytes = _8_size; } break;
        case '9': if (s){ offset = _9_small; numBytes = _9_small_size; }else{ offset = _9; numBytes = _9_size; } break;
      }
      if (afterDecimal) decimalPlace++;
    }

    byte space = characterSpacing;
    if (small){
      space = smallCharacterSpacing;
      height = smallDigitHeight;
      y = yPos + 16;
    }
    else{
      height = digitHeight;
      y = yPos;
    }

    byte xSpaceNeeded = numBytes * 8 / height + space;
    
    // end with elipses if decimal goes off screen 
    if (afterDecimal && xPos > SCREEN_WIDTH - 35){
      offset = decimalPointBitmap_small;
      numBytes = decimalPointBitmap_small_size;
      height = smallDigitHeight;
      y = yPos + 16;

      xSpaceNeeded = numBytes * 8 / height;
      epd.writeBlock(offset, numBytes, xPos, y, height, ram);
      xPos += xSpaceNeeded;
      epd.writeBlock(offset, numBytes, xPos, y, height, ram);
      xPos += xSpaceNeeded;
      epd.writeBlock(offset, numBytes, xPos, y, height, ram);
      return;
    }
    if (xPos + xSpaceNeeded >= SCREEN_WIDTH) return;

    // write character
    epd.writeBlock(offset, numBytes, xPos, y, height, ram);
    xPos += xSpaceNeeded;

    i++;
  }
  
  // add cursor 
  if (editing && yPos == xRegPos){
    if (small){
      byte xSpaceNeeded = cursorBitmap_small_size * 8 / digitHeight;
      if (xPos + xSpaceNeeded >= SCREEN_WIDTH) return;
      epd.writeBlock(cursorBitmap_small, cursorBitmap_small_size, xPos, yPos, digitHeight, ram);
      xPos += cursorBitmap_small_size * 8 / height;
    } else {
      byte xSpaceNeeded = cursorBitmap_size * 8 / digitHeight;
      if (xPos + xSpaceNeeded >= SCREEN_WIDTH) return;
      epd.writeBlock(cursorBitmap, cursorBitmap_size, xPos, yPos, digitHeight, ram);
      xPos += cursorBitmap_size * 8 / height;
    }
  }
}

void refreshDisplay(){
  epd.fillScreen(0xFF);
  updateX = true;
  updateY = true;
  battIndicator.updateNeeded = true;
  updateDisplay();
}

// Outputs exactly 21 characters with commas, no scientific notation
char *format_string(const char* input) {
    static char output[22];  // Final output: 21 + null
    char digits[64];         // All digit characters
    char expanded[64];       // Final expanded number
    byte i = 0, d = 0;
    int8_t exponent = 0;
    int dot_pos = -1, e_pos = -1;
    bool negative = false;

    if (input[0] == '\0'){
      output[0] = '\0';
      return output;
    }

    if (input[0] == '0' && input[1] == '\0'){
      output[0] = '0';
      output[1] = '\0';
      return output;
    }

    bool ends_in_point = false;
    for (byte i = 0; i <21; i++){
      if (input[i] == '\0'){
        if (input[i-1] == '.'){
          ends_in_point = true;
          break;
        }
      }
    }

    // Handle sign
    if (input[i] == '-') {
        negative = true;
        i++;
    }

    // Parse number before 'e' or 'E'
    while (input[i]) {
        if ((input[i] == 'e' || input[i] == 'E') && e_pos == -1) {
            e_pos = i;
            break;
        } else if (input[i] == '.' && dot_pos == -1) {
            dot_pos = d;
        } else if (input[i] >= '0' && input[i] <= '9') {
            digits[d++] = input[i];
        }
        i++;
    }

    if (dot_pos == -1) dot_pos = d;

    // Parse exponent (if any)
    if (e_pos != -1) {
        int8_t sign = 1;
        i = e_pos + 1;
        if (input[i] == '+') i++;
        else if (input[i] == '-') { sign = -1; i++; }

        while (input[i] >= '0' && input[i] <= '9') {
            exponent = exponent * 10 + (input[i] - '0');
            i++;
        }
        exponent *= sign;
    }

    if (exponent > 12){
      output[0] = 'N';
      output[1] = '\0';
      return output;
    }

    if (exponent < -12){
      output[0] = '0';
      output[1] = '\0';
      return output;
    }

    // Expand scientific notation by shifting decimal
    byte new_dot = dot_pos + exponent;
    byte f = 0;
    if (new_dot <= 0) {
        expanded[f++] = '0';
        expanded[f++] = '.';
        for (byte i = 0; i < -new_dot; i++) expanded[f++] = '0';
        for (byte i = 0; i < d; i++) expanded[f++] = digits[i];
    } else if (new_dot >= d) {
        for (byte i = 0; i < d; i++) expanded[f++] = digits[i];
        for (byte i = 0; i < (new_dot - d); i++) expanded[f++] = '0';
    } else {
        for (byte i = 0; i < new_dot; i++) expanded[f++] = digits[i];
        expanded[f++] = '.';
        for (byte i = new_dot; i < d; i++) expanded[f++] = digits[i];
    }
    expanded[f] = '\0';

    // Insert commas in the integer part
    char with_commas[80];
    byte e = 0, int_len = 0;

    while (expanded[int_len] && expanded[int_len] != '.')
        int_len++;

    byte commas = (int_len - 1) / 3;
    byte i_part = 0;

    for (byte i = 0; i < int_len; i++) {
        if ((int_len - i) % 3 == 0 && i != 0)
            with_commas[e++] = ',';
        with_commas[e++] = expanded[i];
    }

    // Add decimal part (if it fits)
    while (expanded[i_part + int_len] && e < sizeof(with_commas) - 1)
        with_commas[e++] = expanded[int_len++];

    with_commas[e] = '\0';

    // Build final output: 21 chars max
    byte o = 0;
    if (negative && o < 21)
        output[o++] = '-';

    for (i = 0; with_commas[i] && o < 21; i++)
        output[o++] = with_commas[i];

    if (ends_in_point && output[o-1] != '.') output[o++] = '.';

    while (o < 21)
        output[o++] = ' ';

    output[o] = '\0';
    return output;
}