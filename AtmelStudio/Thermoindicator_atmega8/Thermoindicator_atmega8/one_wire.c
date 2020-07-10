#include "thermo_indicator.h"

void onewire_low()
{
	PORTD &= ~(ONEWIRE_PIN);
}

void onewire_high()
{
	PORTD |= (ONEWIRE_PIN);
}

/*
**	Sets ONEWIRE_PIN to output. Default value is HIGH.
*/
void onewire_set_output()
{
	PORTD |= (ONEWIRE_PIN);
	DDRD |= (ONEWIRE_PIN);
}

/*
**	Sets ONEWIRE_PIN to input. No pull-up.
*/
void onewire_set_input()
{
	PORTD &= ~(ONEWIRE_PIN);
	DDRD &= ~(ONEWIRE_PIN);
}

int onewire_level()
{
	return (PIND & ONEWIRE_PIN);
}


/*
**	Sends on wire signal for starting work
*/
int onewire_reset()
{
	onewire_set_output();
	onewire_low();
	_delay_us(640);		// Reset signal
	onewire_high();
	_delay_us(2);
	onewire_set_input();
	for (char c = 80;  c > 0; c--)
	{
		if (!onewire_level())			// If response got ...
		{
			while (!(onewire_level()))	// Wait until response ends
			{
				_delay_us(1);
			}
			return 1;
		}
		_delay_us(1);
	}
	return 0;
}

void onewire_send_bit(char bit)
{
	onewire_set_output();
	onewire_low();			// Time slot initiation
	if (bit)
	{
		_delay_us(5);
		onewire_high();		// Send 1
		_delay_us(90);
	} else
	{
		_delay_us(90);		// Send 0
		onewire_high();
		_delay_us(5);
	}
}

/*
**	Sends 1 byte. Least bit first.
*/
void onewire_send(char b)
{
	for (char p = 8; p; p--)
	{
		onewire_send_bit(b & 1);
		b >>= 1;
	}
}

char onewire_read_bit()
{
	onewire_set_output();
	onewire_low();
	_delay_us(2);		// Time slot initiation
	onewire_set_input();
	_delay_us(8);		// Sampling
	char r = onewire_level();
	_delay_us(90);		// Wait until time slot ends
	return r;
}

/*
**	Reads 1 byte. Least bit first. Returns read byte.
*/
char onewire_read()
{
	char r = 0;
	for (char p = 8; p; p--)
	{
		r >>= 1;
		if (onewire_read_bit())
		r |= 0x80;
	}
	return r;
}
