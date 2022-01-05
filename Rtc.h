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

#ifndef _RTC_H_
#define _RTC_H_

#define DS1302_RST PA2
#define DS1302_CLK PB7
#define DS1302_IO  PB5

#define RTC_COMMAND_SECONDS 	0x80
#define RTC_COMMAND_MINURES 	0x82
#define RTC_COMMAND_HOURS   	0x84
#define RTC_COMMAND_DATE    	0x86
#define RTC_COMMAND_MONTH   	0x88
#define RTC_COMMAND_DAY     	0x8A
#define RTC_COMMAND_YEAR    	0x8C
#define RTC_COMMAND_WP      	0x8E
#define RTC_COMMAND_CONFIG  	0x90
#define RTC_COMMAND_BURST_WRITE 0xBE
#define RTC_COMMAND_BURST_READ 	0xBF


#ifdef __cplusplus
extern "C" {
#endif

extern void Ds1302_SetChip(uint8_t ucCommand, uint8_t ucDataOut);
extern uint8_t Ds1302_GetChip(uint8_t ucCommand);

extern void Ds1302_SetChipBurst(uint8_t ucData[]);
extern void Ds1302_GetChipBurst(uint8_t ucData[]);

extern void Rtc_ReadChipTime(void);
extern void Rtc_SetChipTime(void);

extern void Rtc_CheckChipPresent(void);

extern uint8_t g_ucRTCpresent;

typedef struct
{
    uint8_t ucSeconds;
    uint8_t ucMinutes;
    uint8_t ucHours;
    uint8_t ucDay;
    uint8_t ucMonth;
    uint8_t ucYear;
    uint16_t uiYearWord;
} RTC_STRUCT;

extern volatile RTC_STRUCT gRTC;

#ifdef __cplusplus
}
#endif

#endif /* _RTC_H_ */