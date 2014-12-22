/*
 * main.c
 *
 * Created: 26/11/2014 4:01:32 PM
 *  Author: benj1
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"
#include "spi.h"
#include "sd.h"
#include "adc.h"
#include "timing.h"

uint16_t step = 0, data_counter = 0, block_counter = 1, memory_block = 0;
uint8_t counter = 0, duration = 0, channel = 1, timer_running = 0, trigger_log = 0, fake_data = 0, trigger_start = 0;
uint8_t usart_data = 0, error_flag = 0, downsample_counter = 0;
uint8_t data[BLOCK_SIZE];
const char *connect = "OK\n";

//#define DEBUG_MAIN 1


//Interrupt service routine for analogue to digital converter
ISR(ADC_vect)
{
	/*if (trigger_log == 1)
	{
		#ifdef DEBUG_MAIN
			printf("Waiting for logging\n");
		#endif
		return;
	}*/
	//Collect data from ADC
	//uint16_t adc_data = ADCW;
	//printf("%d: %d\n", channel, ADCW);		
	//printf("%d\n", fake_data);
	//Check the data is being collected correctly
	if (downsample_counter == 0)
	{
		data[data_counter++] = channel;	
		/********************TEMP******************************/
		data[data_counter++] = (uint16_t) fake_data++;		
	}
	
	
	//If more than a block of data has been collected, trigger logging
	if (data_counter > (BLOCK_SIZE - 1))	
	{
		//#ifdef DEBUG_MAIN
		//	printf("Data collected, block %d full\n", block_counter);
		//#endif
		data_counter = 0;
        trigger_log = 1;		
	}
	
	// Prepare for the next ADC read
	ADMUX &= 0xF0;                     //Clear the older channel that was read
	ADMUX |= channel++;                	//Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);               //Starts a new conversion	
	
	//Only read channels 1 to 3
	if (channel >= 4)
	{
		
		downsample_counter++;
		channel = 1;
		if (downsample_counter >= 4)
		{
			downsample_counter = 0;
		}
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

ISR(USART_RX_vect)
{
	unsigned char data = UDR0;
	usart_rx_buffer[counter++] = data;
	if (usart_rx_buffer[counter - 1] == '\n')
	{
		counter = 0;
		if (strstr(usart_rx_buffer, connect) != NULL)
		{
			trigger_start = 1;
		}
	}
	
}

int main(void)
{	
	while (1){
	switch (step){
		
		case 0: 
			/* Init the UART */
			usart_init();
			stdout = &usart_stdout;		
			
			// Initialise / reset the values
			step = 0; 
			data_counter = 0; 
			block_counter = 1;
			memory_block = 0;
			counter = 0; 
			duration = 0; 
			channel = 1; 
			timer_running = 0; 
			trigger_log = 0; 
			fake_data = 0; 
			trigger_start = 0;
			usart_data = 0;	
			error_flag = 0;
			//realloc(data, BLOCK_SIZE * sizeof(uint8_t));		
			sei();
			
			//Move to next step
			step = 5;
			break;
			
		case 5:
			//Wait for pairing to be completed
			if(trigger_start == 1)
			{
				cli();
				printf("Paired\nCollecting Data\n");				
				step = 10;
			}
			else
			{
				printf("Pairing\n");
			}
			break;
			
	    case 10:
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
		    step = 20;
	    }
	    else{ step = 999;}
	    break;					
			
		case 20:
			timer1_overflow_init();
			channel = 1;
            adc_init();
			#ifdef DEBUG_MAIN
				printf("\n\n\n\n\n-------------------------------------------\n");
				printf("Systems Configured\n");
			#endif
			
			step = 30;
			break;
			
		case 30: //Wait for receipt of duration of test
			/*#ifdef DEBUG_MAIN 
				printf("Requesting test duration:\n"); 
			#endif
			duration = usart_read();
			#ifdef DEBUG_MAIN 
				printf("Received %d\n", duration); 
			#endif*/
			
			//Enable interrupts
			sei();
			timer_running = 1;
			step = 40;
			break;
				
		case 40: 
			//If the timer is no longer running exit the state
			if (timer_running == 0)
			{
				if (error_flag == 1)
				{
					step = 999;
				}
				else
				{
					//Stop the timer and the ADC
					timer1_stop();
					adc_stop();					
					trigger_start = 0;
					step = 45;
				}
			}
			//If a log request has been trigger, log the data
			if (trigger_log == 1)
			{			
				memory_block = block_counter * BLOCK_SIZE;
				#ifdef DEBUG_MAIN				
					printf("Logging block counter %d\n", block_counter);
					printf("Logging memory block %x\n", memory_block);	
				#endif			
				if (write_sector(memory_block >> 8,memory_block & 0x00FF, data) != 1)	
				{
					//An error has occurred during writing
					//Stop the timer and branch to the complete state
					cli();
					timer1_stop();
					adc_stop();					
					printf("SD Card write error\n");
					printf("Memory block %d\n", memory_block);
					printf("Memory block counter %d\n", block_counter);
					error_flag = 1;
					step = 999;
					break;
				}
				else
				{
					#ifdef DEBUG_MAIN
						printf("Write complete\n");
					#endif
					block_counter++;
					trigger_log = 0;
					sei();
					timer1_start();
					adc_start();
				}				
			}
			break;
			
		case 45:
			 //Wait to receive signal to transmit data
			 printf("Ready for data?\n");
			 if(trigger_start == 1)
			 {
				 step = 50;
			 }
			 break;
		
		case 50:
			 //Data collection complete
			 //Stop timers
		     cli();
			 timer1_stop();
			 adc_stop();
			 printf("Blocks to read, %d\n", block_counter);		 
			 for (uint16_t i = 0; i <= block_counter; i++)
			 {
				
				memset(data, 0x00, BLOCK_SIZE);
				memory_block = i * BLOCK_SIZE;
				printf("Data for block, %d\n", i);
				if(read_sector(memory_block >> 8,memory_block & 0x00FF, data) == 1)
				{
					for(uint16_t j = 0; j < BLOCK_SIZE; j++)
					{
						printf("%d, ", data[j]);
					}
				}
				else
				{
					printf("Error reading back data\n");
					step = 999;
					break;
				}
				printf("\n");
			}				 		 		 
			 counter = 0;
			 step = 60;
			 #ifdef DEBUG_MAIN
				printf("Blocks read\n");
			 #endif
			 //Need to free data
			 printf("Done\n");
			 break;
				
		case 60: step = 1000; HIGH_CS(); break;
		case 999: printf("Error\n"); step = 1000; HIGH_CS(); break;
		
		case 1000: while(1);
		
			}
	}
	
	return 0;
}
