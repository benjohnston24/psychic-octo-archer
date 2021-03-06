/*
 * sd.c
 *
 * Created: 26/11/2014 4:23:30 PM
 *  Author: benj1
 */ 

#include <stdio.h>
#include <util/delay.h>
#include "spi.h"
#include "sd.h"

void send_command(uint8_t CMD, uint16_t argH, uint16_t argL, uint8_t CRC)
{
	uint8_t data_array[CMD_SIZE];
	//SD card required the MSB to be sent first
	uint16_t addrH = (argH << 9);	
	uint16_t addrL = (argL << 9);	
	
	//01 is required to be at the start of the byte
	data_array[0] = CMD ^ 0x40;
	data_array[1] = ((uint8_t) addrH) && 0xFF;	
	data_array[2] = (addrH) >> 8;
	data_array[3] = (addrL) >> 8;	
	data_array[4] = ((uint8_t) addrL) && 0xFF;
	data_array[5] = CRC;
	
	spi_send(data_array, CMD_SIZE);	
}

void set_block_length(uint16_t block_len)
{
	uint8_t data_array[CMD_SIZE];
	data_array[0] = SET_BLOCKLEN ^ 0x40;
	data_array[1] = 0x00;
	data_array[2] = 0x00;
	data_array[3] = 0x02;
	data_array[4] = 0x00;
	data_array[5] = NO_CRC;
	
	spi_send(data_array, CMD_SIZE);	
}

uint8_t check_response(uint8_t test_response)
{
	uint8_t response;
	for(uint8_t i = 0; i < 255; i++)
	{
		response = spi_receive_byte();
		if (response == test_response)
		{
			//Correct response received
			return 1;
		}
	}
	//Response not received in time
	return 0;
}

uint8_t sd_init(void)
{
	HIGH_CS();
	
	//Send 80 clk cycles to reset
	for(int i = 0; i < 10; i++){
	    spi_receive_byte();}
		
	LOW_CS();
	
	#ifdef DEBUG_SD
	printf("SD Send idle command\n");
	#endif
	
	//Request the card to go into idle state
    send_command(GO_IDLE_STATE, 0x00, 0x00, IDLE_CRC);
	
	#ifdef DEBUG_SD
	printf("SD Waiting for response\n");
	#endif		
	
	//Check for the correct response
	if (check_response(IN_IDLE) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card idle timeout\n");
		#endif		
		HIGH_CS();		
		return 0;
	}
	
	HIGH_CS();	
	#ifdef DEBUG_SD
	printf("SD In idle state\n");
	#endif	

	spi_receive_byte();
	LOW_CS();
	
	//printf("Send OP command\n");
	
	//Initialise SD card
	uint8_t i;
	for(i = 0; i < 255; i++)
	{
		send_command(SEND_OP_COND, 0x00, 0x00, NO_CRC);
		if (check_response(OK) == 1)
		{
			break;
		}
	}
	
	//Check for the correct response
	if (i == 254)
	{
		#ifdef DEBUG_SD
		printf("SD card initialise timeout\n");
		#endif
		HIGH_CS();			
		return 0;
	}	
	
	#ifdef DEBUG_SD
	printf("SD In initial state\n");
	#endif	
	
	HIGH_CS();
	
	//Send some clock cycles
	spi_receive_byte();
	
	LOW_CS();	
	
	//Set the block length
	set_block_length(SET_BLOCKLEN);
	
	//Check for the correct response
	if (check_response(OK) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card block length timeout\n");
		#endif		
		HIGH_CS();			
		return 0;
	}	
	
	#ifdef DEBUG_SD
	printf("SD Block length set\n");
	#endif	
	//When finished send CS high
	HIGH_CS();
	return 1;	
}

uint8_t write_sector(uint16_t addressH, uint16_t addressL, uint8_t* data)
{	
	
	LOW_CS();
	
	//Send write command
	send_command(WRITE_SINGLE_BLOCK, addressH, addressL, NO_CRC);
	
	spi_receive_byte();
	
	//Check for the correct response
	if (check_response(OK) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card write request timeout\n");
		#endif		
		HIGH_CS();			
		return 0;
	}
	
	//Send one byte gap
	spi_receive_byte();
	
	//Send start data token
	spi_send_byte(DATA_TOKEN);
	
	//Send data
	spi_send(data, BLOCK_SIZE);
	
	//Send CRC
	spi_send_byte(0xFF);
	spi_send_byte(0xFF);	
		
	#ifdef DEBUG_SD
	printf("SD Waiting for data to be written\n");
	#endif		
	//Check response
	if (check_response(DATA_WRITTEN) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card data not written correctly\n");
		#endif
		HIGH_CS();			
		return 0;
	}
	
	//Wait until write procedure is complete
	#ifdef DEBUG_SD
	printf("SD waiting for write to be done\n");
	#endif
	
	for (uint16_t i = 0;  i < 1024; i++)
	{
		if(spi_receive_byte() != 0x00)
		{
			//Complete
			#ifdef DEBUG_SD
			printf("SD card write done\n");
			#endif				
			return 1;
		}
	}
	
	#ifdef DEBUG_SD
	printf("SD waiting for write to be done\n");
	printf("SD Status request\n");
	#endif
	
	send_command(STATUS_REQUEST, 0, 0, NO_CRC);
	if (check_response(OK) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card write status request timeout\n");
		#endif		
		HIGH_CS();
		return 0;
	}		
	
	if (check_response(OK) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card write R1 error\n");
		#endif		
		HIGH_CS();
		return 0;
	}
	
	if (check_response(OK) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card write R2 error\n");
		#endif		
		HIGH_CS();
		return 0;
	}		
	
	HIGH_CS();
	return 1;
	
	
}

uint8_t read_sector(uint16_t addressH, uint16_t addressL, uint8_t* data)
{
	LOW_CS();
		
	//Send read command
	send_command(READ_SINGLE_BLOCK, addressH, addressL, NO_CRC);
	
	#ifdef DEBUG_SD
	printf("SD Waiting for read response\n");
	#endif
	
	//Check for the correct response
	if (check_response(OK) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card read request timeout\n");
		#endif		
		HIGH_CS();			
		return 0;
	}
	
	#ifdef DEBUG_SD
	printf("SD Waiting for data token\n");
	#endif
		
	//Wait for data token
	if (check_response(DATA_TOKEN) != 1)
	{
		#ifdef DEBUG_SD
		printf("SD card read data token timeout\n");
		#endif		
		HIGH_CS();
		return 0;
	}
	
	#ifdef DEBUG_SD
	printf("SD Data Token Received\n");
	#endif	
	
	//Get data
	#ifdef DEBUG_SD
	printf("SD Waiting for Data\n");
	#endif	
			
	for (uint16_t i = 0; i < BLOCK_SIZE; i++)
	{
		data[i] = spi_receive_byte();
	}
	
	#ifdef DEBUG_SD
	printf("SD Data Received\n");
	#endif
		
	
	//Discard the CRC
	spi_receive_byte();
	spi_receive_byte();
	
	HIGH_CS();
	return 1;
		
}


