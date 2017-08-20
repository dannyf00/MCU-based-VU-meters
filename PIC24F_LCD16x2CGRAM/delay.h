#ifndef __DELAY_H
#define __DELAY_H

#include "gpio.h"				//we use _nop_()

void delay(volatile int dly);

void delay_us(volatile unsigned int us);

void delay_ms(volatile unsigned int ms);

#endif //delay_h_
