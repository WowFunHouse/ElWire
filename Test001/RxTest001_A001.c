/****************************************************************************
 File:			RxTest001_A001.c

 LCD Demo

 Version:		0.01
 Description:	Serial Port Receive Module

 Created on:	2013-11-16
 Created by:	Michael

 Board:			RichMCU RZ-51V2.0

 Connection:	

 Jumpers:		
****************************************************************************/
#include "lib_uty001.h"
#include "lcd_lib001.h"

#define		DELAYLONG		30000

//#define	DEBUG

void main(void)
{
	for (;;)
	{
#ifdef	LCD_DEMO_ANIM_CHAR
		lcdDemoAnimatedChar();
#endif

#ifdef	LCD_DEMO_FLIP_ROW
		lcdDemoFlip();
#endif
	}
} /* main */


/**************************************************************
 Title:			Serial Communication Receive Module
 
 File:			RxTest001_A001.c

 Version:		0.01
 Description:	Serial Wireless Receive Module

 Created on:	2013-11-16
 Created by:	Michael Sin

 Board:			RichMCU RZ-51V2.0

 Connections:	Default

 Jumpers:		All on		
 **************************************************************/
#include <STC89.H>

#define	LED			P1

unsigned char	serialTxBusy;							/* 1=Busy; 0=ready */

void serialInit(void);
void serialPutChar(unsigned char c);
void actionSerialRx(unsigned char c);
unsigned char serialGetChar(void);

void delay(unsigned int d)
{
	unsigned int t;

	for (t=0; t<d; t++);

} /* delay */

void main(void)
{
	unsigned char	i;

	serialInit();

	i = 123;

	for(;;)
	{
		if (serialTxBusy != 1)					/* Transmittion not busy */
		{
			serialPutChar(++i);
			delay(500);
		    P0=i;
		}
	}
} /* main */


void serialInit(void)
{
	serialTxBusy = 0;							/* Unset Busy Flag */

	SM0 = 0;									/* Set Serial port to Mode 0 */
	SM1 = 0;
	SM2 = 0;

	ES = 1;										/* Enable Serial Interrupt */
	EA = 1;										/* Enable All Interrupts   */

	REN = 1;									/* Enable receiving */
								 
} /* serialInit */

void ISRserial(void) interrupt 4 using 2 
{
	unsigned char	c;

	ES = 0;
		
	if (RI == 1)								/* Data received */
	{
		c = serialGetChar();
		actionSerialRx(c);						/* Process data */
		RI = 0;									/* Clear RI */
	}

	if (TI == 1)								/* Data transmitted */
	{
	 	TI = 0;									/* Clear TI */
	 	serialTxBusy = 0;						/* Ready for next transmittion */
	}
	
	ES = 1;

} /* ISRserial */

void serialPutChar(unsigned char c)
{
	serialTxBusy = 1;							/* Transmittion is busy */
	SBUF = c;								    /* Send data out */
	
} /* serialPutChar */

unsigned char serialGetChar(void)
{
	unsigned char	c;

	c = SBUF;									/* read data */
	
	RI = 0;										/* clear RI */

	return c;

} /* serialGetChar */

void actionSerialRx(unsigned char c)
{
	LED = c;

} /* actionSerialRx */

#include <reg51.h>

void InitUART(void)
{
    TMOD = 0x20;
    SCON = 0x50;
    TH1 = 0xFD;
    TL1 = TH1;
    PCON = 0x00;
    EA = 1;
    ES = 1;
    TR1 = 1;
}

void SendOneByte(unsigned char c)
{
    SBUF = c;
    while(!TI);
    TI = 0;
}

void main(void)
{
    InitUART();
}

void UARTInterrupt(void) interrupt 4
{
    if(RI)
    {
        RI = 0;
        //add your code here!
    }
    else
        TI = 0;
}