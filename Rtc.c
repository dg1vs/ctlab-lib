/*
 * Copyright (c) 2012-13 by Paul Schmid
 * Copyright (c) 2017-18 by Karsten Schmidt
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "main.h"
#include "debug.h"
#include "helper_macros.h"
#include "Rtc.h"



uint8_t g_ucRTCpresent;
volatile RTC_STRUCT gRTC;


void Ds1302_SetChip(uint8_t ucCommand, uint8_t ucDataOut)
{
    uint8_t count;

    uint8_t sreg = SREG;
    cli();

    uint8_t spcr = SPCR;       // save SPI settings for SD card
    SPCR = 0;                  // disable SPI interface

    PORTB &= ~(1 << DS1302_CLK);
    PORTA |=  (1 << DS1302_RST);

    // ucCommand contains already the "command" from the data sheet of DS1302
    ucCommand &= 0xFE;        // just make sure it is the write command (LSB = 0)
    count = 8;

    while (count)
    {
        if (ucCommand & 0x01)
        {
            PORTB |= (1 << DS1302_IO);
        }
        else
        {
            PORTB &= ~(1 << DS1302_IO);
        }

        PORTB |= (1 << DS1302_CLK);
        ucCommand = ucCommand >> 1;
        nop();
        nop();
        PORTB &= ~(1 << DS1302_CLK);
        count--;
    }

    count = 8;

    while (count)
    {
        if (ucDataOut & 0x01)
        {
            PORTB |= (1 << DS1302_IO);
        }
        else
        {
            PORTB &= ~(1 << DS1302_IO);
        }

        PORTB |= (1 << DS1302_CLK);
        ucDataOut = ucDataOut >> 1;
        nop();
        nop();
        PORTB &= ~(1 << DS1302_CLK);
        count--;
    }

    PORTA &= ~(1 << DS1302_RST);

    SPCR = spcr;
    SREG = sreg;
}


uint8_t Ds1302_GetChip(uint8_t ucCommand)
{
    uint8_t count;
    uint8_t ucDataIn = 0;

    uint8_t sreg = SREG;
    cli();

    uint8_t spcr = SPCR;        // save SPI settings for SD card
    SPCR = 0;                   // disable SPI interface

    PORTB &= ~(1 << DS1302_CLK);
    PORTA |=  (1 << DS1302_RST);

    // ucCommand contains already the "command" from the data sheet of DS1302
    ucCommand |= 0x01;          // just make sure it is the read command (LSB = 1)
    count = 8;

    while (count)
    {
        if (ucCommand & 0x01)
        {
            PORTB |= (1 << DS1302_IO);
        }
        else
        {
            PORTB &= ~(1 << DS1302_IO);
        }

        PORTB |= (1 << DS1302_CLK);
        ucCommand = ucCommand >> 1;
        nop();
        nop();
        PORTB &= ~(1 << DS1302_CLK);
        count--;
    }

    DDRB &= ~(1 << DS1302_IO);
    count = 8;

    while (count)
    {
        ucDataIn = ucDataIn >> 1;
        if (PINB & (1 << DS1302_IO))
        {
            ucDataIn |= 0x80;   // set MSB
        }
        else
        {
            ucDataIn &= 0x7F;   // clear MSB
        }

        PORTB |= (1 << DS1302_CLK);
        nop();
        nop();
        PORTB &= ~(1 << DS1302_CLK);
        count--;
    }

    PORTA &= ~(1 << DS1302_RST);
    DDRB |= (1 << DS1302_IO);

    SPCR = spcr;
    SREG = sreg;

    return ucDataIn;
}


void Ds1302_SetChipBurst(uint8_t ucData[])
{

    uint8_t ucCountBit;
    uint8_t ucCountByte;
    uint8_t ucCommand;
    uint8_t ucDataOut;

    uint8_t sreg = SREG;

    cli();

    uint8_t spcr = SPCR;       // save SPI settings for SD card
    SPCR = 0;                  // disable SPI interface

    PORTB &= ~(1 << DS1302_CLK);
    PORTA |=  (1 << DS1302_RST);

    ucCommand = RTC_COMMAND_BURST_WRITE;


    ucCountBit = 8;

    while (ucCountBit)
    {
        if (ucCommand & 0x01)
        {
            PORTB |= (1 << DS1302_IO);
        }
        else
        {
            PORTB &= ~(1 << DS1302_IO);
        }

        PORTB |= (1 << DS1302_CLK);
        ucCommand = ucCommand >> 1;
        nop();
        nop();
        PORTB &= ~(1 << DS1302_CLK);
        ucCountBit--;
    }

    ucCountByte = 0;

    while (ucCountByte < 8)
    {
        ucDataOut = ucData[ucCountByte];
        ucCountBit = 8;

        while (ucCountBit)
        {
            if (ucDataOut & 0x01)
            {
                PORTB |= (1 << DS1302_IO);
            }
            else
            {
                PORTB &= ~(1 << DS1302_IO);
            }

            PORTB |= (1 << DS1302_CLK);
            ucDataOut = ucDataOut >> 1;
            nop();
            nop();
            PORTB &= ~(1 << DS1302_CLK);
            ucCountBit--;
        }
        ucCountByte++;
    }

    PORTA &= ~(1 << DS1302_RST);

    SPCR = spcr;
    SREG = sreg;
}


void Ds1302_GetChipBurst(uint8_t ucData[])
{

    uint8_t ucCountBit;
    uint8_t ucCountByte;
    uint8_t ucCommand;
    uint8_t ucDataIn;

    uint8_t sreg = SREG;

    cli();

    uint8_t spcr = SPCR;       // save SPI settings for SD card
    SPCR = 0;                  // disable SPI interface

    PORTB &= ~(1 << DS1302_CLK);
    PORTA |=  (1 << DS1302_RST);

    ucCommand = RTC_COMMAND_BURST_READ;

    ucCountBit = 8;

    while (ucCountBit)
    {
        if (ucCommand & 0x01)
        {
            PORTB |= (1 << DS1302_IO);
        }
        else
        {
            PORTB &= ~(1 << DS1302_IO);
        }

        ucCommand = ucCommand >> 1;
        nop();
        PORTB |= (1 << DS1302_CLK);
        ucCountBit--;
        nop();
        PORTB &= ~(1 << DS1302_CLK);
    }

    PORTB |= (1 << DS1302_IO);
    DDRB &= ~(1 << DS1302_IO);
    ucCountByte = 0;

    while (ucCountByte < 8)
    {
        ucCountBit = 8;
        ucDataIn = 0;

        while (ucCountBit)
        {
            ucDataIn = ucDataIn >> 1;
            if (PINB & (1 << DS1302_IO))
            {
                ucDataIn |= 0x80;   // set MSB
            }
            else
            {
                ucDataIn &= 0x7F;   // clear MSB
            }

            PORTB |= (1 << DS1302_CLK);
            nop();
            nop();
            PORTB &= ~(1 << DS1302_CLK);
            ucCountBit--;
        }
        ucData[ucCountByte] = ucDataIn;
        ucCountByte++;
    }

    PORTA &= ~(1 << DS1302_RST);
    DDRB |= (1 << DS1302_IO);

    SPCR = spcr;
    SREG = sreg;
}

uint8_t BCDtoByte(uint8_t ucBCD)
{
    return ((ucBCD >> 4) * 10 ) + (ucBCD & 0x0F);
}


uint8_t BytetoBCD(uint8_t ucByte)
{
    return ((ucByte / 10) << 4 ) | (ucByte % 10);
}


void Rtc_ReadChipTime(void)
{
    uint8_t ucTimeBuffer[8];

    if (g_ucRTCpresent)
    {
        Ds1302_GetChipBurst(ucTimeBuffer);

        uint8_t sreg = SREG;
        cli();

        gRTC.ucHours   = BCDtoByte(ucTimeBuffer[2]);
        gRTC.ucMinutes = BCDtoByte(ucTimeBuffer[1]);
        gRTC.ucSeconds = BCDtoByte(ucTimeBuffer[0]);
        gRTC.ucDay     = BCDtoByte(ucTimeBuffer[3]);
        gRTC.ucMonth   = BCDtoByte(ucTimeBuffer[4]);
        gRTC.ucYear    = BCDtoByte(ucTimeBuffer[6]);
        gRTC.uiYearWord= (uint16_t)BCDtoByte((ucTimeBuffer[6])) + 2000;

        SREG = sreg;
    }
}


void Rtc_SetChipTime(void)
{
    uint8_t ucTimeBuffer[8];

    if (g_ucRTCpresent)
    {
        uint8_t sreg = SREG;
        cli();

        ucTimeBuffer[2] = BytetoBCD(gRTC.ucHours)   & 0x3F;
        ucTimeBuffer[1] = BytetoBCD(gRTC.ucMinutes) & 0x7F;
        ucTimeBuffer[0] = BytetoBCD(gRTC.ucSeconds) & 0x7F;
        ucTimeBuffer[3] = BytetoBCD(gRTC.ucDay)     & 0x3F;
        ucTimeBuffer[4] = BytetoBCD(gRTC.ucMonth)   & 0x1F;
        ucTimeBuffer[6] = BytetoBCD(gRTC.ucYear);
        ucTimeBuffer[5] = 0;                // not used (actually day of week)

        Ds1302_SetChip(RTC_COMMAND_WP, 0x00);   // disable write protection
        Ds1302_SetChipBurst(ucTimeBuffer);
        Ds1302_SetChip(RTC_COMMAND_WP, 0x80);   // enable write protection

        SREG = sreg;
    }
}


void Rtc_CheckChipPresent(void)
{
    uint8_t ucWP;

    Ds1302_SetChip(RTC_COMMAND_WP, 0x00);       // RTC unlock and lock to see if the chip is present
    ucWP = Ds1302_GetChip(RTC_COMMAND_WP);

    if (ucWP == 0x00)
    {
        Ds1302_SetChip(RTC_COMMAND_WP, 0x80);   // RTC lock anyway
        ucWP = Ds1302_GetChip(RTC_COMMAND_WP);
        if (ucWP == 0x80)
        {
            g_ucRTCpresent = 1;
        }
        else
        {
            g_ucRTCpresent = 0;
        }
    }
    else
    {
        g_ucRTCpresent = 0;
    }
	// TODO geht nicht warum... wieder ergänzen
	// LOG_INFO(PSTR("RTC is %Spresent\n"), g_ucRTCpresent ? PSTR("") : PSTR("not "));

}


void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
    uint8_t sreg = SREG;
    cli();

    *year  = gRTC.uiYearWord;
    *month = gRTC.ucMonth;
    *day   = gRTC.ucDay;
    *hour  = gRTC.ucHours;
    *min   = gRTC.ucMinutes;
    *sec   = gRTC.ucSeconds;

    SREG = sreg;
}
