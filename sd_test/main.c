/*
 * main.c
 *
 * Created: 26/11/2014 4:01:32 PM
 *  Author: benj1
 */ 

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"
#include "spi.h"
#include "sd.h"
#include "adc.h"
#include "timing.h"

uint16_t step = 0, data_counter = 0, block_counter = 1;
uint8_t counter = 0, duration = 0, channel = 1, timer_running = 0, trigger_log = 0, fake_data = 0;
uint8_t *data, *data_start, *data_to_write;


void copy_data(void)
{
	for (uint16_t k = 0; k < BLOCK_SIZE; k++)
	{
		data_to_write[k] = data[k];
		//printf("%d, ", data_to_write[k]);
	}
}

//Interrupt service routine for analogue to digital converter
ISR(ADC_vect)
{
	uint16_t adc_data = ADCW;
	//printf("%d: %d\n", channel, ADCW);
	//ADMUX &= 0xF0;                     //Clear the older channel that was read
	//ADMUX |= channel;                	//Defines the new ADC channel to be read
	//printf("%d\n", fake_data);	
	data[data_counter++] = fake_data++;
	if (data_counter > (BLOCK_SIZE - 1))	
	{
		/*printf("\nOriginal Data:\n");
		for (uint16_t k = 0; k < BLOCK_SIZE; k++)
		{
			printf("%d, ", data[k]);
		}*/	
		
		//printf("\nData being copied:\n");	
		copy_data();
		trigger_log = 1;
		data = data_start;
		data_counter = 0;
		block_counter++;
	}
	
	ADCSRA |= (1<<ADSC);               //Starts a new conversion	
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
	if (counter >= 1)
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
			printf("\n\n\n\n\n-------------------------------------------\n");
			printf("Systems Configured\n");
			data = calloc(BLOCK_SIZE, sizeof(uint8_t));
			data_to_write = calloc(BLOCK_SIZE, sizeof(uint8_t));
			data_start = data;
			if((data != NULL) | (data_to_write != NULL))
			{
				step = 1;
			}
			else
			{
				printf("Error allocating memory for test data\n");
				step = 999;
			}
			break;
			
	    case 1: 	// Configure the SPI
			spi_init();
			printf("SPI Configured\n");
			if(sd_init() == 1){
				printf("SD Initialised\n");
				step = 2;}
			else{ step = 999;}
			break;
			
		case 2: //Wait for receipt of duration of test
			printf("Requesting test duration:\n");		    
			//duration = usart_read();
			printf("Received %d\n", duration);
			//Enable interrupts
			//sei();
			timer_running = 1;
			step = 3;//5;
			break;
			
		case 3:	//Write data
			memset(data, 0x32, BLOCK_SIZE);
			if(write_sector(0x12, 0x34, data) == 1){step = 4;}
			else{ step = 999;}
			printf("Data written\n");
			break;
			
		case 4:	//Read data back;
			memset(data, 0x00, BLOCK_SIZE);
			if(read_sector(0x12, 0x34, data) == 1){
				for(uint16_t i = 0; i < BLOCK_SIZE; i++)
				{
					printf("0x%x, ", data[i]);
				}
				step = 101;
			}
			else{ step = 999;}
			break;
			
		case 5: //Test reading time
			if (timer_running == 0)
			{
				step = 7;
			}
			if (trigger_log == 1)
			{
				uint16_t memory_block = block_counter * BLOCK_SIZE;
				memset(data_to_write, 0x32, BLOCK_SIZE);
				for (uint16_t k = 0; k < BLOCK_SIZE; k++)
				{
					printf("%d, ", data_to_write[k]);
				}			
				if (write_sector((memory_block >> 8),
				                  (memory_block), data_to_write) != 1)
				{
					cli();
					timer1_stop();
					adc_stop();					
					step = 999;
				}
				printf("%x\n",  memory_block);
				printf("%x\n",  (memory_block >> 8));
				printf("%x\n",  (memory_block & 0x00FF));
			}
			break;
			
		case 6:
			step=5;
			break;
			
		case 7:
		     cli();
			 timer1_stop();
			 adc_stop();
			 printf("Done\n");
			 //Print the contents of the data
			 printf("Block Counter: %d\n", block_counter);
			 printf("Data Counter: %d\nData:\n", data_counter);
			 for (uint16_t k = 0; k < data_counter; k++)
			 {
				 printf("%d, ", data[k]);
			 }
			 printf("\nCopied Data:\n");
			 for (uint16_t k = 0; k < data_counter; k++)
			 {
				 printf("%d, ", data_to_write[k]);
			 }			 
			 counter = 0;
			 step = 1000;
			 //Need to free data
			 break;
				
		case 101: printf("Done\n"); step = 1000; HIGH_CS(); break;
		case 999: printf("Error\n"); step = 1000; HIGH_CS(); break;
		
		case 1000: while(1);
		
			}
	}
	
	return 0;
}
