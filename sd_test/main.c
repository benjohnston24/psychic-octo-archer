/*
 * main.c
 *
 * Created: 26/11/2014 4:01:32 PM
 *  Author: benj1
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"
#include "spi.h"
#include "sd.h"
#include "adc.h"
#include "timing.h"

uint16_t step = 0, data_counter = 0, block_counter = 1;
uint8_t counter = 0, duration = 0, channel = 1, timer_running = 0, trigger_log = 0, fake_data = 0;
uint8_t data[BLOCK_SIZE];

//#define DEBUG_MAIN 1


//Interrupt service routine for analogue to digital converter
ISR(ADC_vect)
{
	//Collect data from ADC
	//uint16_t adc_data = ADCW;
	//printf("%d: %d\n", channel, ADCW);		
	//printf("%d\n", fake_data);
	data[data_counter++] = channel;	
	
	/**********************TEMP**********************************/
	data[data_counter++] = (uint16_t) fake_data++;
	
	//If more than a block of data has been collected, trigger logging
	if (data_counter > (BLOCK_SIZE - 1))	
	{
		data_counter = 0;
		block_counter++;
        trigger_log = 1;		
	}
	
	// Prepare for the next ADC read
	ADMUX &= 0xF0;                     //Clear the older channel that was read
	ADMUX |= channel++;                	//Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);               //Starts a new conversion	
	
	//Only read channels 1 to 3
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
	if (counter >= 0)
	{   
		timer_running = 0;
	}
}

int main(void)
{	
	while (1){
	switch (step){
		
		case 0: 
			/* Init the UART */
			usart_init();
			timer1_overflow_init();
			adc_init();
			stdout = &usart_stdout;
			#ifdef DEBUG_MAIN
				printf("\n\n\n\n\n-------------------------------------------\n");
				printf("Systems Configured\n");
			#endif
			//data = calloc(BLOCK_SIZE, sizeof(uint8_t));
			//data_to_write = calloc(BLOCK_SIZE, sizeof(uint8_t));
			//data_start = data;
			/*if((data != NULL) | (data_to_write != NULL))
			{
				step = 1;
			}
			else
			{
				printf("Error allocating memory for test data\n");
				step = 999;
			}*/
			step = 1;
			break;
			
	    case 1: 	
		    // Configure the SPI
			spi_init();
			#ifdef DEBUG_MAIN 
				printf("SPI Configured\n"); 
			#endif
			if(sd_init() == 1)
			{
				#ifdef DEBUG_MAIN 
					printf("SD Initialised\n");
				#endif
				step = 2;
			}
			else{ step = 999;}
			break;
			
		case 2: //Wait for receipt of duration of test
			#ifdef DEBUG_MAIN 
				printf("Requesting test duration:\n"); 
			#endif
			//duration = usart_read();
			#ifdef DEBUG_MAIN 
				printf("Received %d\n", duration); 
			#endif
			//Enable interrupts
			sei();
			timer_running = 1;
			step = 5;
			break;
				
		case 5: 
			//If the timer is no longer running exit the state
			if (timer_running == 0)
			{
				step = 7;
			}
			//If a log request has been trigger, log the data
			if (trigger_log == 1)
			{			
				uint16_t memory_block = block_counter * BLOCK_SIZE;
				if (write_sector(memory_block >> 8,memory_block & 0x00FF, data) != 1)	
				{
					//An error has occurred during writing
					//Stop the timer and branch to the complete state
					cli();
					timer1_stop();
					adc_stop();					
					printf("SD Card write error\n");
					step = 7;
				}				
			}
			break;
			
		case 7:
			 //Data collection complete
			 //Stop timers
		     cli();
			 timer1_stop();
			 adc_stop();
			 #ifdef DEBUG_MAIN
				 printf("Done\n");
				 printf("Block Counter: %d\n", block_counter);
				 printf("Reading Data\n");
			 #endif
			 //Send the block count
			 printf("$b, %d\n\r", block_counter);
			 for (uint16_t i = 0; i <= block_counter; i++)
			 {
				
				memset(data, 0x00, BLOCK_SIZE);
				uint16_t memory_block = i * BLOCK_SIZE;
				if(read_sector(memory_block >> 8,memory_block & 0x00FF, data) == 1)
				{
					#ifdef DEBUG_MAIN
						printf("Read Block: %d\n", i);
					#endif
					for(uint16_t j = 0; j < BLOCK_SIZE; j++)
					{
						printf("%d, ", data[j]);
					}
				}
				else
				{
					printf("Error reading back data\n");
				}
			}				 		 		 
			 counter = 0;
			 step = 101;
			 #ifdef DEBUG_MAIN
				printf("Blocks read\n");
			 #endif
			 //Need to free data
			 break;
				
		case 101: step = 1000; HIGH_CS(); break;
		case 999: printf("Error\n"); step = 1000; HIGH_CS(); break;
		
		case 1000: while(1);
		
			}
	}
	
	return 0;
}
