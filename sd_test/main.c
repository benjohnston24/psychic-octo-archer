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
uint8_t *data[BLOCK_SIZE];


/*void copy_data(void)
{
	for (uint16_t k = 0; k < BLOCK_SIZE; k++)
	{
		data_to_write[k] = data[k];
		//printf("%d, ", data_to_write[k]);
	}
}*/

//Interrupt service routine for analogue to digital converter
ISR(ADC_vect)
{
	uint16_t adc_data = ADCW;
	//printf("%d: %d\n", channel, ADCW);
	ADMUX &= 0xF0;                     //Clear the older channel that was read
	ADMUX |= channel;                	//Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);               //Starts a new conversion		
	//printf("%d\n", fake_data);	
	data[data_counter++] = fake_data++;
	if (data_counter > (BLOCK_SIZE - 1))	
	{
		//printf("Trigger Write\n");
		//printf("Data Counter: %d", data_counter);
		//copy_data();
		//data = data_start;
		data_counter = 0;
		block_counter++;
        trigger_log = 1;		
	}
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
			sei();
			timer_running = 1;
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
			//printf("Trigger Log: %d\n", trigger_log);
			if (trigger_log == 1)
			{
				//timer1_stop();
				//adc_stop();				
				uint16_t memory_block = block_counter * BLOCK_SIZE;
				//printf("Memory Block: %x\n", memory_block);
				if (write_sector(memory_block >> 8,memory_block & 0x00FF, data) != 1)	
				{
					cli();
					timer1_stop();
					adc_stop();					
					step = 7;
				}
				//data = data_start;
				//timer1_start();
				//adc_start();				
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
			 printf("Block Counter: %d\n", block_counter);
			 usart_read();
			 for (uint16_t i = 0; i <= block_counter; i++)
			 {
				
				memset(data, 0x00, BLOCK_SIZE);
				uint16_t memory_block = i * BLOCK_SIZE;
				if(read_sector(memory_block >> 8,memory_block & 0x00FF, data) == 1)
				{
					printf("Read Block: %d\n", i);
					//for(uint16_t j = 0; j < BLOCK_SIZE; j++)
					//{
					//	printf("%d, ", data[j]);
					//}
				}
				else
				{
					printf("Error reading back data\n");
				}
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
