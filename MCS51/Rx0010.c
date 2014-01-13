/**************************************************************
 Title:			El-Wire Dancer Wireless Receiver		

 
 File:			rx0001.c

 Version:		0.01
 Description:	Receive packet wireless from transmitter
 				through WSN-02/03 433MHz wireless module

 Created on:	2014-01-12
 Created by:	Michael 
 
 
 Board:			Custom made MCS51 WSN-02/03 control system

 Connections:	P1.0 -> El-Wire Blue
 				P1.1 -> El-Wire Green
				P1.2 -> El-Wire Aux Color
				P1.3 -> El-Wire Eye-Glasses

				P2.0 -> LED (RED) - Dancer1 wireless data transmitting
 				P2.1 -> LED (YLW) - Dancer2 wireless data transmitting
				P2.2 -> LED (GRN) - Dancer3 wireless data transmitting
				P2.3 -> LED (YLW) - Dancer4 wireless data transmitting
				P2.4 -> LED (RED) - Dancer5 wireless data transmitting
				P2.5 -> LED (RED) - Default ElWire Sequence -> All On
				P2.6 -> LED (YLW) - Diagnostic in process
				P2.7 <- Key#2 In  - Default ElWire Sequence -> All On

				INT0 <- Key#1 In  - Diagnostic (Interrupt 0)

				P3.0 -> Serial Port Rx -> WSN-02/03 Tx Pin5
				P3.1 -> Serial Port Tx -> WSN-02/03 Rx Pin4
				P3.7 -> WSN-02/03 NRST Reset Pin6 (Reset:Low)

 Jumpers:		N/A

 X'tal:			14.07456MHz (for baud rate: 38400bps)
 **************************************************************/
#include <STC89.H>

#define	DANCER_ID			0

#undef	DEBUG
#define	DEBUG_ARDUINO_OFF
#undef	DEBUG_LED_PTN
#define	DEBUG_SLOW

#define	DEBUG_DELAYMS		10
#define	DEBUG_PKT			0x48

#define	LED_ON				0
#define	LED_OFF				1

#define	WSN_RST				P37

#define ELWIRE_PORT			P2

#define	ELWIRE_BLU_SW		P20
#define	ELWIRE_GRN_SW		P21
#define	ELWIRE_AUX_SW		P22
#define	ELWIRE_GLS_SW		P23

#define	ELWIRE_BLU_LED		P24
#define	ELWIRE_GRN_LED		P25
#define	ELWIRE_AUX_LED		P26
#define	ELWIRE_GLS_LED		P27

#define	KEY_AutoSeq			P27

#define	ELWIRE_ON			1;				// Turn on  El-Wire
#define	ELWIRE_OFF			0;				// Turn off El-Wire

#define	ELWIRE_ALL_ON		0x0f;			// Turn on  all El-Wires and LEDs
#define	ELWIRE_ALL_OFF		0xf0;			// Turn off all El-Wires and LEDs

#define	ELWIRE_PKT_ON		0x80
#define	ELWIRE_PKT_OFF		0x00

#define	ELWIRE_PKT_BLU		0x00			// El-Wire Blue
#define	ELWIRE_PKT_GRN		0x01			// El-Wire Green
#define	ELWIRE_PKT_AUX		0x02			// El-Wire Aux Color
#define	ELWIRE_PKT_GLS		0x03			// El-Wire Eye-Glasses

unsigned char sysDiagReq=0;
unsigned char dancerSerialDataRdy;
unsigned char dancerSerialDataRxd;

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

	IT0	 = 1;			// INT0: Falling edge trigger
	EX0	 = 1;			// Enable External Interrupt INT0
	ES   = 1;		    // Disable Serial Port Interrupt
	EA   = 1;			// Enable All Interrupts

    TR1  = 1;			// Start Timer1 as baud rate generator

} /* uartInit38400 */

void dancerBoardInit(void)
{
	ELWIRE_PORT		 = 0;		// Turn off all EL-Wires

	WSN_RST			 = 0;		// Reset WSN-02/03 Wireless module
	delayms(10);				// Delay 10ms
	WSN_RST			 = 1;		// Normal Operation
	delayms(10);				// Wait for WSN-02/03 ready
	
} /* dancerBoardInit */

void dancerElWirePattern(unsigned char pattern)
{
	unsigned char elwire, led;

	pattern &= 0x0f;			// Lower 4 bits only for controlling El-Wires

	led = pattern << 4;
	elwire = led | pattern;

#ifdef DEBUG
	DEBUG_PORT	= elwire;
#endif

	ELWIRE_PORT = elwire;

} /* dancerElWirePattern */

