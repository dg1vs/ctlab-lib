/*
 * Copyright (c) 2007 by Hartmut Birr
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
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

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <string.h>


#include "I2CRegister.h"
#include "lcd.h"
#include "panel.h"

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

static void LCDWriteCmd(uint8_t value)
{
    uint8_t data[2];

    data[0] = value;

    data[1] = 1;                        // RS=0, R/W=0, E=1
    I2C_Register_Write(LCD_PCA9555D_ADDRESS, 2, 2, data);

    data[1] = 0;                        // RS=0, R/W=0, E=0
    I2C_Register_Write(LCD_PCA9555D_ADDRESS, 3, 1, data + 1);
}

static void LCDWriteData(uint8_t value)
{
    uint8_t data[2];

    data[0] = value;

    data[1] = 0x05;                     // RS=1, R/W=0, E=1
    I2C_Register_Write(LCD_PCA9555D_ADDRESS, 2, 2, data);

    data[1] = 0x04;                     // RS=1, R/W=0, E=0
    I2C_Register_Write(LCD_PCA9555D_ADDRESS, 3, 1, data + 1);
}


uint8_t Lcd_Init(void)
{
    uint8_t data[2];
    uint8_t i;


    /* config register see schematics */
    data[0] = 0x00;                     // P0 is output
    data[1] = 0xf8;                     // P1.0-2 is output, P1.3-7 is input
    if (I2C_Register_Write(LCD_PCA9555D_ADDRESS, 6, 2, data))
    {
        DPRINT(PSTR("%s: Couldn't detect a device at address LCD_PCA9555D_ADRESS \n"), __FUNCTION__);
        return 0;
    }

    // polarity inversion register
    data[0] = 0x00;
    data[1] = 0xf8;                     /* invert the buttons; now 4 buttons */
    I2C_Register_Write(LCD_PCA9555D_ADDRESS, 4, 2, data);

    _delay_ms(30);

#ifdef COMPILE_WITH_DISPLAY204
    LCDWriteCmd(0x34);                  // System set
    LCDWriteCmd(0x09);                  //
    LCDWriteCmd(0x30);
    LCDWriteCmd(0x0c);                  // display on
    LCDWriteCmd(0x01);                  // clear Display
    _delay_ms(2);

    LCDWriteCmd(0x06);                  // Entry Mode set
#else
    LCDWriteCmd(0x38);                  // System set

    LCDWriteCmd(0x0c);                  // display on
    LCDWriteCmd(0x01);                  // clear Display
    _delay_ms(2);
    LCDWriteCmd(0x06);                  // Entry Mode set
#endif

    LCDWriteCmd(LCD_PCA9555D_ADDRESS);
    for (i = 0; i < sizeof(cursor); i++)
    {
        LCDWriteData(pgm_read_byte(&cursor[i]));
    }

    memset(DisplayData, ' ', sizeof(DisplayData));

    return 1;
}

static void LCDInternalWrite(uint8_t x, uint8_t y, uint8_t len, const char* data, uint8_t prog)
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
                LCDWriteCmd(0x80 | addr);
                valid = 1;
            }
            LCDWriteData(ch);
            *ptr = ch;
        }
        data++;
        addr++;
        ptr++;
    }
}


void Lcd_Write(uint8_t x, uint8_t y, uint8_t len, const char* data)
{
    LCDInternalWrite(x, y, len, data, 0);
}

void Lcd_Write_P(uint8_t x, uint8_t y, uint8_t len, const char* data)
{
    LCDInternalWrite(x, y, len, (const char*)data, 1);
}

void Lcd_OverWrite_P(uint8_t x, uint8_t y, uint8_t len, const char* data)
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

    I2C_Register_Read(LCD_PCA9555D_ADDRESS, 1, 1, &data);

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
