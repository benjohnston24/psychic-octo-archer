/*
 * adc.h
 *
 * Created: 2/12/2014 4:03:53 PM
 *  Author: benj1
 */ 


#ifndef ADC_H_
#define ADC_H_

void adc_init(void);
uint16_t read_adc(uint8_t channel);
void adc_stop(void);
void adc_start(void);


#endif /* ADC_H_ */