void dancerElWireOn(unsigned char elwireIdx)
{
	switch (elwireIdx)
	{
		case ELWIRE_PKT_BLU:
			ELWIRE_BLU_SW  = ELWIRE_ON;
			ELWIRE_BLU_LED = LED_ON;
			break;

		case ELWIRE_PKT_GRN:
			ELWIRE_GRN_SW  = ELWIRE_ON;
			ELWIRE_GRN_LED = LED_ON;
			break;

		case ELWIRE_PKT_AUX:
			ELWIRE_AUX_SW  = ELWIRE_ON;
			ELWIRE_AUX_LED = LED_ON;
			break;
		
		case ELWIRE_PKT_GLS:
			ELWIRE_GLS_SW  = ELWIRE_ON;
			ELWIRE_AUX_LED = LED_ON;
			break;

		default:
			break;
		}
} /* dancerElwireOn */

void dancerElWireOff(unsigned char elwireIdx)
{
	switch (elwireIdx)
	{
		case ELWIRE_PKT_BLU:
			ELWIRE_BLU_SW  = ELWIRE_OFF;
			ELWIRE_BLU_LED = LED_OFF;
			break;

		case ELWIRE_PKT_GRN:
			ELWIRE_GRN_SW  = ELWIRE_OFF;
			ELWIRE_GRN_LED = LED_OFF;
			break;

		case ELWIRE_PKT_AUX:
			ELWIRE_AUX_SW  = ELWIRE_OFF;
			ELWIRE_AUX_LED = LED_OFF;
			break;
		
		case ELWIRE_PKT_GLS:
			ELWIRE_GLS_SW  = ELWIRE_OFF;
			ELWIRE_AUX_LED = LED_OFF;
			break;

		default:
			break;
		}
} /* dancerElwireOff */

void dancerElWireDiag(void)
{
	unsigned char elwire;
	unsigned char i, n;
	unsigned int  t;

	i=1;
	t=100;									// Delay 100ms per El-Wire

	// Turn off all wires
	ELWIRE_PORT = ELWIRE_ALL_OFF;			// Turn off the El-Wire in reversed order		

	for (n=0; n<3; n++)
	{	  
		for (elwire=0; elwire<4; elwire++)
		{
			dancerElWirePattern(i << elwire);	// Turn on each El-Wire
			delayms(t);
		}
	
		for (i=0; i<3; i++)
		{
			ELWIRE_PORT = ELWIRE_ALL_OFF;
			delayms(t);
			ELWIRE_PORT = ELWIRE_ALL_ON;
			delayms(t);
		}
		delayms(2*t);
	}

	ELWIRE_PORT = ELWIRE_ALL_OFF;

} /* elwireDiag */
		
void dancerSysDiag(void)
{
	dancerElWireDiag();

} /* dancerSysDiag */

unsigned char dancerSerialGetByte()
{
 	return 0;
} /* dancerSerialGetByte */

			 	
void main(void)
{
	unsigned char dancerID, dancerPacket;
	unsigned char elwire, mode;

	sysDiagReq = 0;				// Reset Diagnostic Request	Flag

    uartInit38400();

	dancerBoardInit();

	dancerSysDiag();			// Perform initial diagnostic

	for (;;)
	{
		if (dancerSerialDataRdy != 0)
		{
			dancerPacket = dancerSerialDataRxd;
			dancerSerialDataRdy = 0;				// Reset Serial Data Rxd flag

			dancerID = dancerPacket & 0x07;

			if (dancerID == DANCER_ID)
			{
				elwire = dancerPacket >> 4;			// Which El-Wire
				mode   = (dancerPacket >> 3) & 1;	// El-Wire On/Off

				if (mode != 0)
				{
					dancerElWireOn(elwire);
				}
				else
				{
					dancerElWireOff(elwire);
				}
			}		
		}
	}		
} /* main */

void ISRSerialPort() interrupt 4 using 1
{
    if (TI)							// Transmittion finished
    {
		TI = 0;			  			// Reset transmit flag for next transmittion
	}
	else
	{
		RI = 0;						// Clear Serial Interrupt Flag

		dancerSerialDataRxd = SBUF;	// Read data from serial port
	   	dancerSerialDataRdy = 1;
	}

} /* ISRSerialPort */

void ISRsysDiagnostic() interrupt 0	using 2
{
	sysDiagReq=1;

} /* ISRsysDiagnostic */

