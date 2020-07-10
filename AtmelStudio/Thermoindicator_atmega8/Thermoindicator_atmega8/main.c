#include "thermo_indicator.h"

const char d_digits[] = {
	0b00000011,
	0b10011111,
	0b00100101,
	0b00001101,
	0b10011001,
	0b01001001,
	0b01000001,
	0b00011111,
	0b00000001,
	0b00001001,
	0b00010001,
	0b11000001,
	0b01100011,
	0b10000101,
	0b01100001,
	0b01110001
};

const char d_minus = 0b11111101;
const char d_empty = 0b11111111;
const char d_dot = 0b11111110;
char *disp;

ISR(TIMER1_OVF_vect)
{
	static char interupt_n;
	
	interupt_n++;
	check_temp(interupt_n);
}

int main(void)
{
	mc_init();
	while (1)
	{	
		display_update();
	}
}

void mc_init()
{
	disp = (char*)malloc(sizeof(char) * 4);
	for (int i = 0; i < 4; i++)
	disp[i] = d_empty;
	
	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0x01;
	
	TCCR1B |= (1<<1); TCCR1B &= ~(1<<2 | 1<<0); // interruptions occur each 0,5s
	TIMSK |= (1<<2);
	TCNT1 = 0;
	sei();
}

void display_update()
{
	char d_num = 1;
	
	for (int i = 0; i < 4; i++)
	{
		DIGIT_PORT = d_num;
		SEG_PORT = disp[i];
		d_num = d_num << 1;
		_delay_ms(3);
	}
}

