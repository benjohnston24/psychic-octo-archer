/*
 * usart.c
 *
 * Created: 26/11/2014 4:04:15 PM
 *  Author: benj1
 */ 
#include <stdio.h>
#include <avr/io.h>
#include "usart.h"

void uart_init(void)
{
	// Setup the UART baud rate
	UBRR0H = (BAUD_RATE_UBRR >> 8);
	UBRR0L = BAUD_RATE_UBRR;

	// Enable the UART receiver and transceiver
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	// Set the frame format : 8 bits data, 1 bit stop
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void uart_write(uint8_t byte)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = byte;
}

int uart_stream_write(char c, FILE *stream)
{
	if (c == '\n')
	uart_stream_write('\r', stream);

	uart_write(c);
	return 0;
}
