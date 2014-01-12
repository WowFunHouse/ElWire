/**************************************************************
 Title:			Parallel to Wireless Transmitter		
 
 File:			tx0001.c

 Version:		0.01
 Description:	Receive parallel packet from Arduino
 				and transmit through WSN-02/03 433MHz transmitter

 Created on:	2014-01-11
 Created by:	Michael

 Board:			Custom made MCS51 WSN-02/03 control system

 Connections:	P1.0 <- Arduino Pin 2 (Dancer ID Bit0)
 				P1.1 <- Arduino Pin 3 (Dancer ID Bit1)
				P1.2 <- Arduino Pin 4 (Dancer ID Bit2)
				P1.3 <- Arduino Pin 5 (ElWire Index Bit 0)
				P1.4 <- Arduino Pin 6 (ElWire Index Bit 1)
				P1.5 <- Arduino Pin 7 (ElWire Mode 1:On 0:Off)
				P1.6 <- Arduino Pin 8 (Data Ready  1:Ready)
				P1.7 -> Arduino Pin 9 (Data Rx Ack 0:Ack 1:Standby)
			
				P2.0 -> LED (RED) - Dancer1 wireless data transmitting
 				P2.1 -> LED (YLW) - Dancer2 wireless data transmitting
				P2.2 -> LED (GRN) - Dancer3 wireless data transmitting
				P2.3 -> LED (YLW) - Dancer4 wireless data transmitting
				P2.4 -> LED (RED) - Dancer5 wireless data transmitting
				P2.5 -> LED (RED) - Default ElWire Sequence -> All On
				P2.6 -> LED (YLW) - Diagnostic in process
				P2.7 <- Key In    - Default ElWire Sequence -> All On

				INT0 <- Key In - Diagnostic (Interrupt 0)

 Jumpers:		N/A

 X'tal:			14.07456MHz (for baud rate: 38400bps)
 **************************************************************/
#include <STC89.H>

void uartInit38400(void)
{
	// X'tal: 14.7456MHz
	// 38400 Baud, 8 Bit Data, No Parity, 1 Stop Bit

    TMOD = T1_M2;		// Set Timer 2 to Mode 2 (8-bit counter auto reload)
    SCON = 0x40;		// 8 bit data
    TH1  = 0xFF;
    TL1  = TH1;
    PCON = 0x00;		// No double baud rate

	EA   = 1;			// Enable All Interrupts
	ES   = 1;		    // Enable Serial Port Interrupt

    TR1  = 1;			// Start Timer1 as baud rate generator

} /* uartInit38400 */

void serialTxOneBye(unsigned char c)
{
    SBUF = c;			// Load data into Serial Buffer and transmittion will start automatically

    while(!TI);			// Wait for transmittion finished
    TI = 0;			  	// Reset transmit flag for next transmittion

} /* serialTxOneByte */

void delayms(unsigned int t)   // 1ms delay: torrance -0.651041666667us
{
    unsigned int d;
	unsigned char a,b;

	for (d=0; d<t; d++)
	{
	    for(b=245;b>0;b--)
	        for(a=1;a>0;a--);
	}
} /* delayms */

void main(void)
{
	unsigned char c;

    uartInit38400();

	for (;;)
	{
		P10 = 0;
		P20 = 0;

		delayms(500);

		for (c=32; c<127; c++)
			serialTxOne(c);

		P10 = 1;
		P20 = 1;

		delayms(250);
	}		
} /* main */

