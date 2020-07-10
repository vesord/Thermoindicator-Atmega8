#include "thermo_indicator.h"

extern const char d_digits[];
extern const char d_minus;
extern const char d_empty;
extern const char d_dot;
extern char *disp;

void check_temp(char interrupt_n)
{
	static char was_convertion;
	static char offset;
	
	if (!was_convertion)
	{
		if (convertion_init())
		{
			was_convertion = 1;
			offset = interrupt_n % 2;
		}
	}
	else if (!((interrupt_n - offset) % 2))
	{
		convertion_get_res();
		was_convertion = 0;
	}
}

char convertion_init()
{
	if(!onewire_reset())
	{
		return (0);
	}
	else
	{
		onewire_send(0xCC);
		_delay_us(10);
		onewire_send(0x44);
		return (1);
	}
}

void convertion_get_res()
{
	if(!onewire_reset())
	{
		return ;
	}
	else
	{
		onewire_send(0xCC);
		_delay_us(5);
		onewire_send(0xBE);
		_delay_us(5);
		char scratchpad[9];
		char b = 0;
		for (int i = 0; i < 9; i++)
		{
			b = onewire_read();
			scratchpad[i] = b;
		}
		int16_t t = (scratchpad[1] << 8) | scratchpad[0];
		set_display(t);
	}
}

void set_display(int16_t t)
{
	char sign = 0;
	int digit = 0;
	char frac = 0;
	
	if (t < 0)
	{
		sign = 1;
		t = -t;
	}
	frac = t & 0xF;
	t /= 16;
	disp[digit++] = (t / 100) ? d_digits[t / 100] : d_empty;
	disp[digit++] = (t / 10 % 10) || (t / 100) ? d_digits[t / 10 % 10] : d_empty;
	disp[digit++] = d_digits[t % 10] & d_dot;
	disp[digit] = d_digits[frac * 10 / 16];
	if (sign)
	{
		if (disp[1] == d_empty)
		disp[1] = d_minus;
		else
		disp[0] = d_minus;
	}
}