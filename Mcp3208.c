/**
* @file Mcp3208.c
* @author dg1vs
* @date 2010/07/09
* @brief Handler for ADC-Converter MCP3208
*
* Driver for the Microchip MCP3208 ADC.
* MCP3208 : 12 BIT, 100ksps, 8 Channels
*/


#include <avr/io.h>
#include <avr/interrupt.h>

#include "IOPins.h"
#include "Mcp3208.h"



/** Wrapper routine for talking to an MCP3208 chip over the SPI bus.
*   Requests the ADC to perform conversion and returns the result.
*
* @param  channel selects channel. For MCP3208 ch is between 0-7 (Total 8 channels)
*
* @return The digital value of analog input on selected channel. 
* 		  Since we have 12 bit ADC the return value is between 0-4095
*
*/
uint16_t Mcp3208_StartConvReadResult(uint8_t channel)
{
	uint8_t byte,data_high,data_low;
	static uint16_t test;

	byte=0b00000110;

	if(channel>3)
		byte|=0b00000001;

    uint8_t sreg = SREG;
    cli();

	uint8_t spcr = SPCR;            // save SPI settings for SD card
    uint8_t spsr = SPSR;            // save SPI settings for SD card
    
    SPCR = 0;						// disable SPI interface
	/*
	SPI BUS CONFIGURATION MCP3208
	-----------------------------
	*Master Mode
	*MSB first
	*CPOL=0
	*CPHA=0
	*Above two implies SPI MODE0
	*SCK Freq = FCPU/16 i.e. 1MHz
	*/

	SPCR|=(1<<SPE)|(1<<MSTR)|(1<<SPR0);
	
	//SpiConfig[SPI_DEVICE_MCP3208].SetCSLine(CS_AKTIV);
	set_output_bitval(SPI_CS_MCP3208,0);
	
	//Spi_TransferByte(byte);
	SPDR = byte;
	while (!(SPSR & (1<<SPIF)));
	
	
	byte=channel<<6;

	//data_high=Spi_TransferByte(byte);
	SPDR = byte;
	while (!(SPSR & (1<<SPIF)));
	data_high = SPDR;
	
	data_high&=0b00001111;

	//data_low=Spi_TransferByte(0xFF);
	SPDR = 0xFF;
	while (!(SPSR & (1<<SPIF)));
	data_low = SPDR;

	//SpiConfig[SPI_DEVICE_MCP3208].SetCSLine(CS_PASSIV);
	set_output_bitval(SPI_CS_MCP3208,1);

	SPCR = spcr;                    // Restore settings for SD mode
    SPSR = spsr;					// Restore settings for SD mode
	
	SREG = sreg;					// Restore IRQ

	test = ((data_high<<8)|data_low);
	return test;
}


void Mcp3208_Init()
{
	Mcp3208_StartConvReadResult(0);
}

void Mcp3208_DeInit()
{

}



