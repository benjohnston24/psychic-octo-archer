/*
 * main.c
 *
 * Created: 26/11/2014 4:01:32 PM
 *  Author: benj1
 */ 

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "spi.h"
#include "sd.h"
#include "adc.h"
#include "timing.h"

uint16_t step = 0;
uint8_t counter = 0, duration = 0, channel = 1;
uint16_t data[BLOCK_SIZE];


//Interrupt service routine for analogue to digital converter
ISR(ADC_vect)
{
	uint16_t data = ADCW;
	printf("%d: %d\n", channel, ADCW);
	ADCSRA |= (1<<ADSC);
	channel++;
	if (channel >= 4)
	{
		channel = 1;
	}
}

//Timer overflow counter
ISR(TIMER1_OVF_vect)
{
	//TCCR0B = 0x00;
	counter++;
	if (counter >= 2)
	{   
		step = 7;
	}
}

void main(void)
{	
	while (1){
	switch (step){
		
		case 0: 
			/* Init the UART */
			usart_init();
			timer1_overflow_init();
			adc_init();
			stdout = &usart_stdout;
			printf("\n\n\n\n\n-------------------------------------------\n");
			printf("Systems Configured\n");
			step = 1;
			break;
			
	    case 2: 	// Configure the SPI
			spi_init();
			printf("SPI Configured\n");
			if(sd_init() == 1){
				printf("SD Initialised\n");
				step = 2;}
			else{ step = 999;}
			break;
			
		case 1: //Wait for receipt of duration of test
			printf("Requesting test duration:\n");		    
			duration = usart_read();
			printf("Received %d\n", duration);
			//Enable interrupts
			sei();
			step = 5;
			break;
			
		case 3:	//Write data
			memset(data, 0x32, BLOCK_SIZE);
			if(write_sector(0x12, 0x34, data) == 1){step = 4;}
			else{ step = 999;}
			printf("Data written\n");
			break;
			
		case 4:	//Read data back;
			memset(data, 0x00, BLOCK_SIZE);
			if(read_sector(0x12, 0x34, data_back) == 1){
				for(uint16_t i = 0; i < BLOCK_SIZE; i++)
				{
					printf("0x%x, ", data_back[i]);
				}
				step = 101;
			}
			else{ step = 999;}
			break;
			
		case 5: //Test reading time
			step = 6;
			break;
			
		case 6:
			step=5;
			break;
			
		case 7:
		     cli();
			 counter = 0;
			 step = 1;
			 break;
				
		case 101: printf("Done\n"); step = 1000; HIGH_CS(); break;
		case 999: printf("Error\n"); step = 1000; HIGH_CS(); break;
		
		case 1000: while(1);
		
			}
	}
	
	return 0;
}
