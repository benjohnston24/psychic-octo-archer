/*
 * sd.h
 *
 * Created: 26/11/2014 4:23:38 PM
 *  Author: benj1
 */ 


#ifndef SD_H_
#define SD_H_

#define CMD_SIZE 6
#define GO_IDLE_STATE 0 //Software reset
#define SEND_OP_COND 1 //Initiate initialisation process
#define READ_CSD 9 //Read CSD register
#define READ_CID 10 //Read CID register
#define STOP_TRANSMISSION 12 //Stop to read data
#define SET_BLOCKLEN 16 //Change R/W block size
#define READ_SINGLE_BLOCK 17 //Read a block of data
#define READ_MULTIPLE_BLOCKS 18 //Read multiple blocks of data
#define WRITE_SINGLE_BLOCK 24 //Write a single block of data
#define WRITE_MULTIPLE_BLOCKS 25 //Write multiple blocks of data
#define CMD55 0
#define CMD58 0

#define SEND_CYCLES 0xFF //Value to send to SD card when need clock cycles
#define IDLE_CRC 0x95 //CRC for Idle command
#define NO_CRC 0x00 //Value to use when CRC not required
#define IN_IDLE 0x01 //Response when the SD card enters idle
#define OK 0x00 //Valid response
#define BUSY 0x00 //Response received when SD card is busy
#define BLOCK_SIZE 256 //Size of a single block
#define BLOCK_LEN_HIGH 0x02 //512 bytes, high byte
#define BLOCK_LEN_LOW 0x00 //512 bytes, low byte
#define DATA_TOKEN 0xFE //Data token
#define DATA_WRITTEN 0xe5 //Token received when data is written

#define INT_TO_HIGH_BYTE(integer)	(uint8_t) (integer >> 8)
#define INT_TO_LOW_BYTE(integer)	(uint8_t) integer

void send_command(uint8_t CMD, unsigned int argH, unsigned int argL, uint8_t CRC);
uint8_t check_response(uint8_t test_response);
uint8_t sd_init(void);
uint8_t write_sector(unsigned int addressH, unsigned int addressL, uint8_t* data);
uint8_t read_sector(unsigned int addressH, unsigned int addressL, uint8_t* data);

#endif /* SD_H_ */