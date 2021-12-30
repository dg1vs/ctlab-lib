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

#ifndef __LCD_H__
#define __LCD_H__

#include <inttypes.h>
#include <avr/pgmspace.h>

#define COLUMN_MAX      8
#define LINE_MAX        2
#define LCD_PCA9555D_ADDRESS 0x40

#define SCROLL_DISTANCE 3

#define BUTTON_DOWN     0x01
#define BUTTON_UP       0x02
#define BUTTON_ENTER    0x04

#define CURSOR_SOLID    0
#define CURSOR_CONCAVE  1
#define CURSOR_HATCHED  2
#define BLOCK_SOLID     255
#define BLOCK_HATCHED   3
#define SMALL_R_HIGH    4
#define SMALL_R_LOW     5
#define SMALL_A_INV     6
#define SMALL_A         7

#ifdef __cplusplus
extern "C" {
	#endif

uint8_t Lcd_Init(void);
void Lcd_Write(uint8_t, uint8_t, uint8_t, const char*);
void Lcd_Write_P(uint8_t, uint8_t, uint8_t, const char*);
uint8_t Lcd_GetButton(void);

extern const PROGMEM char ucWhites[];

#ifdef __cplusplus
}
#endif

#endif /* _LCD_H_ */

