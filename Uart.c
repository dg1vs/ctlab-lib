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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Uart.h"

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)

#define UART_UCSRA      UCSRA
#define UART_UCSRB      UCSRB
#define UART_UCSRC      UCSRC
#define UART_UBRRL      UBRRL
#define UART_UBRRH      UBRRH
#define UART_UCSRC_INIT ((1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0))
#define UART_U2X        U2X
#define UART_RXEN       RXEN
#define UART_RXC        RXC
#define UART_RXCIE      RXCIE
#define UART_TXEN       TXEN
#define UART_UDR        UDR
#define UART_DOR        DOR
#define UART_FE         FE
#define UART_PE         PE
#define UART_UDRE       UDRE
#define UART_UDRIE      UDRIE

#elif defined (__AVR_ATmega168__) || defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__) || defined(__AVR_AT90CAN128__) || defined(__AVR_ATmega1284P__)

#define UART_UCSRA      UCSR0A
#define UART_UCSRB      UCSR0B
#define UART_UCSRC      UCSR0C
#define UART_UBRRL      UBRR0L
#define UART_UBRRH      UBRR0H
#define UART_UCSRC_INIT ((1<<UCSZ01)|(1<<UCSZ00))
#define UART_U2X        U2X0
#define UART_RXEN       RXEN0
#define UART_RXC        RXC0
#define UART_RXCIE      RXCIE0
#define UART_TXEN       TXEN0
#define UART_UDR        UDR0
#define UART_DOR        DOR0
#define UART_FE         FE0
#define UART_PE         UPE0
#define UART_UDRE       UDRE0
#define UART_UDRIE      UDRIE0
#else
#error Please define your UART registers
#endif

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
#define UART_RX_vect    USART_RXC_vect
#define UART_UDRE_vect  USART_UDRE_vect
#elif defined (__AVR_ATmega168__)
#define UART_RX_vect    USART_RX_vect
#define UART_UDRE_vect  USART_UDRE_vect
#elif defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__) || defined(__AVR_AT90CAN128__) || defined(__AVR_ATmega1284P__)
#define UART_RX_vect    USART0_RX_vect
#define UART_UDRE_vect  USART0_UDRE_vect
#else
#error Please define your UART interrupt vectors
#endif

#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE 128
#endif

#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 128
#endif

uint8_t UartRxBuffer[UART_RX_BUFFER_SIZE];
volatile uint8_t UartRxCount;
volatile uint8_t UartRxCurrent;

uint8_t UartTxBuffer[UART_TX_BUFFER_SIZE];
#if UART_TX_BUFFER_SIZE >= 256
volatile uint16_t UartTxCount;
volatile uint16_t UartTxCurrent;
#else
volatile uint8_t UartTxCount;
volatile uint8_t UartTxCurrent;
#endif
volatile uint8_t UartStatus;

void Uart_Init(void)
{
    Uart_InitUBRR(INIT_UBRR);
}

void Uart_InitUBRR(uint16_t ubrr)
{
    uint8_t sreg = SREG;

    /* disable interrupts during initialisation */
    cli();

    /* disable the UART */
    UART_UCSRA = UART_UCSRB = UART_UCSRC = 0;

    /* set the baudrate */
    UART_UBRRH = ubrr / 256;
    UART_UBRRL = ubrr % 256;
#ifdef UART_2X
    UART_UCSRA = (1<<UART_U2X);
#endif

    /* set parity, stop bits and data length */
    UART_UCSRC = UART_UCSRC_INIT;

    Uart_ResetRxBuffer();
    Uart_ResetTxBuffer();

    /* restore the state of the interrupt */
    SREG = sreg;
}

void Uart_ResetRxBuffer(void)
{
    uint8_t sreg = SREG;

    cli();
    UART_UCSRB &= ~((1 << UART_RXCIE)|(1 << UART_RXEN));

    UartRxCount = 0;
    UartRxCurrent = 0;

    UartStatus &= ~(UART_RX_BUFFER_OVERFLOW|UART_PARITY_ERROR|UART_DATA_OVERRUN|UART_FRAME_ERROR);

    UART_UCSRB |= (1<<UART_RXCIE) | (1<<UART_RXEN);
    SREG = sreg;
}

