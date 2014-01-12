#include <stc89.h>

void delay(void)
{
	long unsigned int t;

	for (t=0; t<10000; t++);
}
void main(void)
{
	for (;;)
	{
		P1=~P1;
		delay();
	}
} /* main */