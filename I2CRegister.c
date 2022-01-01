/*
 * Copyright (c) 2007 by Hartmut Birr
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

#include <util/twi.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "I2CRegister.h"

int8_t I2CRegister_Read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* data)
{

    I2C_MSG msg[2];

    msg[0].addr = addr;
    msg[0].read = 0;
    msg[0].nostart = 0;
    msg[0].len = 1;
    msg[0].data[0] = reg;
    msg[1].addr = addr;
    msg[1].read = 1;
    msg[1].nostart = 0;
    msg[1].data_ptr = data;
    msg[1].len = len;

    if (2 == I2C_Transfer(msg, 2))
    {
        if (len <= 2)
        {
            data[0] = msg[1].data[0];
            if (len == 2)
            {
                data[1] = msg[1].data[1];
            }
        }
        return 0;
    }
    return -1;
}


int8_t I2CRegister_ReadImplicit(uint8_t addr, uint8_t len, uint8_t* data)
{

    I2C_MSG msg[1];


    msg[0].addr = addr;
    msg[0].read = 1;
    msg[0].nostart = 0;
    msg[0].data_ptr = data;
    msg[0].len = len;

    if (1 == I2C_Transfer(msg, 1))
    {
        if (len <= 2)
        {
            data[0] = msg[0].data[0];
            if (len == 2)
            {
                data[1] = msg[0].data[1];
            }
        }
        return 0;
    }
    return -1;
}



int8_t I2CRegister_Write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* data)
{
    I2C_MSG msg[2];

    msg[0].addr = addr;
    msg[0].read = 0;
    msg[0].nostart = 0;
    msg[0].len = 1;
    msg[0].data[0] = reg;

    msg[1].addr = addr;
    msg[1].read = 0;
    msg[1].nostart = 1;
    msg[1].len = len;

    if (len <= 2)
    {
        msg[1].data[0] = data[0];
        if (len == 2)
        {
            msg[1].data[1] = data[1];
        }
    }
    else
    {
        msg[1].data_ptr = data;
    }
    return ((2 == I2C_Transfer(msg, 2)) ? 0 : -1 );
}


int8_t I2CRegister_WriteImplicit(uint8_t addr, uint8_t len, uint8_t* data)
{
    I2C_MSG msg[2];

    msg[0].addr = addr;
    msg[0].read = 0;
    msg[0].nostart = 0;
    msg[0].len = 1;
    msg[0].data[0] = data[0];                           // pointer register value

    if (len > 1)
    {
        msg[1].addr = addr;
        msg[1].read = 0;
        msg[1].nostart = 1;
        msg[1].len = len - 1;

        if ((len - 1) <= 2)
        {
            if ((len - 1) >= 1)
                msg[1].data[0] = data[1];                   // first or single data byte
            if ((len - 1) == 2)
            {
                msg[1].data[1] = data[2];               // optional second data byte
            }
        }
        else
        {
            msg[1].data_ptr = data + 1;
        }

        return ((2 == I2C_Transfer(msg, 2)) ? 0 : -1);  // write pointer and 1 or 2 data bytes
    }
    else
        return ((1 == I2C_Transfer(msg, 1)) ? 0 : -1);      // just "write" pointer
}
