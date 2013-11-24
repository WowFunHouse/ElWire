/*********************
*****************************************
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

//#define	DEBUG

#define	LED			P1

void serialInit(void);
void serialTxChar(unsigned char c);

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
		serialTxChar(++i);
		delay(500);

#ifdef DEBUG
	    P0=i;
#endif

	}
} /* main */

void serialInit(void)
{
    TMOD = 0x20;
    SCON = 0x50;

    TH1 = 0xFD;
    TL1 = TH1;

    PCON = 0x00;

	ES = 1;										/* Enable Serial Interrupt */
	EA = 1;										/* Enable All Interrupts   */

    TR1 = 1;									/* Start timer1 for serial port */
								 
} /* serialInit */

void serialTxChar(unsigned char c)
{
    SBUF = c;									/* Load data to be transmitted */

    while(!TI);									/* Wait for data being transmitted */
    TI = 0;										/* Data transmitted */

} /* serialTxChar */

void actionSerialDataRxd(unsigned char c)
{
	LED = c;									/* Show received data */

} /* actionSerialDataRxd */

void ISRserial(void) interrupt 4 using 2 
{
	ES = 0;										/* Disable serial port interrupt */

    if (RI != 0)								/* Data received */
    {
        RI = 0;									/* Clear interrupt flag */
		actionSerialDataRxd(SBUF);					/* Process received data */
    }
    else
	{
        TI = 0;									/* Clear interrupt flag */
	}

	ES = 1;										/* Enable serial port interrupt */

} /* ISRserial */
