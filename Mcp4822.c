/**
* @file Mcp4822.c
* @author dg1vs
* @date 2010/07/09
* @brief Handler for DAC-Converter MCP4822
*
* Driver for the Microchip MCP4822 DAC, which is an dual 12-bit
* DAC with an SPI interface.
* The MCP4822 has a pin for latching the DAC output, which has to be handled.
*
*/


#include <avr/io.h>
#include <avr/interrupt.h>

#include "debug.h"
#include "Mcp4822.h"
#include "IOPins.h"

// TODO DKS 
// We have 2 MCP4822, that means we need 2 instanzes of the driver, use the idea of Spi_ConfigType SpiConfig[];
void Mcp4822_Transmit(uint8_t instanz, uint8_t channel, uint8_t gain, uint8_t output_enable, uint16_t data)
{
	// Make sure each of our flags is either a 0 or a 1:
	channel &= 0x01;
	gain &= 0x01;
	output_enable &= 0x01;
	// Make sure our data is only 12 bits
	data &= 0xFFF;
	// Now pack the data with our flags:
	// channel			bit 15
	// gain				bit 13
	// output_enable	bit 12
	uint8_t lowByte = data & 0xff;
	uint8_t highByte = ((data >> 8) & 0xff) | channel << 7 | gain << 5 | output_enable << 4;

	// Einfügen debug lowByte / Highbyte
    uint8_t sreg = SREG;
    cli();

    uint8_t spcr = SPCR;            // save SPI settings 
    uint8_t spsr = SPSR;            // save SPI settings 
    SPCR = 0;						// disable SPI interface
	
	/*
	SPI BUS CONFIGURATION MCP4822
	-----------------------------
	*Master Mode
	*MSB first
	*CPOL=0
	*CPHA=0
	*Above two implies SPI MODE0
	*SCK Freq = FCPU/4 
	*/
	SPCR|=(1<<SPE)|(1<<MSTR);
	
	// TODO B
	//SpiConfig[SPI_DEVICE_MCP4822_A].SetCSLine(CS_AKTIV);
	if (instanz == 0)
		set_output_bitval(SPI_CS_MCP4822_A,0);
	else
		set_output_bitval(SPI_CS_MCP4822_B,0);
		
	SPDR = highByte;
	while (!(SPSR & (1<<SPIF)));
	
	SPDR = lowByte;
	while (!(SPSR & (1<<SPIF)));


	//todo SPI_LDAC_MCP4822_PORT

	//SpiConfig[SPI_DEVICE_MCP4822_A].SetCSLine(CS_PASSIV);
	if (instanz == 0)
		set_output_bitval(SPI_CS_MCP4822_A,1);
	else
		set_output_bitval(SPI_CS_MCP4822_B,1);
	

	SPCR = spcr;                    // Restore SPI-settings 
    SPSR = spsr;					// Restore SPI-settings 
	
	SREG = sreg;					// Restore IRQ
}

void Mcp4822_Init(void)
{
	activate_output_bit(SPI_CS_MCP4822_A);
	activate_output_bit(SPI_CS_MCP4822_B);
	activate_output_bit(SPI_LDAC_MCP4822);
	set_output_bitval(SPI_LDAC_MCP4822,0);  // fix to low -> transfer to dac register on rising edge of CS
	// just in case
}

void Mcp4822_DeInit(void)
{
	deactivate_output_bit(SPI_CS_MCP4822_A);
	deactivate_output_bit(SPI_CS_MCP4822_B);
	deactivate_output_bit(SPI_LDAC_MCP4822);
}

