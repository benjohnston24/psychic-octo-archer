/*
 * spi.h
 *
 * Created: 26/11/2014 4:07:20 PM
 *  Author: benj1
 */ 

#include <avr/io.h>

#ifndef SPI_H_
#define SPI_H_

#define HIGH_CS()   PORTB |= _BV(DDB2);
#define LOW_CS()	PORTB &= ~_BV(DDB2);

void spi_init(void);
char spi_receive_byte(void);
void spi_send(uint8_t* data, uint16_t length);

#endif /* SPI_H_ */