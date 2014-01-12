/**************************************************************
 Title:			Parallel to Wireless Transmitter		
 
 File:			tx0011.c

 Version:		0.50
 Description:	Receive parallel pkt from Arduino
 				and transmit through WSN-02/03 433MHz transmitter

 Created on:	2014-01-11
 Created by:	Michael
  
 Board:			Custom made MCS51 WSN-02/03 control system

 Connections:	P1.0 <- Arduino Pin 2 (Dancer ID Bit0)
 				P1.1 <- Arduino Pin 3 (Dancer ID Bit1)
				P1.2 <- Arduino Pin 4 (Dancer ID Bit2)
				P1.3 <- Arduino Pin 7 (ElWire Mode 1:On 0:Off)

				P1.4 <- Arduino Pin 5 (ElWire Index Bit 0)
				P1.5 <- Arduino Pin 6 (ElWire Index Bit 1)

				P1.6 <- Arduino Pin 8 (Data Ready  0:Ready)
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

				P3.0 -> Serial Port Rx -> WSN-02/03 Tx Pin5
				P3.1 -> Serial Port Tx -> WSN-02/03 Rx Pin4
				P3.7 -> WSN-02/03 NRST Reset Pin6 (Reset:Low)

 Jumpers:		N/A

 X'tal:			14.07456MHz (for baud rate: 38400bps)
 **************************************************************/
#include <STC89.H>

#undef	DEBUG
#define	DEBUG_ARDUINO_OFF
#undef	DEBUG_LED_PTN
#define	DEBUG_SLOW

#define	DEBUG_DELAYMS		10				// Delay 10ms
#define	DEBUG_PKT			0x48

#define	LEDOn				0
#define	LEDOff				1

#define	WSN_RST				P37

#define PORT_Arduino		P1

#define DATA_Rdy			P16
#define	DATA_Ack			P17

#define	PORT_Ctrl_Status	P2

#define	LED_Dancer1			P20
#define LED_Dancer2			P21
#define LED_Dancer3	 		P22
#define LED_Dancer4	  		P23
#define LED_Dancer5			P24

#define	LED_AutoSeq		P25
#define	LED_Diagnostic		P26

#define	KEY_AutoSeq		P27

#define	ELWIRE_ON			0x80
#define	ELWIRE_OFF			0x00

#define	ELWIRE_BLU			0x00			// El-Wire Blue
#define	ELWIRE_GRN			0x01			// El-Wire Green
#define	ELWIRE_AUX			0x02			// El-Wire Aux Color
#define	ELWIRE_GLS			0x03			// El-Wire Eye-Glasses

void delayms(unsigned int t);
void uartInit38400(void);
void serialTxChar(unsigned char c);
void initBoard(void);
unsigned char keyAutoSeq(void);
void ledDancerOn(unsigned arduinoData);
void ledDancerOff(unsigned arduinoData);
void ledDancerAllOff(void);
void sysDiagLedRolling(void);
void sysDiagLedFlashing(void);
void elwireDiag(void);
void sysDiagnostic(void);
void elwireAutoSeq(void);
unsigned char arduinoGetData(void);
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

	WSN_RST			 = 0;		// Reset WSN-02/03 Wireless module
	delayms(10);				// Delay 50ms
	WSN_RST			 = 1;		// Normal Operation
	delayms(10);				// Wait for WSN-02/03 ready
	
} /* initBoard */

unsigned char keyAutoSeq(void)
{
	return ((~KEY_AutoSeq) & 1);

} /* keyAutoSeq */

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

void ledDancerOff(unsigned arduinoData)
{
	arduinoData &= 0x7;		// Filter out the Dancer#

	switch (arduinoData)
	{
		case 0:
			LED_Dancer1 = LEDOff;
			break;
		
		case 1:
			LED_Dancer2 = LEDOff;
			break;

		case 2:
			LED_Dancer3 = LEDOff;
			break;

		case 3:
			LED_Dancer4 = LEDOff;
			break;

		case 4:
			LED_Dancer5 = LEDOff;
			break;

	  	case 7:
			LED_Dancer1 = LEDOff;
			LED_Dancer2 = LEDOff;
			LED_Dancer3 = LEDOff;
			LED_Dancer4 = LEDOff;
			LED_Dancer5 = LEDOff;
			break;

		default:
			break;
	}
} /* ledDancerOn */

