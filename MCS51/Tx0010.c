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
				P1.7 -> Arduino Pin 9 (Data Rx Ack 0:Ack   1:Standby)
			
				P2.0 -> LED (RED) - Dancer1 wireless data transmitting
 				P2.1 -> LED (YLW) - Dancer2 wireless data transmitting
				P2.2 -> LED (GRN) - Dancer3 wireless data transmitting
				P2.3 -> LED (YLW) - Dancer4 wireless data transmitting
				P2.4 -> LED (RED) - Dancer5 wireless data transmitting
				P2.5 -> LED (RED) - Default ElWire Sequence -> All On
				P2.6 -> LED (YLW) - Diagnostic in process
				P2.7 <- Key#2 In  - Default ElWire Sequence -> All On

				INT0 <- Key#1 In  - Diagnostic (Interrupt 0)

 Jumpers:		N/A

 X'tal:			14.07456MHz (for baud rate: 38400bps)
 **************************************************************/
#include <STC89.H>

#define	LEDOn				0
#define	LEDOff				1

#define PORT_Arduino		P1

#define DATA_Rdy			P16
#define	DATA_Ack			P17

#define	PORT_Ctrl_Status	P2

#define	LED_Dancer1			P20
#define LED_Dancer2			P21
#define LED_Dancer3	 		P22
#define LED_Dancer4	  		P23
#define LED_Dancer5			P24

#define	LED_DefaultSeq		P25
#define	LED_Diagnostic		P26

#define	KEY_DefaultSeq		P27

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

void serialTxChar(unsigned char c)
{
    SBUF = c;			// Load data into Serial Buffer and transmittion will start automatically

    while(!TI);			// Wait for transmittion finished
    TI = 0;			  	// Reset transmit flag for next transmittion

} /* serialTxChar */

void initBoard(void)
{
	PORT_Arduino 	 = 0xff;	// Set Port ready for Input from Arduino, set DataAck=1 (Standy)
	PORT_Ctrl_Status = 0xff;	// Turn all LED Off and ready to read Key#2	status
	
} /* initBoard */


unsigned char keyDefaultSeq(void)
{
	return ((KEY_DefaultSeq) & 1);

} /* keyDefaultSeq */

void ledDancerOn(unsigned arduinoData)
{
	arduinoData &= 0x7;		// Filter out the Dancer#

	switch (arduinoData)
	{
		case 0:
			LED_Dancer1 = LEDOn;
			break;
		
		case 1:
			LED_Dancer2 = LEDOn;
			break;

		case 2:
			LED_Dancer3 = LEDOn;
			break;

		case 3:
			LED_Dancer4 = LEDOn;
			break;

		case 4:
			LED_Dancer5 = LEDOn;
			break;

	  	case 7:
			LED_Dancer1 = LEDOn;
			LED_Dancer2 = LEDOn;
			LED_Dancer3 = LEDOn;
			LED_Dancer4 = LEDOn;
			LED_Dancer5 = LEDOn;
			break;

		default:
			break;
	}
} /* ledDancerOn */

void ledDancerAllOff(void)
{
	LED_Dancer1 = LEDOff;
	LED_Dancer2 = LEDOff;
	LED_Dancer3 = LEDOff;
	LED_Dancer4 = LEDOff;
	LED_Dancer5 = LEDOff;

} /* ledDancerAllOff */

void sysDiagLedRolling(void)
{
	unsigned int t;

	t=500;			// Delay 500ms
	
	LED_Dancer1 = LEDOn;
	delayms(t);
	
	LED_Dancer2 = LEDOn;
	delayms(t);
	
	LED_Dancer3 = LEDOn;
	delayms(t);
	
	LED_Dancer4 = LEDOn;
	delayms(t);
	
	LED_Dancer5 = LEDOn;
	delayms(t);
	
	ledDancerAllOff();

} /* sysDiagLedRolling */

void sysDiagLedFlashing(void)
{
	unsigned int t, n;

	t=500;			// Delay 500ms
	
	for (n=0; n<2; n++)
	{
		LED_Dancer1 = LEDOn;
		delayms(t);
		LED_Dancer1 = LEDOff;
		delayms(t);
		
		LED_Dancer2 = LEDOn;
		delayms(t);
		LED_Dancer2 = LEDOff;
		delayms(t);
		
		LED_Dancer3 = LEDOn;
		delayms(t);
		LED_Dancer3 = LEDOff;
		delayms(t);
		
		LED_Dancer4 = LEDOn;
		delayms(t);
		LED_Dancer4 = LEDOff;
		delayms(t);
		
		LED_Dancer5 = LEDOn;
		delayms(t);
		LED_Dancer5 = LEDOff;
		delayms(t);
	}

} /* sysDiagLedFlashing */
		
void sysDiagnostic(void)
{
	LED_Diagnostic = LEDOn;

	sysDiagLedFlashing();

	LED_Diagnostic = LEDOff;

} /* sysDiagnostic */

void elwireDefaultSeq(void)
{
	LED_DefaultSeq = LEDOn;

	sysDiagLedRolling();

	LED_DefaultSeq = LEDOff;

} /* elwireDefaultSeq */

unsigned char arduinoGetData(void)
{
	unsigned char arduinoData;

	if (!DATA_Rdy)			// No data available
	{
		return 0;
	}

	arduinoData = PORT_Arduino & 0x7f;	// Read Data from Arduino

	DATA_Ack = 0; 			// Send Ack to Arduino
	
	while (DATA_Rdy); 		// Wait for Arduino ack my ACK
	DATA_Ack = 1;		  	// Reset ACK to standby

	return arduinoData;		// Keep bit-6 On to indicate data available from Arduino

} /* arduinoDataReady */



void main(void)
{
	unsigned char arduinoMidi;

    uartInit38400();

	initBoard();

	for (;;)
	{
		if (keyDefaultSeq())
		{
			elwireDefaultSeq();
			continue;
		}

		arduinoMidi = arduinoGetData();

		arduinoMidi = 'A';

		if (arduinoMidi)
		{
			ledDancerOn(arduinoMidi);

			serialTxChar(arduinoMidi);

			ledDancerAllOff();
		}
	}		
} /* main */

//void ISRsysDiagnostic() interrupt 0	using 2
//{
//	sysDiagnostic();
//
//} /* ISRsysDiagnostic */