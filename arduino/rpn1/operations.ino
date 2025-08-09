#include "globals.h"

void do_enter(){
    if(editing){
    stack[0] = fp64_atof(inputText);
    //Serial.println(fp64_to_string(stack[0], 16, 10));
    stack[3] = stack[2];
    stack[2] = stack[1];
    stack[1] = stack[0];
    editing = false;
  }
  else{
    stack[3] = stack[2];
    stack[2] = stack[1];
    stack[1] = stack[0];
  }
}

void do_add(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_add( stack[0], stack[1] );
  stack[1] = stack[2];
  stack[2] = stack[3];
  editing = false;
}

void do_subtract(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_sub( stack[1], stack[0] );
  stack[1] = stack[2];
  stack[2] = stack[3];
  editing = false;
}

void do_multiply(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_mul( stack[0], stack[1] );
  stack[1] = stack[2];
  stack[2] = stack[3];
  editing = false;
}

void do_divide(){
  if (editing) stack[0] = fp64_atof(inputText);
  // if y is 0, result is always 0
  if (fp64_compare(stack[1], fp64_sd(0.0)) == 0){
    //Serial.println("zero numerator");
    stack[0] = fp64_sd(0.0);
  }
  else stack[0] = fp64_div( stack[1], stack[0] );
  stack[1] = stack[2];
  stack[2] = stack[3];
  editing = false;
}

void do_swap(){
  if (editing) stack[0] = fp64_atof(inputText);
  float64_t temp = stack[0];
  stack[0] = stack[1];
  stack[1] = temp;
  editing = false;
}

void do_rotate(){
  if (editing) stack[0] = fp64_atof(inputText);
  float64_t temp = stack[0];
  stack[0] = stack[1];
  stack[1] = stack[2];
  stack[2] = stack[3];
  stack[3] = temp;
  editing = false;
}

void do_inverse(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_div( fp64_sd(1.0), stack[0] );
  editing = false;
}

void do_square_root(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_sqrt(stack[0]);
  editing = false;
}

void do_negative(){
  if (editing){
    if (inputText[0] == '\0'){
      inputText[0] = '-';
      inputText[1] = '\0';
      cursorPos = 1;
    } else {
      stack[0] = fp64_atof(inputText);
      stack[0] = fp64_mul( stack[0], fp64_sd(-1.0) );
      strcpy( inputText, fp64_to_string(stack[0], 16, 10) );
      cursorPos = 0;
      for (byte i = 0; inputText[i] != '\0'; i++){
        cursorPos++;
        if (i == strlen(inputText)) break;
      }
    }
  } else {
    stack[0] = fp64_mul( stack[0], fp64_sd(-1.0) );
  }
}

void do_pi(){
  stack[0] = fp64_atof("3.141592653589793"); //float64_NUMBER_PI;
  editing = false;
}

void do_delete(){
  if(!editing){
    editing = true;
    cursorPos = 0;
    inputText[cursorPos]= '\0';
  }
  if (cursorPos > 0) cursorPos--;
  inputText[cursorPos] = '\0';
}

void do_square(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_square( stack[0] );
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
  //char str[maxStringLength];
  //numToString (stack[0], str);
  //Serial.println (num_to_string(stack[0]));
}

void do_sin(){
  if (editing) stack[0] = fp64_atof(inputText);
  if (modeIndicator.deg) stack[0] = degToRad (stack[0]);
  stack[0] = fp64_sin( stack[0] );
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
}

void do_cos(){
  if (editing) stack[0] = fp64_atof(inputText);
  if (modeIndicator.deg) stack[0] = degToRad (stack[0]);
  stack[0] = fp64_cos( stack[0] );
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
}

void do_tan(){
  if (editing) stack[0] = fp64_atof(inputText);
  if (modeIndicator.deg) stack[0] = degToRad (stack[0]);
  stack[0] = fp64_tan( stack[0] );
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
}

void inverse(){
  menuBar.currentMenu = menuBar.inverseMenu;
  menuBar.updateNeeded = true;
  modeIndicator.show();
}

void do_asin(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_asin( stack[0] );
  if (modeIndicator.deg) stack[0] = radToDeg (stack[0]);
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
}

void do_acos(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_acos( stack[0] );
  if (modeIndicator.deg) stack[0] = radToDeg (stack[0]);
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
}

void do_atan(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_atan( stack[0] );
  if (modeIndicator.deg) stack[0] = radToDeg (stack[0]);
  editing = false;
  //Serial.println (fp64_to_string(stack[0], 16, 10));
}

void more(){
  menuBar.currentMenu = menuBar.moreMenu;
  menuBar.updateNeeded = true;
}

void inToMm(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_mul(stack[0], fp64_sd(25.4));
  editing = false;
}

void mmToIn(){
  if (editing) stack[0] = fp64_atof(inputText);
  stack[0] = fp64_div(stack[0], fp64_sd(25.4));
  editing = false;
}

void rad_mode(){
  modeIndicator.deg = false;
  modeIndicator.show();
  menuBar.moreMenu[2].bitmapOffset = degBitmap;
  menuBar.moreMenu[2].action = deg_mode;
  menuBar.updateNeeded = true;
}

void deg_mode(){
  modeIndicator.deg = true;
  modeIndicator.show();
  menuBar.moreMenu[2].bitmapOffset = radBitmap;
  menuBar.moreMenu[2].action = rad_mode;
  menuBar.updateNeeded = true;
}

void exit_menu(){
  menuBar.currentMenu = menuBar.blankMenu;
  menuBar.updateNeeded = true;
}

float64_t degToRad(float64_t d){
    // convert to radians
    // ( deg x pi / 180 = rad )
    float64_t r;
    r = fp64_mul(d, float64_NUMBER_PI);
    r = fp64_div(r, fp64_sd(180.0));
    return r;
}

float64_t radToDeg(float64_t r){
  // rad = deg x 180 / pi
  float64_t d;
  d = fp64_mul(r, fp64_sd(180.0));
  d = fp64_div(d, float64_NUMBER_PI);
  return d;
}