#ifdef DEBUG_LED_PTN
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

	t=100;			// Delay 100ms
	
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

	t=50;			// Delay 50ms
	
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
#endif /* DEBUG_LED_PTN */

void elwireDiag(void)
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
			pkt = elwire<<4 | dancer | 0x08;		// Turn on the El-Wire
		
			elwireTxPacket(pkt, 1);
			delayms(t);
		}
	}

	// Turn off all wire backward
	for (dancer=0; dancer<5; dancer++)
	{
		for (elwire=0; elwire<4; elwire++)
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

void elwireAutoSeq(void)
{
	unsigned char pkt, dancer, elwire;
	unsigned char n;
	unsigned int  t;

	t=50;												// Delay 50ms per El-Wire

	LED_AutoSeq = LEDOn;
	
//	sysDiagLedRolling();

	// Turn off all wire
	for (dancer=0; dancer<5; dancer++)
	{
		for (elwire=0; elwire<4; elwire++)
		{
			pkt = (3-elwire)<<4 | (4-dancer) & 0xf7;		// Turn off the El-Wire in reversed order
			
			elwireTxPacket(pkt, 1);
		}
		delayms(t);
	}

	// Turn on the El-Wire colors one by one except the Eye-Glasses
	for (elwire=0; elwire<3; elwire++)
	{
		for (dancer=0; dancer<5; dancer++)
		{
			pkt = elwire<<4 | dancer | ELWIRE_ON;				// Turn on the El-Wire
		
			elwireTxPacket(pkt, 1);
		}
		delayms(2*t);
	}

	elwire = ELWIRE_GLS;										// Toggle the EL-Wire Eye-Glasses

	for (n=0; n<2; n++)
	{
		for (dancer=0; dancer<5; dancer++)
		{
			pkt = elwire<<4 | dancer | ELWIRE_ON;  				// Turn on the El-Wire
		}
		delayms(t);

		for (dancer=0; dancer<5; dancer++)
		{
			pkt = elwire<<4 | dancer | ELWIRE_OFF;  			// Turn off the El-Wire
		}
		delayms(t);
	}

	for (dancer=0; dancer<5; dancer++)
	{
		pkt = elwire<<4 | dancer | ELWIRE_ON;  					// Turn on the El-Wire
	}
		
	LED_AutoSeq = LEDOff;

} /* elwireAutoSeq */

unsigned char arduinoGetData(void)
{
	unsigned char arduinoData;

#ifdef DEBUG_ARDUINO_OFF
	arduinoData = DEBUG_PKT;
#else
	if (DATA_Rdy != 0)		// No data available
	{					
		return 0;
	}

	arduinoData = PORT_Arduino & 0x3f | 0x80;	// Read Data from Arduino

	DATA_Ack = 0; 			// Send Ack to Arduino
	
	while (DATA_Rdy == 0); 	// Wait for Arduino ack my ACK and reset RDY
	DATA_Ack = 1;		  	// Reset ACK to standby
#endif

	return arduinoData;		// Keep bit-6 On to indicate data available from Arduino

} /* arduinoGetData */

void elwireTxPacket(unsigned char pkt, unsigned char led)
{
	if (led) ledDancerOn(pkt);

#ifdef	DEBUG_SLOW
	delayms(DEBUG_DELAYMS);
#endif

	serialTxChar(pkt); 			// Send data to dancers
	serialTxChar(pkt);			// Resend for insurance

	if (led) ledDancerOff(pkt);

#ifdef	DEBUG_SLOW
	delayms(DEBUG_DELAYMS);
#endif
} /* elwireTxPacket */

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

