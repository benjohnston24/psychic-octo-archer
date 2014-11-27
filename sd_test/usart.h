/*
 * usart.h
 *
 * Created: 26/11/2014 4:05:19 PM
 *  Author: benj1
 */ 
#include <stdio.h>

#ifndef USART_H_
#define USART_H_



#define BAUD_RATE 57600

#if (BAUD_RATE == 115200)
#define BAUD_RATE_UBRR 8
#elif (BAUD_RATE == 57600)
#define BAUD_RATE_UBRR 16
#else
#define BAUD_RATE_UBRR (F_CPU/16/BAUD_RATE - 1)
#endif

void uart_init(void);
void uart_write(uint8_t byte);
int uart_stream_write(char c, FILE *stream);

static FILE uart_stdout = FDEV_SETUP_STREAM(uart_stream_write, NULL, _FDEV_SETUP_WRITE);

#endif /* USART_H_ */