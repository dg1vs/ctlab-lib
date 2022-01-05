/**
* @file Mcp4822.h
* @author dg1vs
* @date 2010/07/09
* @brief Handler for DAC-Converter MCP4822
*
*/

#ifndef _MCP4822_H_
#define _MCP4822_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Wrapper routine for talking to an MCP4822 chip over the SPI bus.
*
* @param  channel selects which channel is being written to: \n
*	0 = A \n
*	1 = B \n
* @param gain is a user-selectable gain of 1x or 2x:
*	0 = 2x Vref
*	1 = 1x Vref
* @param output_enable determines whether a given channel is outputting
*   a voltage, or whether the output goes high-impedance:
*	0 = Output High-Z
*	1 = Output Enabled
* @param data the value to be converted
*
* @return void
*
*/
void Mcp4822_Transmit(uint8_t instanz, uint8_t channel, uint8_t gain, uint8_t output_enable, uint16_t data);

void Mcp4822_Init(void);

void Mcp4822_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _MCP4822_H_ */
