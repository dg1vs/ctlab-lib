/*
 * Copyright (c) 2006, 2007 by Hartmut Birr
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

#ifndef _I2C_H_
#define _I2C_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        uint8_t addr;
        uint8_t len;
        uint8_t read : 1;
        uint8_t nostart : 1;
        union
        {
            uint8_t* data_ptr;
            uint8_t data[2];
        };
    } I2C_MSG;

    void I2C_Init(void);
    int8_t I2C_Transfer(I2C_MSG* msg, uint8_t count);
    //int8_t i2c_read_regs(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif
