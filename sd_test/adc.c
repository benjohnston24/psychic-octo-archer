/*
 * adc.c
 *
 * Created: 2/12/2014 4:03:44 PM
 *  Author: benj1
 */ 

#include <avr/io.h>
#include "adc.h"

void adc_init(void){
	ADCSRA |= (0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  //16Mhz/8 = 2Mhz the ADC reference clock
	ADMUX |= (1<<REFS0);                			//Voltage reference from Avcc (5v)
	ADCSRA |= (1<<ADEN);                			//Turn on ADC
	ADCSRA |= (1<<ADSC);                			//Do an initial conversion because this one is the slowest and to ensure that everything is up and running
	//Wait for the conversion to be complete
	while(ADCSRA & (1<<ADSC));
	//Enable interrupts
	ADCSRA |= (1 << ADIF) | (1 << ADIE);
	ADCSRA |= (1<<ADSC);
}

uint16_t read_adc(uint8_t channel){
	ADMUX &= 0xF0;                    	//Clear the older channel that was read
	ADMUX |= channel;                	//Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);               //Starts a new conversion
	while(ADCSRA & (1<<ADSC));         //Wait until the conversion is done
	return ADCW;                   	   //Returns the ADC value of the chosen channel
}

void adc_stop(void)
{
	ADCSRA |= (0<<ADEN);
}

void adc_start(void)
{
	ADCSRA |= (1<<ADEN);	
}