void Uart_ResetTxBuffer(void)
{
    uint8_t sreg = SREG;

    cli();
    UART_UCSRB &= ~((1<<UART_UDRIE)|(1<<UART_TXEN));

    UartTxCount = 0;
    UartTxCurrent = 0;

    UART_UCSRB |= (1 << UART_TXEN);
    SREG = sreg;
}

ISR(UART_RX_vect)
{
    uint8_t ucsra;

    ucsra = UART_UCSRA;

    do
    {
        UartRxBuffer[UartRxCurrent] = UART_UDR;

        if (UartRxCurrent < UART_RX_BUFFER_SIZE - 1)
        {
            UartRxCurrent++;
        }
        else
        {
            UartRxCurrent = 0;
        }
        if (UartRxCount >= UART_RX_BUFFER_SIZE)
        {
            UartStatus |= UART_RX_BUFFER_OVERFLOW;
        }
        else
        {
            UartRxCount++;
        }
        if (ucsra & (1 << UART_DOR))
        {
            UartStatus |= UART_DATA_OVERRUN;
        }
        if (ucsra & (1 << UART_FE))
        {
            UartStatus |= UART_FRAME_ERROR;
        }
        if (ucsra & (1 << UART_PE))
        {
            UartStatus |= UART_PARITY_ERROR;
        }

        ucsra = UART_UCSRA;
    }
    while (ucsra & (1<<UART_RXC));
}

uint8_t Uart_GetRxData(uint8_t *buffer, uint8_t size)
{
    uint8_t sreg;
    uint8_t count = 0;

    while (UartRxCount > 0 && count < size)
    {
        sreg = SREG;
        cli();

        if (UartRxCurrent >= UartRxCount)
        {
            *buffer = UartRxBuffer[UartRxCurrent - UartRxCount];
        }
        else
        {
            *buffer = UartRxBuffer[UART_RX_BUFFER_SIZE + UartRxCurrent - UartRxCount];
        }
        UartRxCount--;

        SREG = sreg;

        count++;
        buffer++;
    }
    return count;
}

ISR(UART_UDRE_vect)
{
    do
    {
        if (UartTxCount == 0)
        {
            UART_UCSRB &= ~(1<<UART_UDRIE);
            break;
        }
        else
        {
            UART_UDR = UartTxBuffer[UartTxCurrent];
            if (UartTxCurrent < UART_TX_BUFFER_SIZE - 1)
            {
                UartTxCurrent++;
            }
            else
            {
                UartTxCurrent=0;
            }
            UartTxCount--;
        }
    }
    while (UART_UCSRA & (1<<UART_UDRE));
}

uint8_t Uart_SetTxData(uint8_t *buffer, uint8_t size, uint8_t wait)
{
    uint8_t count = 0;
#if 1
    uint8_t sreg;
    do
    {
        sreg = SREG;
        cli();
        while (UartTxCount < UART_TX_BUFFER_SIZE && count < size)
        {
            if (UartTxCurrent + UartTxCount >= UART_TX_BUFFER_SIZE)
            {
                UartTxBuffer[UartTxCurrent + UartTxCount - UART_TX_BUFFER_SIZE] = *buffer;
            }
            else
            {
                UartTxBuffer[UartTxCurrent + UartTxCount] = *buffer;
            }
            UartTxCount++;

            UART_UCSRB |= (1<<UART_UDRIE);

            SREG = sreg;

            buffer++;
            count++;

            sreg = SREG;
            cli();
        }
        SREG = sreg;
    }
    while (wait && count < size);
#else
    while (count < size)
    {
        while(!(UART_UCSRA & (1<<UART_UDRE)));
        UART_UDR = *buffer;
        buffer++;
        count++;
    }
#endif
    return count;
}

uint8_t Uart_GetStatus(void)
{
    return UartStatus;
}

uint8_t Uart_GetRxCount(void)
{
    return UartRxCount;
}

uint8_t Uart_GetTxCount(void)
{
    return UartTxCount;
}
