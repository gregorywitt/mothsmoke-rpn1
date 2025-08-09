#ifndef _GLOBALS_H_
#define _GLOBALS_H_


// Pins

#define POWER_ENABLE_PIN 12
#define POWER_BUTTON_PIN 8
#define LED_PIN 13
#define EPD_DC_PIN A4
#define EPD_RESET_PIN A5
#define EPD_BUSY_PIN 6
// all columns are on interrupts
const uint8_t col_pins[5] = {0, 1, 2, 3, 7};
const uint8_t row_pins[6] = {A2, A1, A0, 5, 4, 9};


// Global Variables

#define EEPROM_SHUTDOWN_NORMAL_ADDR 0
#define EEPROM_MODE_ADDR 1

const byte yRegPos = 16;
const byte xRegPos = 64;
const byte digitHeight = 40;
const byte smallDigitHeight = 24;
const byte battIndicatorHeight = 16;
const byte characterSpacing = 2;
const byte smallCharacterSpacing = 1;
const byte maxStringLength = 28;
const byte keyBufferSize = 5;
float64_t maxValue = fp64_atof("9999999999999");

float64_t stack[4];
char inputText[maxStringLength];

const byte num_dec_places = 4;

bool editing = true;
bool updateX = true;
bool updateY = true;

byte cursorPos = 0;
int digitOffsets[10];
int digitSmallOffsets[10];
unsigned long numSecsAsleep = 0;

// keypad

const byte num_cols = 5;
const byte num_rows = 6;
const byte debounceTime = 18;
byte numKeypresses = 0;
bool keyStates[30];
byte keyBuffer[keyBufferSize];
byte keyBufferIndex = 0;
unsigned long keyChangeTime;

#endif /* _GLOBALS_H_ */
