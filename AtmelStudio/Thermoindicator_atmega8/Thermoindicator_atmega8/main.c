#define F_CPU 1000000UL
#define SEG_PORT PORTB
#define DIGIT_PORT PORTC

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

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

void mc_init();
void display_update(char *disp);
void check_temp(char interrupt_n);

ISR(TIMER1_OVF_vect)
{
	static char interupt_n;
	
	interupt_n++;
	check_temp(interupt_n);
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

void display_update(char *disp)
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

int a = 0;

#define ONEWIRE_PIN 0b00000001

void onewire_low()
{
	PORTD &= ~(ONEWIRE_PIN);
}

void onewire_high()
{
	PORTD |= (ONEWIRE_PIN);
}

void onewire_set_output()
{
	PORTD |= (ONEWIRE_PIN);
	DDRD |= (ONEWIRE_PIN);
}

void onewire_set_input()
{
	PORTD &= ~(ONEWIRE_PIN);
	DDRD &= ~(ONEWIRE_PIN);
}

int onewire_level()
{
	return (PIND & ONEWIRE_PIN);
}

int onewire_reset()
{
	onewire_set_output();
	onewire_low();
	_delay_us(640);
	onewire_high();
	_delay_us(2);
	onewire_set_input();
	for (char c = 80;  c > 0; c--) 
	{
		if (!onewire_level()) 
		{
			while (!(onewire_level())) 
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
	onewire_low();
	if (bit) 
	{
		_delay_us(5); // Низкий импульс, от 1 до 15 мкс (с учётом времени восстановления уровня)
		onewire_high();
		_delay_us(90); // Ожидание до завершения таймслота (не менее 60 мкс)
	} else 
	{
		_delay_us(90); // Низкий уровень на весь таймслот (не менее 60 мкс, не более 120 мкс)
		onewire_high();
		_delay_us(5); // Время восстановления высокого уровеня на шине + 1 мс (минимум)
	}
}

// Отправляет один байт, восемь подряд бит, младший бит вперёд
// b - отправляемое значение
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
	_delay_us(2); // Длительность низкого уровня, минимум 1 мкс
	onewire_set_input();
	_delay_us(8); // Пауза до момента сэмплирования, всего не более 15 мкс	
	char r = onewire_level();
	_delay_us(90); // Ожидание до следующего тайм-слота, минимум 60 мкс с начала низкого уровня
	return r;
}

// Читает один байт, переданный устройством, младший бит вперёд, возвращает прочитанное значение
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

int main(void)
{
	mc_init();
	while (1)
	{	
		display_update(disp);
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