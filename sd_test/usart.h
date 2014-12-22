/*
 * usart.h
 *
 * Created: 26/11/2014 4:05:19 PM
 *  Author: benj1
 */ 
#include <stdio.h>

#ifndef USART_H_
#define USART_H_

#define BAUD_RATE 9600
#define F_CPU 16000000
#define BAUD_RATE_UBRR (F_CPU/16/BAUD_RATE - 1)
#define BUFFER_SIZE 20

unsigned char usart_rx_buffer[BUFFER_SIZE];

void usart_init(void);
void usart_write(uint8_t byte);
uint8_t usart_read(void);
int usart_stream_write(char c, FILE *stream);
void usart_disable_receive_interrupt(void);

#define TRANSMIT 1

#if (TRANSMIT == 1)
	static FILE usart_stdout = FDEV_SETUP_STREAM(usart_stream_write, NULL, _FDEV_SETUP_WRITE);
#else
	static FILE usart_stdout = FDEV_SETUP_STREAM(NULL, NULL, _FDEV_SETUP_WRITE);
#endif

#endif /* USART_H_ */