/*
 * Copyright (c) 2006, 2007, 2008 by Hartmut Birr
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

#include <util/twi.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "I2C.h"

//#define USE_TICKER_FOR_TIMEOUT

#ifdef USE_TICKER_FOR_TIMEOUT
#include "timer.h"
#else
#include <util/delay.h>
#endif

#define NDEBUG
#include "debug.h"

#ifndef I2C_BUS_SPEED
#define I2C_BUS_SPEED   400000L
#endif

#ifndef I2C_TIMEOUT_MS
#define I2C_TIMEOUT_MS  15
#endif

void I2C_Init(void)
{
    DPRINT(PSTR("initI2C()\n"));
    TWSR = 0;
#if F_CPU < 3600000UL
    TWBR = 10;
#else
    TWBR = (F_CPU / I2C_BUS_SPEED - 16) / 2;
    DPRINT(PSTR("frequency=%d.%03dMHz, i2c-speed=%dkHz, TWBR=0x%x\n"),
           (uint16_t)(F_CPU / 1000000), (uint16_t)((F_CPU / 1000) % 1000),
           (uint16_t)(I2C_BUS_SPEED / 1000), (uint8_t)((F_CPU / I2C_BUS_SPEED - 16) / 2));
#endif
}

#define I2C_ERR_OK          0
#define I2C_ERR_TIMEOUT     -1
#define I2C_ERR_SLA_NACK    -2
#define I2C_ERR_DATA_NACK   -3
#define I2C_ERR_ARB         -4
#define I2C_ERR_BUS         -5

static I2C_MSG* i2c_msg;
static volatile uint8_t i2c_msg_len;
static uint8_t* i2c_data;
static uint8_t i2c_data_len;
static volatile uint8_t i2c_err;
static volatile uint8_t i2c_busy;
static volatile uint8_t i2c_status;

int8_t i2c_transfer(I2C_MSG* msg, uint8_t count)
{
    i2c_msg = msg;
    i2c_msg_len = count;
    i2c_err = I2C_ERR_OK;
    i2c_busy = 1;
#ifdef USE_TICKER_FOR_TIMEOUT
    uint32_t timeout;
#else
    uint16_t timeout;
#endif

#ifdef DEBUG
    if (count > 1)
    {
        DPRINT(PSTR("%d\n"), msg[1].len);
    }
#endif
    // send start condition
    TWCR = (1 << TWSTA) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);

#ifdef USE_TICKER_FOR_TIMEOUT
    timeout = getTicker() + I2C_TIMEOUT_MS * 1000UL / TICKER_PERIOD;
    while(i2c_busy && getTicker() < timeout)
#else
    timeout = 0;
    while(i2c_busy && timeout < 8 * I2C_TIMEOUT_MS)
#endif
    {
#ifdef USE_TICKER_FOR_TIMEOUT
#else
        _delay_us(125);
        timeout++;
#endif
    }
    if (i2c_busy)
    {
        TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN);
        CHECKPOINT;
    }
    DPRINT(PSTR("%d\n"), i2c_msg_len);
    return count - i2c_msg_len;
}

ISR(TWI_vect)
{
    uint8_t twcr = (1 << TWEN);

    i2c_status = TWSR & 0xF8;
    TWCR = twcr;

    sei();

    switch(i2c_status)
    {
        case TW_REP_START:  // repeated start condition has been transmitted
        case TW_START:      // start condition has been transmitted
            DPRINT(i2c_status == TW_START ? PSTR("TW_START\n") : PSTR("TW_REP_START\n"));
            if (i2c_msg->read)
            {
                // send SLA+R
                TWDR = i2c_msg->addr | TW_READ;
            }
            else
            {
                // send SLA+W
                TWDR = i2c_msg->addr & ~TW_READ;
            }
            // start transfer
            twcr = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            i2c_data_len = i2c_msg->len;
            i2c_data = i2c_data_len > 2 ? i2c_msg->data_ptr : i2c_msg->data;
            break;

        case TW_MT_SLA_ACK: // SLA+W has been transmitted and ACK has been received.
            DPRINT(PSTR("TW_MT_SLA_ACK\n"));
            // send first data byte
            TWDR = *i2c_data;
            // start transfer
            twcr = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            i2c_data++;
            i2c_data_len--;
            break;

        case TW_MT_DATA_ACK:    // Data byte has been transmitted and ACK has been received.
            DPRINT(PSTR("TW_MT_DATA_ACK\n"));
            if (i2c_data_len == 0)
            {
                i2c_msg++;
                i2c_msg_len--;
                if (i2c_msg_len == 0)
                {
                    // send stop condition
                    twcr = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                    i2c_err = I2C_ERR_OK;
                    i2c_busy = 0;
                    break;
                }
                else if (!i2c_msg->nostart)
                {
                    // send start condition
                    twcr = (1 << TWSTA) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                    break;
                }
                i2c_data_len = i2c_msg->len;
                i2c_data = i2c_data_len > 2 ? i2c_msg->data_ptr : i2c_msg->data;
            }

            // send data
            TWDR = *i2c_data;
            // start transfer
            twcr = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            i2c_data++;
            i2c_data_len--;
            break;

        case TW_MT_SLA_NACK:    // SLA+W has been transmitted, but not acknowledged.
        case TW_MR_SLA_NACK:    // SLA+R has been transmitted, but not acknowledged.
            DPRINT(i2c_status == TW_MT_SLA_NACK ? PSTR("TW_MT_SLA_NACK\n") : PSTR("TW_MR_SLA_NACK\n"));
            // send stop condition
            twcr = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            i2c_err = I2C_ERR_SLA_NACK;
            i2c_busy = 0;
            break;

        case TW_MT_DATA_NACK:   // Data byte has been transmitted, but not acknowledged.
            DPRINT(PSTR("TW_MT_DATA_NACK\n"));
            // send stop condition
            twcr = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            i2c_err = I2C_ERR_DATA_NACK;
            i2c_busy = 0;
            break;

        case TW_MT_ARB_LOST:    // Arbitration lost while in master mode.
            DPRINT(PSTR("TW_MT_ARB_LOST\n"));
            i2c_err = I2C_ERR_ARB;
            i2c_busy = 0;
            // FIXME !!!!!!!!!!!!!!!
            break;

        case TW_MR_SLA_ACK: // SLA+R has been transmitted and ACK has been received.
            DPRINT(PSTR("TW_MR_SLA_ACK\n"));
            if (i2c_data_len > 1)
            {
                // send ACK
                twcr = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            else
            {
                // send NACK
                twcr = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;

        case TW_MR_DATA_ACK:    // Data received, ACK returned.
            DPRINT(PSTR("TW_MR_DATA_ACK\n"));
            *i2c_data++ = TWDR;
            if (i2c_data_len > 2)
            {
                // send ACK
                twcr = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            else
            {
                // send NACK
                twcr = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            i2c_data_len--;
            break;

        case TW_MR_DATA_NACK:   // Data byte has been received, but not acknowledged.
            DPRINT(PSTR("TW_MR_DATA_NACK\n"));
            *i2c_data = TWDR;
            if (i2c_msg_len > 1)
            {
                // send start condition
                twcr = (1 << TWSTA) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                i2c_msg++;
            }
            else
            {
                // send stop condition
                twcr = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                i2c_err = I2C_ERR_OK;
                i2c_busy = 0;
            }
            i2c_msg_len--;
            break;

        case TW_NO_INFO:    // No info.
            DPRINT(PSTR("TW_NO_INFO\n"));
            // no action
            break;

        case TW_BUS_ERROR:  // Bus error.
            DPRINT(PSTR("TW_BUS_ERROR\n"));
            i2c_err = I2C_ERR_BUS;
            i2c_busy = 0;
            // execute stop (bus is reset, no stop condition is sent)
            twcr = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;

        default:
            DPRINT(PSTR("%02x\n"), i2c_status);
    }

    cli();
    TWCR = twcr;
}
