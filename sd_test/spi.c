/*
 * spi.c
 *
 * Created: 26/11/2014 4:07:32 PM
 *  Author: benj1
 */ 

#include <avr/io.h>

#include <stdio.h>
#include <avr/io.h>

//Pin allocations
#define DDR_SPI DDRB
#define DD_CS   DDB2
#define DD_MOSI DDB3
#define DD_MISO DDB4
#define DD_SCK  DDB5

//
#define CMD_SIZE 6

void spi_init(void)
{
	/* Set MOSI and SCK output */
	DDR_SPI |= _BV(DD_MOSI) | _BV(DD_SCK) | _BV(DD_CS);
	DDR_SPI &= ~_BV(DD_MISO);
	
	/* Enable SPI, Master, set clock rate fck/128 */
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1);
	SPSR = _BV(SPI2X);
}

void spi_send_byte(uint8_t byte)
{
	SPDR = byte;
	//Wait for transmission
	while(!(SPSR & (1<<SPIF)));
	
}

uint8_t spi_receive_byte(void)
{
	uint8_t data = 0xFF;
	SPDR = data;
	
	//Wait for response
	while(!(SPSR & (1<<SPIF)));
	data = SPDR;
	
	return data;
}

void spi_send(uint8_t* data, uint16_t length)
{
	for(int i=0; i < length; i++)
	{
		spi_send_byte(data[i]);
		printf("%d ", data[i]);
	}
	printf("\n");
}


