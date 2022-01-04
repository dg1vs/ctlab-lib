/*
 * Copyright (c) 2007 by Hartmut Birr, Thoralt Franz
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

#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <inttypes.h>

#ifdef __cplusplus
    extern "C" {
#endif

void Encoder_MainFunction(void);
int16_t Encoder_GetPosition(uint8_t);
int16_t Encoder_GetAndResetPosition(void);
void Encoder_SetAcceleration(uint16_t uI1, uint16_t uI2, uint16_t uI3, uint16_t uI4);

#ifdef __cplusplus
    extern "C" {
#endif

#endif /* _ENCODER_H_ */