/*

function ShiftInADC(ADCchannel:byte):Integer;
//Sende Channel an MCP3208, erhalte Raw-Wert
//Achtung: kehrt Reihenfolge 0..7 um wg. Layout
var myCh, myCtrl: byte;
begin
myCh:=7-ADCchannel;      // Reihenfolge umkehren!
if ((myCh and $04) = 4) then
myCtrl:=%00000111;
else
myCtrl:=%00000110;
endif;
// SPI Control Register: SPIE SPE DORD MSTR CPOL CPHA SPR1 SPR0
SPCR:= %01010001; // SPI enable, Master, MSB first, CPOL 0.0, fclk/16
STRADC:=low;
SPIsendbyte:=myCtrl;
SPIbytetransfer;
SPIsendbyte:=myCh shl 6;
SPIbytetransfer;
TempW_high:=SPIreceivebyte and $0f;
SPIsendbyte:=0;
SPIbytetransfer;
TempW_low:=SPIreceivebyte;
nop;
STRADC:=high;
SPCR:= %01011100; // SPI enable, Master, MSB first, CPOL 1.1, fclk/4
return(TempI);
end;


procedure ReadADC(myADC:byte);    // liefert Param!
var ADCtemp:Integer;
begin
{
	if IntegrateADC then
	ADCtemp:=ShiftInADC(myIndex);
	ADCtemp:=ADCtemp+ShiftInADC(myIndex);
	ADCtemp:=ADCtemp+ShiftInADC(myIndex);
	ADCtemp:=ADCtemp+ShiftInADC(myIndex);
	ADCtemp:=ADCtemp shr 2;
	else
	endif;
}
ADCtemp:=ShiftInADC(myADC);
ADCrawArray[myADC]:=ADCtemp;
ADCtemp:=ADCtemp+ADCoffsets[myADC];
Param:=Float(ADCtemp)*ADCscales[myADC]*ADCbaseScale; //FS/2 Grundskalierung 12 Bit
end;


  InitADCBaseScale: Float  = 1/409.5;      // 2442 für FracScale=1000
  InitDACBaseScale: LongInt  = 269541; // berücksichtigt Grundverstärkung

  InitDACvalueArray      : array[0..7] of Float =
  (0,0,0,0,0,0,0,0);

  // ADCoffsets, 10..17 AD12  (-2145 für bipolar Rgnd=10k/Rref=6k8/Rv=30k)
  InitADCOffsetArray      : array[0..7] of Integer =
  (0, 0, 0, 0,
  0, 0, 0, 0);
  
  // ADCscales 10..17 (2.1 für bipolar 10V, Rgnd=10k/Rref=6k8/Rv=30k)
  InitADCscaleArray      : array[0..7] of Float =
  (1.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 1.0);
  
  

    100..107: // DEF 0 ADC Offset
    ParamInt:=ADCOffsets[myIndex];
    WriteParamIntSer;
    |
    110..117: // DEF 10 ADC Skalierung
    Param:=ADCScales[myIndex];
    WriteParamSer;
    |

    100..107:
    ADCOffsets[myIndex]:=ParamInt;
    if EEunLocked then
    InitADCOffsetArray[myIndex]:=ParamInt;
    endif;
    |
    110..117:
    ADCScales[myIndex]:=Param;
    if EEunLocked then
    InitADCScaleArray[myIndex]:=Param;
    endif;
    |


function GetNewValue(mySubCh:Integer):boolean;
//Werte in Param holen, wenn in SubCh; Raw-Werte in ParamInt
//return-Wert True, wenn Integer
var myResultIsInteger:boolean;
myIndex:Byte;
begin
ParamInt:=0;
Param:=0;
myResultIsInteger:=true;
myIndex:=byte(mySubCh mod 10);  // angespr. Register 0..9 errechnen aus SubCh-Rest
case mySubCh of
8:
GetFreqTimerString; // Liefert auch Param
|
10..17, 50..57: // aktuellen Wert holen
ReadADC(myIndex);
if mySubCh >=50 then
ParamInt:=ADCrawArray[myIndex];
else
ParamToParamFrac;
myResultIsInteger:=false;
endif;
|
20..23: // aktuellen Wert holen
ParamFrac:=DACValues[myindex];
ParamFracToParam;
myResultIsInteger:=false;
|
30..37:
ParamInt:=Integer(GetPort(myIndex));
|
40..47: //DIR Port direction
ParamInt:=integer(DDRArray[myIndex]);
|
60..79:
ParamLong:=ReceiveFPGA(byte(mySubCh-60));
ParamInt:=Integer(ParamLong);
|
300..309: // Registerwert holen
Param:=RegisterArray[myIndex];
ParamToParamFrac;
myResultIsInteger:=false;
|
endcase;
ParamByte:=lo(ParamInt);
if myResultIsInteger then
ParamFrac:=LongInt(ParamInt)*FracScale;  // für Display
endif;
return (myResultIsInteger);
end;
*/
