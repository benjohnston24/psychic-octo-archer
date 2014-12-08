/*
 * timing.h
 *
 * Created: 1/12/2014 3:20:45 PM
 *  Author: benj1
 */ 


#ifndef TIMING_H_
#define TIMING_H_


void timer1_stop (void);
void timer0_compa_init(void);
void timer0_free_run_init(void);
void timer1_overflow_init(void);
void timer1_stop(void);
void timer1_start(void);


#endif /* TIMING_H_ */