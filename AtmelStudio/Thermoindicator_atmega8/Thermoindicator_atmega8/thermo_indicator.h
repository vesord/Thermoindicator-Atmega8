#ifndef THERMO_INDICATOR_H_
#define THERMO_INDICATOR_H_

# define F_CPU 1000000UL
# define SEG_PORT PORTB
# define DIGIT_PORT PORTC

# include <avr/io.h>
# include <util/delay.h>
# include <avr/interrupt.h>
# include <stdlib.h>

# define ONEWIRE_PIN 0b00000001 // Pin on PORTD which will be connected to DS1822

/*
**	main.c
*/
void	mc_init();
void	display_update();

/*
**	one_wire.c
*/
void	onewire_set_output();
void	onewire_set_input();
void	onewire_low();
void	onewire_high();
int		onewire_level();
int		onewire_reset();
void	onewire_send_bit(char bit);
void	onewire_send(char b);
char	onewire_read_bit();
char	onewire_read();

/*
**	convertion.c
*/
void	check_temp(char interrupt_n);
char	convertion_init();
void	convertion_get_res();
void	set_display(int16_t t);

#endif /* THERMO_INDICATOR_H_ */