/*
 * timing.c
 *
 * Created: 1/12/2014 3:20:36 PM
 *  Author: benj1
 */ 

#include <avr/io.h>
#include "timing.h"


void timer0_compa_init(void)
{
	//No output pins used
	TCCR0A = 0;
	//Set the clock speed for F_CPU / 1024
	TCCR0B |= (1 << CS02) | (0 << CS01) | (0 << CS00);
	//Configure the Output compare register for compare A
	OCR0A = 0x00;
	//Configure the compare interrupt
	//TIMSK0 |= (1 << OCIE0A);
	
}

void timer0_free_run_init(void)

{
	TCCR0A = 0;
	TCCR0B |= (0 << CS02) | (0 << CS01) | (1 << CS00);
}

void timer1_overflow_init(void)
{
	//No output pins used
	TCCR1A = 0;
	timer1_start();
	//Configure the overflow interrupt
	TIMSK1 |= (1 << TOIE1);
	
}

void timer1_stop(void)
{
	TCCR1B = 0;
}

void timer1_start(void)
{
	//Set the clock speed for F_CPU / 1024
    TCCR1B |= (1 << CS02) | (0 << CS01) | (1 << CS00);
	//TCCR1B |= (0 << CS02) | (0 << CS01) | (1 << CS00);		
}