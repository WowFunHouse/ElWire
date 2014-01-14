/**************************************************************
 Title:			El-Wire Dancer Wireless Receiver		
 
 File:			rx0030.c

 Version:		0.92 Beta
	
 			
 Description:	Receive packet wireless from transmitter
 				through WSN-02/03 433MHz wireless module

 Created on:	2014-01-12

 Created by:	Michael 
 
 Board:			Custom made MCS51 WSN-02/03 control system

 Connections:	P2.0 -> El-Wire Blue
 				P2.1 -> El-Wire Green
				P2.2 -> El-Wire Aux Color
				P2.3 -> El-Wire Eye-Glasses

				P2.4 -> LED: El-Wire Blue
				P2.5 -> LED: El-Wire Green
				P2.6 -> LED: El-Wire Aux Color
				P2.7 -> LED: El-Wire Eye-Glasses

				P3.0 -> Serial Port Rx -> WSN-02/03 Tx Pin5
				P3.1 -> Serial Port Tx -> WSN-02/03 Rx Pin4
				P3.7 -> WSN-02/03 NRST Reset Pin6 (Reset:Low)

 Jumpers:		N/A

 X'tal:			14.07456MHz (for baud rate: 38400bps)
 **************************************************************/
#include <STC89.H>

#define	DANCER_ID			0

#undef	DEBUG

#define	DEBUG_DELAYMS		10
#define	DEBUG_PKT			0x48

#define	LED_ON				0
#define	LED_OFF				1

#define	WSN_RST				P37
#define	LED_STATUS			P36

#ifdef	DEBUG										
#define ELWIRE_PORT			P1

#define	ELWIRE_BLU_SW		P10
#define	ELWIRE_GRN_SW		P11
#define	ELWIRE_AUX_SW		P12
#define	ELWIRE_GLS_SW		P13

#define	ELWIRE_BLU_LED		P14
#define	ELWIRE_GRN_LED		P15
#define	ELWIRE_AUX_LED		P16
#define	ELWIRE_GLS_LED		P17
#else
#define ELWIRE_PORT			P2

#define	ELWIRE_BLU_SW		P20
#define	ELWIRE_GRN_SW		P21
#define	ELWIRE_AUX_SW		P22
#define	ELWIRE_GLS_SW		P23

#define	ELWIRE_BLU_LED		P24
#define	ELWIRE_GRN_LED		P25
#define	ELWIRE_AUX_LED		P26
#define	ELWIRE_GLS_LED		P27

#endif

#define	ELWIRE_BLU			01				// P24
#define	ELWIRE_GRN			02			    // P25
#define	ELWIRE_AUX			04			    // P26
#define	ELWIRE_GLS			08			    // P27

#define	ELWIRE_ON			0;				// Turn on  El-Wire
#define	ELWIRE_OFF			1;				// Turn off El-Wire

#define	ELWIRE_ALL_ON		0x00;			// Turn on  all El-Wires and LEDs
#define	ELWIRE_ALL_OFF		0xff;			// Turn off all El-Wires and LEDs

#define	ELWIRE_PKT_BLU		0x00			// El-Wire Blue
#define	ELWIRE_PKT_GRN		0x01			// El-Wire Green
#define	ELWIRE_PKT_AUX		0x02			// El-Wire Aux Color
#define	ELWIRE_PKT_GLS		0x03			// El-Wire Eye-Glasses

unsigned char sysDiagReq;					// INT0 (Not in used)
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

void dancerSysLEDFlash(void)
{
	LED_STATUS = ~LED_STATUS;
	delayms(2);
	LED_STATUS = ~LED_STATUS;

} /* dancerSysLEDFlash */

