/*!
 * @file EPD.h
 *
 * This is for driving a GDEY0213B74 2.13 Inch 250x122 Eink Display w/ SSD1680 chipset
 * the best clues are at https://github.com/ZinggJM/GxEPD2
 * with some more clues at https://www.good-display.com/product/391.html
 *
 */

#ifndef _EPD_H_
#define _EPD_H_

#include <SPI.h>

// (memory height) actual screen is 122
#define SCREEN_HEIGHT 128 
#define SCREEN_WIDTH 250
#define MEM_SIZE 4000
#define WHITE true
#define BLACK false

#define SSD1680_DRIVER_CONTROL 0x01
#define SSD1680_GATE_VOLTAGE 0x03
#define SSD1680_SOURCE_VOLTAGE 0x04
#define SSD1680_PROGOTP_INITIAL 0x08
#define SSD1680_PROGREG_INITIAL 0x09
#define SSD1680_READREG_INITIAL 0x0A
#define SSD1680_BOOST_SOFTSTART 0x0C
#define SSD1680_DEEP_SLEEP 0x10
#define SSD1680_DATA_MODE 0x11
#define SSD1680_SW_RESET 0x12
#define SSD1680_TEMP_CONTROL 0x18
#define SSD1680_TEMP_WRITE 0x1A
#define SSD1680_MASTER_ACTIVATE 0x20
#define SSD1680_DISP_CTRL1 0x21
#define SSD1680_DISP_CTRL2 0x22
#define SSD1680_WRITE_RAM1 0x24
#define SSD1680_WRITE_RAM2 0x26
#define SSD1680_WRITE_VCOM 0x2C
#define SSD1680_READ_OTP 0x2D
#define SSD1680_READ_STATUS 0x2F
#define SSD1680_WRITE_LUT 0x32
#define SSD1680_WRITE_BORDER 0x3C
#define SSD1680_SET_RAMXPOS 0x44
#define SSD1680_SET_RAMYPOS 0x45
#define SSD1680_SET_RAMXCOUNT 0x4E
#define SSD1680_SET_RAMYCOUNT 0x4F

// don't abort a partial update until at after this (in ms) 
#define MIN_UPDATE_TIME 220 


class EPD{
public:
    EPD(byte _dc_pin, byte _reset_pin, byte _busy_pin, bool (*_poll)());
    ~EPD();
    
    void begin();
    void fillScreen(byte d);
    void fillScreenFast(byte d);
    void hardwareReset();
    void powerDown();
    void writeBlock(byte bytes[], int numBytes, byte xPos, byte yPos, byte h, byte which_ram);
    void filledRect(bool white, int numBytes, byte xPos, byte yPos, byte h, byte which_ram);
    void updatePartial();

protected:

    void fullUpdate();
    void powerUp();
    void waitWhileBusy();
    void write_command(uint8_t c);
    void write_data(uint8_t d);

    int dc_pin, reset_pin, busy_pin;

    bool (*poll)(); // keypad polling function to call while waiting for display updates
};

#endif /* _EPD_H_ */
