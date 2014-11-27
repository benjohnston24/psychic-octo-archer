/*
 * main.c
 *
 * Created: 26/11/2014 4:01:32 PM
 *  Author: benj1
 */ 

#include <avr/io.h>
#include <avr/delay.h>
#include "usart.h"
#include "spi.h"
#include "sd.h"

void main(void)
{
	uint16_t step = 0;
	uint8_t data[BLOCK_SIZE];
	uint8_t data_back[BLOCK_SIZE];		
	
	while (1){
	switch (step){
		
		case 0: 
		    DDRD |= (0 << PORTD2);
			/* Init the UART */
			uart_init();
			stdout = &uart_stdout;
			printf("USART Configured\n");
			//printf("Go\n");
			step = 1;
			break;
			
	    case 1: 	// Configure the SPI
			spi_init();
			printf("SPI Configured\n");
			step = 2;
			break;
		
		case 2:	//Reset
			if(sd_init() == 1){step = 3;}
			else{ step = 999;}
			printf("SD Initialised\n");
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
				
		case 101: printf("Done\n"); step = 1000; HIGH_CS(); break;
		case 999: printf("Error\n"); step = 1000; HIGH_CS(); break;
		
		case 1000: while(1);
		
			}
	}
	
	return 0;
}