void uartInit38400(void)
{
	// X'tal: 14.7456MHz
	// 38400 Baud, 8 Bit Data, No Parity, 1 Stop Bit

    TMOD = T1_M2;		// Set Timer 2 to Mode 2 (8-bit counter auto reload)
    SCON = 0x50;		// 8 bit data, Enable Rx
    TH1  = 0xFF;
    TL1  = TH1;
    PCON = 0x00;		// No double baud rate

	IT0	 = 1;			// INT0: Falling edge trigger
	EX0	 = 1;			// Enable External Interrupt INT0
	ES   = 0;		    // Disable Serial Port Interrupt
	EA   = 1;			// Enable All Interrupts

    TR1  = 1;			// Start Timer1 as baud rate generator

} /* uartInit38400 */

void dancerSysBoardInit(void)
{
	ELWIRE_PORT		 = ELWIRE_ALL_OFF;		// Turn off all EL-Wires
	
	WSN_RST			 = 0;					// Reset WSN-02/03 Wireless module
	delayms(10);							// Delay 10ms
	WSN_RST			 = 1;					// Normal Operation
	delayms(10);							// Wait for WSN-02/03 ready

	LED_STATUS		 = LED_ON;				// Board Ready
		
} /* dancerBoardInit */

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
			ELWIRE_GLS_LED = LED_ON;
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
			ELWIRE_GLS_LED = LED_OFF;
			break;

		default:
			break;
		}
} /* dancerElwireOff */

void dancerElWireControl(unsigned char pattern)
{
	unsigned char elwire, led;

	elwire  = ~(pattern & 0x0f);		// Lower 4 bits only for controlling El-Wires

	led 	= elwire << 4;				// On-board status LEDs

/*
	elwire |= led;						// BUG: Not working - may be compiler bug
*/
	elwire  = led | (led >> 4);

	ELWIRE_PORT = elwire;				// Turn On/Off the El-Wire and Status LEDs

} /* dancerElWireControl */

void dancerElWireDiag(void)
{
	unsigned char elwire;
	unsigned char i, n;
	unsigned int  t;

	i=ELWIRE_ON;
	t=200;									// Delay 100ms per El-Wire

	// Turn off all wires
	ELWIRE_PORT = ELWIRE_ALL_OFF;			// Turn off the El-Wires

	for (n=0; n<3; n++)
	{	  
		for (elwire=0; elwire<4; elwire++)
		{
			dancerElWireControl(i << elwire);	// Turn on each El-Wire
			delayms(t);
		}

		for (i=0; i<2; i++)
		{
			ELWIRE_PORT = ELWIRE_ALL_OFF;
			delayms(t);
	
			ELWIRE_PORT = ELWIRE_ALL_ON;
			delayms(t);
		}
	}
	delayms(2*t);

	ELWIRE_PORT = ELWIRE_ALL_OFF;

} /* elwireDiag */
		
void dancerSysDiag(void)
{
	dancerElWireDiag();

} /* dancerSysDiag */

unsigned char dancerSerialGetByte(void)
{
	unsigned char packet;

	packet = 0;

	if (RI != 0)
	{
		packet = SBUF;
		packet |= 0x80;
		RI = 0;

		dancerSysLEDFlash();
	}

	return packet;

} /* dancerSerialGetByte */

			 	
void main(void)
{
	unsigned char dancerID, dancerPacket;
	unsigned char elwire, mode;

	dancerSysBoardInit();

    uartInit38400();

	dancerSysDiag();								// Perform initial diagnostic

	for (;;)
	{
		dancerPacket = dancerSerialGetByte();		// Check if data avilable?
		if (dancerPacket != 0)
		{
			dancerPacket &= 0x7f;					// Filter out the formating from Rx

			dancerID = dancerPacket & 0x07;	 		// Get the dancer ID

			if (dancerID == DANCER_ID)				// Is it a packet belong to me?
			{
				elwire =  dancerPacket >> 4;		// Which El-Wire
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

} /* ISRSerialPort */

void ISRsysDiagnostic() interrupt 0	using 2
{
	sysDiagReq=1;

} /* ISRsysDiagnostic */

