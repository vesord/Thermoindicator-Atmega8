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
char *disp;

void mc_init();
void display_update(char *disp);
void check_temp();

ISR(TIMER0_OVF_vect)
{
	check_temp();
}


 
void mc_init()
{
	disp = (char*)malloc(sizeof(char) * 4);
	for (int i = 0; i < 4; i++)
		disp[i] = d_empty;
		
	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0x01;
/*	
	TCCR0 |= (1<<2); TCCR0 &= ~(1<<1 | 1<<0);
	TIMSK |= (1<<0);
	TCNT0 = 0;
	sei();
	
	ADMUX |= (1<<REFS1) | (1<<REFS0); // set IVR 2,56 V
	ADMUX &= ~(1<<ADLAR); // right adjust result 
	ADMUX &= ~0b1111;
	ADMUX |= 0b0100; // use ADC4 pin
	ADCSRA |= (1<<ADEN); // Enable ADC
	ADCSRA &= ~(1<<ADPS2);
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1); // discretization frequency 125 kHz
*/
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
	PORTD &= ~(ONEWIRE_PIN); // (re)set pulling up
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
	_delay_us(640); // Пауза 480..960 мкс
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

void onewire_send_bit(char bit) {
	onewire_set_output();
	onewire_low();
	if (bit) {
		_delay_us(5); // Низкий импульс, от 1 до 15 мкс (с учётом времени восстановления уровня)
		onewire_high();
		_delay_us(90); // Ожидание до завершения таймслота (не менее 60 мкс)
		} else {
		_delay_us(90); // Низкий уровень на весь таймслот (не менее 60 мкс, не более 120 мкс)
		onewire_high();
		_delay_us(5); // Время восстановления высокого уровеня на шине + 1 мс (минимум)
	}
}

// Отправляет один байт, восемь подряд бит, младший бит вперёд
// b - отправляемое значение
void onewire_send(char b) {
	for (char p = 8; p; p--) {
		onewire_send_bit(b & 1);
		b >>= 1;
	}
}

char onewire_read_bit() {
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
char onewire_read() {
	char r = 0;
	for (char p = 8; p; p--) {
		r >>= 1;
		if (onewire_read_bit())
		r |= 0x80;
	}
	return r;
}

int main(void)
{
	mc_init();
	
	if(!onewire_reset())
	{
		disp[0] = d_minus;
	}
	else
	{
		onewire_send(0xCC);
		_delay_us(10);
		onewire_send(0x44);
		//disp[0] = d_digits[8]; // Response succeed
		_delay_ms(900);
	}
	if(!onewire_reset())
	{
		disp[0] = d_minus;
	}
	else
	{
		onewire_send(0xCC);
		_delay_us(5);
		onewire_send(0xBE);
		_delay_us(5);
		char scratchpad[9];
		char b = 0;
		for (int i = 0; i < 9; i++) {
			b = onewire_read();
			scratchpad[i] = b;
		}
		int t = (scratchpad[1] << 8) | scratchpad[0];
		disp[0] = d_digits[(t >> 12) & 0b1111];
		disp[1] = d_digits[(t >> 8) & 0b1111];
		disp[2] = d_digits[(t >> 4) & 0b1111];
		disp[3] = d_digits[(t >> 0) & 0b1111];
	}
	
	
	while (1)
	{	
		display_update(disp);
	}
}


void check_temp()
{

}