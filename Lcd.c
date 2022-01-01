/*
 * Copyright (c) 2007, 2008 by Hartmut Birr
 *
 * This program is free software; you can redistribute it and/or
 * mmodify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "config.h"

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

#include "I2CRegister.h"
#include "Lcd.h"

#define NDEBUG
#include "debug.h"

// P0.x -> D7..0
// P1.0 -> E
// P1.1 -> R/W
// P1.2 -> RS


#ifdef COMPILE_WITH_DISPLAY204
	static char DisplayData[80];
	const PROGMEM char ucWhites[] = "                    ";    /* 20 spaces */
#else
	static char DisplayData[16];
	const PROGMEM char ucWhites[] = "        ";                /* 8 spaces */
#endif

static const PROGMEM uint8_t cursor[] = {0x01, 0x03, 0x07, 0x0f, 0x07, 0x03, 0x01, 0x00,   // solid triangle
	0x01, 0x03, 0x05, 0x09, 0x05, 0x03, 0x01, 0x00,   // concave triangle
	0x01, 0x02, 0x05, 0x0a, 0x05, 0x02, 0x01, 0x00,   // hatched triangle
	0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a,   // hatched block
	0x1F, 0x00, 0x0C, 0x0A, 0x0C, 0x0A, 0x0A, 0x00,   // small R - high
	0x00, 0x0C, 0x0A, 0x0C, 0x0A, 0x0A, 0x00, 0x1F,   // small R - low
	0x1F, 0x1F, 0x1B, 0x15, 0x11, 0x15, 0x15, 0x1F,   // small A - inverted
	0x1F, 0x11, 0x04, 0x0A, 0x0E, 0x0A, 0x0A, 0x00    // small A
};

//                                    0x1F, 0x11, 0x15, 0x15, 0x15, 0x11, 0x1F, 0x00,   // big rectangle with vertical bar

void __Lcd_WriteCmd(uint8_t value)
{
    uint8_t data[2];

    data[0] = value;

    data[1] = 0;                        // RS=0, R/W=0, E=0
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 2, 2, data);

    data[1] = 1;                        // RS=0, R/W=0, E=1
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 3, 1, data + 1);

    data[1] = 0;                        // RS=0, R/W=0, E=0
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 3, 1, data + 1);
}

void __Lcd_WriteData(uint8_t value)
{
    uint8_t data[2];

    data[0] = value;

    data[1] = 0x04;                     // RS=1, R/W=0, E=0
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 2, 2, data);

    data[1] = 0x05;                     // RS=1, R/W=0, E=1
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 3, 1, data + 1);

    data[1] = 0x04;                     // RS=1, R/W=0, E=0
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 3, 1, data + 1);
}


uint8_t Lcd_Init(void)
{
    uint8_t data[2];
    uint8_t i;


    // config register
    data[0] = 0x00;                     // P0 is output
    data[1] = 0xf8;                     // P1.0-2 is output, P1.3-7 is input
    if (I2CRegister_Write(LCD_PCA9555D_ADDRESS, 6, 2, data))
    {
        DPRINT(PSTR("%s: Couldn't detect a device at address LCD_PCA9555D_ADDRESS \n"), __FUNCTION__);
        return 0;
    }

    // polarity inversion register
    data[0] = 0x00;
    data[1] = 0x38;                     // invert the buttons
    I2CRegister_Write(LCD_PCA9555D_ADDRESS, 4, 2, data);


    _delay_ms(30);

#ifdef COMPILE_WITH_DISPLAY204
    __Lcd_WriteCmd(0x34);                  // System set
    __Lcd_WriteCmd(0x09);                  //
    __Lcd_WriteCmd(0x30);
    __Lcd_WriteCmd(0x0c);                  // display on
    __Lcd_WriteCmd(0x01);                  // clear Display
    _delay_ms(2);
    __Lcd_WriteCmd(0x06);                  // Entry Mode set
#else
    __Lcd_WriteCmd(0x38);                  // System set
    __Lcd_WriteCmd(0x0c);                  // display on
    __Lcd_WriteCmd(0x01);                  // clear Display
    _delay_ms(2);
    __Lcd_WriteCmd(0x06);                  // Entry Mode set
#endif

    __Lcd_WriteCmd(0x40);
    for (i = 0; i < sizeof(cursor); i++)
    {
        __Lcd_WriteData(pgm_read_byte(&cursor[i]));
    }
    memset(DisplayData, ' ', sizeof(DisplayData));
    return 1;
}

static void __Lcd_InternalWrite(uint8_t x, uint8_t y, uint8_t len, const char* data, uint8_t prog)
{
    uint8_t addr;
    uint8_t valid;
    char ch;
    char* ptr;

    if (x >= COLUMN_MAX || y >= LINE_MAX)
    {
        return;
    }
    addr = x;

#ifdef COMPILE_WITH_DISPLAY204
    // TODO check case vs. if else if in asm
    /* see data-sheet about address */
    if (y == 1)
    {
        addr += 0x20;
    }
    else if (y == 2)
    {
        addr += 0x40;
    }
    else if (y == 3)
    {
        addr += 0x60;
    }
#else
    if (y == 1)
    {
        addr += 64;
    }
#endif

    if (x + len > COLUMN_MAX)
    {
        len = COLUMN_MAX - x;
    }

    ptr = DisplayData + COLUMN_MAX * y + x;
    valid = 0;
    while(len--)
    {
        ch = prog ? pgm_read_byte((const PROGMEM char*)data) : *data;
        if (*ptr == ch)
        {
            valid = 0;
        }
        else
        {
            if (!valid)
            {
                __Lcd_WriteCmd(0x80 | addr);
                valid = 1;
            }
            __Lcd_WriteData(ch);
            *ptr = ch;
        }
        data++;
        addr++;
        ptr++;
    }
}


void Lcd_Write(uint8_t x, uint8_t y, uint8_t len, const char* data)
{
    __Lcd_InternalWrite(x, y, len, data, 0);
}

void Lcd_Write_P(uint8_t x, uint8_t y, uint8_t len, const char* data)
{
    __Lcd_InternalWrite(x, y, len, (const char*)data, 1);
}

void LCDOverwrite_P(uint8_t x, uint8_t y, uint8_t len, const char* data)
{
    //  clearing leading and trailing whitespaces instead of calling LCDClearLine(y) to save traffic on I2C-Bus
    Lcd_Write_P(0, y, x, ucWhites);
    Lcd_Write_P(x+len, y, COLUMN_MAX, ucWhites);

    // now writing real content
    Lcd_Write_P(x, y, len, data);
}


void Lcd_ClearLine(uint8_t y)
{
    Lcd_Write_P(0, y, COLUMN_MAX, ucWhites);
}


uint8_t Lcd_GetButton(void)
{
    uint8_t data = 0x00;
    uint8_t result = 0;

    I2CRegister_Read(LCD_PCA9555D_ADDRESS, 1, 1, &data);

#ifdef COMPILE_WITH_DISPLAY204
    if (data & 0x80)
    {
        result |= BUTTON_SOFT_KEY_2;
    }
    if (data & 0x40)
    {
        result |= BUTTON_SOFT_KEY_1;
    }
#endif

    if (data & 0x20)
    {
        result |= BUTTON_DOWN;
    }
    if (data & 0x10)
    {
        result |= BUTTON_UP;
    }
    if (data & 0x08)
    {
        result |= BUTTON_ENTER;
    }

    return result;
}
