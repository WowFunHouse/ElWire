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

#define	LEDOn				0
#define	LEDOff				1

#define	WSN_RST				P37

#define ELWIRE_PORT			P1

#define	ELWIRE_BLU_SW		P10
#define	ELWIRE_GRN_SW		P11
#define	ELWIRE_AUX_SW		P12
#define	ELWIRE_GLS_SW		P13

#define	LED_AutoSeq			P25
#define	LED_Diagnostic		P26

#define	KEY_AutoSeq			P27

#define	ELWIRE_ON			0x80
#define	ELWIRE_OFF			0x00

#define	ELWIRE_BLU			0x00			// El-Wire Blue
#define	ELWIRE_GRN			0x01			// El-Wire Green
#define	ELWIRE_AUX			0x02			// El-Wire Aux Color
#define	ELWIRE_GLS			0x03			// El-Wire Eye-Glasses

void delayms(unsigned int t);
void uartInit38400(void);
void initBoard(void);
unsigned char keyAutoSeq(void);
void sysDiagLedRolling(void);
void sysDiagLedFlashing(void);
void elwireDiag(void);
void sysDiagnostic(void);
void elwireAutoSeq(void);
void elwireTxPacket(unsigned char pkt, unsigned char led);

int sysDiagReq=0;

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
	ES   = 0;		    // Disable Serial Port Interrupt
	EA   = 1;			// Enable All Interrupts

    TR1  = 1;			// Start Timer1 as baud rate generator

} /* uartInit38400 */

void initBoard(void)
{
	PORT_ELWIRES	 = 0;		// Turn off all EL-Wires

	WSN_RST			 = 0;		// Reset WSN-02/03 Wireless module
	delayms(10);				// Delay 50ms
	WSN_RST			 = 1;		// Normal Operation
	delayms(10);				// Wait for WSN-02/03 ready
	
} /* initBoard */

unsigned char keyAutoSeq(void)
{
	return ((~KEY_AutoSeq) & 1);

} /* keyAutoSeq */

void dancerElWireDiag(void)
{
	unsigned char pkt, dancer, elwire;
	unsigned int  t;

	t=100;											// Delay 100ms per El-Wire

	// Turn off all wire
	for (dancer=0; dancer<5; dancer++)
	{
		for (elwire=0; elwire<4; elwire++)
		{
			pkt = elwire<<4 | dancer & 0xf7;		// Turn off the El-Wire in reversed order
			
			elwireTxPacket(pkt, 1);
			delayms(t);
		}
	}

	for (dancer=0; dancer<5; dancer++)
	{
		for (elwire=0; elwire<4; elwire++)
		{
			pkt = elwire<<4 | dancer | 0x08;				// Turn on the El-Wire
		
			elwireTxPacket(pkt, 1);
			delayms(t);
		}
	}

	// Turn off all wire backward
	for (dancer=0; dancer<5; dancer++)
	{
		for (elwire=0; elwire<3; elwire++)
		{
			pkt = (3-elwire)<<4 | (4-dancer) & 0xf7;		// Turn off the El-Wire in reversed order
			
			elwireTxPacket(pkt, 1);
			delayms(t);
		}
	}
} /* elwireDiag */
		
void sysDiagnostic(void)
{
	LED_Diagnostic = LEDOn;

//	sysDiagLedFlashing();

	elwireDiag();

	LED_Diagnostic = LEDOff;

} /* sysDiagnostic */

void main(void)
{
	unsigned char arduinoMidi;

	sysDiagReq = 0;				// Reset Diagnostic Request	Flag

    uartInit38400();

	initBoard();

	for (;;)
	{
		if (sysDiagReq)
		{
			sysDiagnostic();		// Execute Diagnostic

			sysDiagReq = 0;			// Reset Diagnostic Request Flag			
			continue;
		}

		if (keyAutoSeq())
		{
			elwireAutoSeq();
			continue;
		}

		arduinoMidi = arduinoGetData();			// b7 of data should be 1 (ignored by dancer receiver)
			
		if (arduinoMidi)
		{
			elwireTxPacket(arduinoMidi, 1);		// Send Data with LED status on
		}
	}		
} /* main */

void ISRSerialPort() interrupt 4 using 1
{
    if (TI)					// Transmittion finished
    {
		TI = 0;			  	// Reset transmit flag for next transmittion
	}

} /* ISRSerialPort */

void ISRsysDiagnostic() interrupt 0	using 2
{
	sysDiagReq=1;

} /* ISRsysDiagnostic */

