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

void LCDWriteCmd(uint8_t value)
{
    uint8_t data[2];

    data[0] = value;

    data[1] = 0;                        // RS=0, R/W=0, E=0
    I2CRegister_Write(0x40, 2, 2, data);

    data[1] = 1;                        // RS=0, R/W=0, E=1
    I2CRegister_Write(0x40, 3, 1, data + 1);

    data[1] = 0;                        // RS=0, R/W=0, E=0
    I2CRegister_Write(0x40, 3, 1, data + 1);
}

void LCDWriteData(uint8_t value)
{
    uint8_t data[2];

    data[0] = value;

    data[1] = 0x04;                     // RS=1, R/W=0, E=0
    I2CRegister_Write(0x40, 2, 2, data);

    data[1] = 0x05;                     // RS=1, R/W=0, E=1
    I2CRegister_Write(0x40, 3, 1, data + 1);

    data[1] = 0x04;                     // RS=1, R/W=0, E=0
    I2CRegister_Write(0x40, 3, 1, data + 1);
}

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

uint8_t Lcd_Init(void)
{
    uint8_t data[2];
    uint8_t i;


    // config register
    data[0] = 0x00;                     // P0 is output
    data[1] = 0xf8;                     // P1.0-2 is output, P1.3-7 is input
    if (I2CRegister_Write(0x40, 6, 2, data))
    {
        DPRINT(PSTR("%s: Couldn't detect a device at address 0x40\n"), __FUNCTION__);
        return 0;
    }

    // polarity inversion register
    data[0] = 0x00;
    data[1] = 0x38;                     // invert the buttons
    I2CRegister_Write(0x40, 4, 2, data);


    _delay_ms(30);

    LCDWriteCmd(0x38);                  // System set

    LCDWriteCmd(0x0c);                  // display on

    LCDWriteCmd(0x01);                  // clear Display
    _delay_ms(2);

    LCDWriteCmd(0x06);                  // Entry Mode set

    LCDWriteCmd(0x40);
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


    if (x >= 8 || y >= 2)
    {
        return;
    }

    addr = x;
    if (y == 1)
    {
        addr += 64;
    }

    if (x + len > 8)
    {
        len = 8 - x;
    }

    ptr = DisplayData + 8 * y + x;
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

uint8_t Lcd_GetButton(void)
{
    uint8_t data = 0x00;
    uint8_t result = 0;

    I2CRegister_Read(0x40, 1, 1, &data);

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
