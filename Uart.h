/*
 * Copyright (c) 2006, 2007, 2008 by Hartmut Birr
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

#ifndef _UART_H_
#define _UART_H_

#include <inttypes.h>

#ifndef F_CPU
#error Please define F_CPU
#endif /* !F_CPU */

#ifndef UART_BAUDRATE
#define UART_BAUDRATE       38400
//#define UART_BAUDRATE     57600
//#define UART_BAUDRATE     115200UL
//#define UART_BAUDRATE     9600
#endif


#ifdef UART_2X
#define INIT_UBRR       (F_CPU / 8 / UART_BAUDRATE - 1)
#else
#define INIT_UBRR       (F_CPU / 16 / UART_BAUDRATE - 1)
#endif



#define UART_RX_BUFFER_OVERFLOW 0x01
#define UART_PARITY_ERROR       0x02
#define UART_DATA_OVERRUN       0x04
#define UART_FRAME_ERROR        0x08

#ifdef __cplusplus
extern "C" {
#endif

    void Uart_Init(void);
    void Uart_InitUBRR(uint16_t ubrr);
    uint8_t Uart_GetRxData(uint8_t *buffer, uint8_t size);
    uint8_t Uart_SetTxData(uint8_t *buffer, uint8_t size, uint8_t wait);
    uint8_t Uart_GetStatus(void);
    uint8_t Uart_GetRxCount(void);
    uint8_t Uart_GetTxCount(void);
    void Uart_ResetRxBuffer(void);
    void Uart_ResetTxBuffer(void);

#ifdef __cplusplus
}
#endif

#endif
