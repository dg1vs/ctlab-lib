/**
* @file Mcp3208.c
* @author dg1vs
* @date 2010/07/09
* @brief Handler for ADC-Converter MCP3208
*
* Driver for the Microchip MCP3208 ADC.
* MCP3208 : 12 BIT, 100ksps, 8 Channels
*/


#ifndef _MCP3208_H_
#define _MCP3208_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


void Mcp3208_Init(void);
void Mcp3208_DeInit(void);

uint16_t Mcp3208_StartConvReadResult(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* _MCP3208_H_ */
