//PIC16F684 based VU meter
//drive leds driven with multiplex
//1x4x6 LEDs: 1=single channel, 4=led groups, 6=leds in a group
//led designation
//1x4x6: 1 channel, 4 led pins to cathode and 6 pins to anode
//ie. 4 groups of 6 pins common anodes

#include <stdlib.h>							//we use rand()
#include "config.h"						//configuration words
#include "gpio.h"                           //we use gpio functions
#include "delay.h"                          //we use software delays
#include "adc.h"							//we use adc
#include "tmr1.h"							//we use tmr1

//hardware configuration
#define LED1_PORT		PORTC
#define LED1_DDR		TRISC
#define LED1			(1<<0)

#define LED2_PORT		PORTC
#define LED2_DDR		TRISC
#define LED2			(1<<1)

#define LED3_PORT		PORTC
#define LED3_DDR		TRISC
#define LED3			(1<<2)

#define LED4_PORT		PORTC
#define LED4_DDR		TRISC
#define LED4			(1<<3)

#define LED5_PORT		PORTC
#define LED5_DDR		TRISC
#define LED5			(1<<4)

#define LED6_PORT		PORTC
#define LED6_DDR		TRISC
#define LED6			(1<<5)

#define DIG1_PORT		PORTA
#define DIG1_DDR		TRISA
#define DIG1			(1<<1)

#define DIG2_PORT		PORTA
#define DIG2_DDR		TRISA
#define DIG2			(1<<2)

#define DIG3_PORT		PORTA
#define DIG3_DDR		TRISA
#define DIG3			(1<<4)

#define DIG4_PORT		PORTA
#define DIG4_DDR		TRISA
#define DIG4			(1<<5)

#define VU_TICKS		3000			//timer1 ticks
#define VU_PS			TMR1_PS_1x		//vu prescaler
#define VU_AN			ADC_AN0			//vu signal adc input channel
#define VU_DEC			1				//vu hold decrement
#define VU_DLY			2				//waste some time, in ms
//end hardware configuration

//global defines
//digit macros -> active low (mcu pin to cathode)
#define DIG_OFF(port, pins)	IO_SET(port, pins)
#define DIG_ON(port, pins)	IO_CLR(port, pins)
//led macros -> active high (mcu pin to anode)
#define LED_OFF(port, pins)	IO_CLR(port, pins)
#define LED_ON(port, pins)	IO_SET(port, pins)

//global variables
uint8_t vu_input=0;						//vu meter signal input - adc
uint8_t vu_val;							//digits to be displayed, 0=blank, 1=led1, 2=led1+2, 3=led1+2+3, ..., up to led24
uint8_t vu_peak;						//digit for peak hold. 0=blank, 1=led1, 2=led2, ..., up to led24
const uint8_t *vu_tab;					//vu table, either linear or log, or user defined (optional)
//vu table
//linear
const uint8_t vu_lin[]={
	  5,  16,  26,  37,  48,  58,  69,  80,  90, 101, 112, 122,
	133, 144, 154, 165, 176, 186, 197, 208, 218, 229, 240, 250,
};
	
//log
const uint8_t vu_log[]={
	  2,   3,   4,   5,   6,   7,   8,   9,  20,  11,  12,  15,
	 19,  25,  31,  39,  49,  62,  79,  99, 125, 157, 198, 250,
};

uint8_t vu_eeprom[24];					//store vu_eeprom data

//tmr1 isr
void interrupt isr(void) {
	if (TMR1IF) tmr1_isr();				//run tmr1 isr
}

//update the vu based on vu_val and vu_peak
void vu_update(void) {
	static uint8_t dig=0;				//current digit.0=dig1, 1=dig2, ...
	uint8_t digx6=dig*6;				//=digx6

	//IO_FLP(LED1_PORT, LED1);			//flip the led - debugging only
	
	//turn off all digits - NOP() for RMW
	DIG_OFF(DIG1_PORT, DIG1); NOP4();
	DIG_OFF(DIG2_PORT, DIG2); NOP4();
	DIG_OFF(DIG3_PORT, DIG3); NOP4();
	DIG_OFF(DIG4_PORT, DIG4); NOP4();

	//process vu_val
#if 1
	switch (dig) {
		case 0: 
			if (vu_val >=  1) LED_ON(LED1_PORT, LED1); else LED_OFF(LED1_PORT, LED1); 
			if (vu_val >=  2) LED_ON(LED2_PORT, LED2); else LED_OFF(LED2_PORT, LED2); 
			if (vu_val >=  3) LED_ON(LED3_PORT, LED3); else LED_OFF(LED3_PORT, LED3); 
			if (vu_val >=  4) LED_ON(LED4_PORT, LED4); else LED_OFF(LED4_PORT, LED4); 
			if (vu_val >=  5) LED_ON(LED5_PORT, LED5); else LED_OFF(LED5_PORT, LED5); 
			if (vu_val >=  6) LED_ON(LED6_PORT, LED6); else LED_OFF(LED6_PORT, LED6); 
			break;
		case 1: 
			if (vu_val >=  7) LED_ON(LED1_PORT, LED1); else LED_OFF(LED1_PORT, LED1); 
			if (vu_val >=  8) LED_ON(LED2_PORT, LED2); else LED_OFF(LED2_PORT, LED2); 
			if (vu_val >=  9) LED_ON(LED3_PORT, LED3); else LED_OFF(LED3_PORT, LED3); 
			if (vu_val >= 10) LED_ON(LED4_PORT, LED4); else LED_OFF(LED4_PORT, LED4); 
			if (vu_val >= 11) LED_ON(LED5_PORT, LED5); else LED_OFF(LED5_PORT, LED5); 
			if (vu_val >= 12) LED_ON(LED6_PORT, LED6); else LED_OFF(LED6_PORT, LED6); 
			break;
		case 2: 
			if (vu_val >= 13) LED_ON(LED1_PORT, LED1); else LED_OFF(LED1_PORT, LED1); 
			if (vu_val >= 14) LED_ON(LED2_PORT, LED2); else LED_OFF(LED2_PORT, LED2); 
			if (vu_val >= 15) LED_ON(LED3_PORT, LED3); else LED_OFF(LED3_PORT, LED3); 
			if (vu_val >= 16) LED_ON(LED4_PORT, LED4); else LED_OFF(LED4_PORT, LED4); 
			if (vu_val >= 17) LED_ON(LED5_PORT, LED5); else LED_OFF(LED5_PORT, LED5); 
			if (vu_val >= 18) LED_ON(LED6_PORT, LED6); else LED_OFF(LED6_PORT, LED6); 
			break;
		case 3: 
			if (vu_val >= 19) LED_ON(LED1_PORT, LED1); else LED_OFF(LED1_PORT, LED1); 
			if (vu_val >= 20) LED_ON(LED2_PORT, LED2); else LED_OFF(LED2_PORT, LED2); 
			if (vu_val >= 21) LED_ON(LED3_PORT, LED3); else LED_OFF(LED3_PORT, LED3); 
			if (vu_val >= 22) LED_ON(LED4_PORT, LED4); else LED_OFF(LED4_PORT, LED4); 
			if (vu_val >= 23) LED_ON(LED5_PORT, LED5); else LED_OFF(LED5_PORT, LED5); 
			if (vu_val >= 24) LED_ON(LED6_PORT, LED6); else LED_OFF(LED6_PORT, LED6); 
			break;
	}
#else
	if (vu_val >= digx6+1) LED_ON(LED1_PORT, LED1); else LED_OFF(LED1_PORT, LED1); NOP4();
	if (vu_val >= digx6+2) LED_ON(LED2_PORT, LED2); else LED_OFF(LED2_PORT, LED2); NOP4();
	if (vu_val >= digx6+3) LED_ON(LED3_PORT, LED3); else LED_OFF(LED3_PORT, LED3); NOP4();
	if (vu_val >= digx6+4) LED_ON(LED4_PORT, LED4); else LED_OFF(LED4_PORT, LED4); NOP4();
	if (vu_val >= digx6+5) LED_ON(LED5_PORT, LED5); else LED_OFF(LED5_PORT, LED5); NOP4();
	if (vu_val >= digx6+6) LED_ON(LED6_PORT, LED6); else LED_OFF(LED6_PORT, LED6); NOP4();
#endif

	//process vu_peak
	if (vu_peak == digx6+1) LED_ON(LED1_PORT, LED1);
	if (vu_peak == digx6+2) LED_ON(LED2_PORT, LED2);
	if (vu_peak == digx6+3) LED_ON(LED3_PORT, LED3);
	if (vu_peak == digx6+4) LED_ON(LED4_PORT, LED4);
	if (vu_peak == digx6+5) LED_ON(LED5_PORT, LED5);
	if (vu_peak == digx6+6) LED_ON(LED6_PORT, LED6);

	//advance to the next digit
	switch (dig) {
		case 0: DIG_ON(DIG1_PORT, DIG1); dig=1; break;
		case 1: DIG_ON(DIG2_PORT, DIG2); dig=2; break;
		case 2: DIG_ON(DIG3_PORT, DIG3); dig=3; break;
		case 3: DIG_ON(DIG4_PORT, DIG4); dig=0; break;
	}	
}
		
//initialize the vu pins
void vu_init(void) {
	uint8_t tmp;
	
	//all dig pins output, off
	DIG_OFF(DIG1_PORT, DIG1); IO_OUT(DIG1_DDR, DIG1);
	DIG_OFF(DIG2_PORT, DIG2); IO_OUT(DIG2_DDR, DIG2);
	DIG_OFF(DIG3_PORT, DIG3); IO_OUT(DIG3_DDR, DIG3);
	DIG_OFF(DIG4_PORT, DIG4); IO_OUT(DIG4_DDR, DIG4);

	//all led pins output, off
	LED_OFF(LED1_PORT, LED1); IO_OUT(LED1_DDR, LED1);
	LED_OFF(LED2_PORT, LED2); IO_OUT(LED2_DDR, LED2);
	LED_OFF(LED3_PORT, LED3); IO_OUT(LED3_DDR, LED3);
	LED_OFF(LED4_PORT, LED4); IO_OUT(LED4_DDR, LED4);
	LED_OFF(LED5_PORT, LED5); IO_OUT(LED5_DDR, LED5);
	LED_OFF(LED6_PORT, LED6); IO_OUT(LED6_DDR, LED6);

	//read data to / from eeprom
	if (eeprom_read(0x00) == 0xff) {		//then eeprom isn't written anything
		vu_tab = vu_log;					//default vu table is logrithmic
	} else {								//there is data in eeprom
		for (tmp = 0; tmp < 24; tmp++) vu_eeprom[tmp]=eeprom_read(tmp);
		vu_tab = vu_eeprom;
	}	

	//reset the adc
	vu_val = vu_peak = vu_input = 0;		//initialize vu input
	vu_tab = vu_lin;						//default vu table = log
	adc_init();								//reset the adc
	
	//reset timer1
	tmr1_init(VU_PS, VU_TICKS);				//initialize timer1
	tmr1_act(vu_update);					//install user handler
}


//convert 8-bit adc to max led position
uint8_t vu_adc2val(uint8_t adc) {
	uint8_t i=0;
	
	for (i=0; i<24; i++) if (adc < vu_tab[i]) return i;
	return 23;
}

//convert 8-bit val to hold led position
//institute decay
uint8_t vu_val2peak(uint8_t val) {
	static uint8_t val_peak0=0;				//previous peak. 0=blank, 1=led1, 2=led2, ...
	
	if (val >= val_peak0) val_peak0 = val;	//update val_peak
	else val_peak0 = (val_peak0>=VU_DEC)?(val_peak0-VU_DEC):0;		//otherwise decrement val (unless it is already 0
	return val_peak0;
}

//vu_val low-pass filter
uint8_t vu_filter(uint8_t val) {
	static uint16_t vu_sum=0;
	
	vu_sum += (val - (vu_sum >> 0));		//low pass filter. 0->no filtering, N<8->progressively more filtering
	return vu_sum >> 0;
}
	
	
int main(void) {
	uint16_t tmp;							//temp variable
	
	mcu_init();							    //initialize the mcu
	vu_init();								//reest the vu meter
	ei();									//enable interrupts
	while (1) {
		vu_input = adc_read(VU_AN)>>2;		//read the adc channel, msb 8-bit only
		//using random numbers to test the program
		vu_input = vu_filter (rand() & 0x00ff);	//implement a low-pass filter
		//convert adc value into led positions for the bar
		vu_val = vu_adc2val(vu_input); 		//find the led position
		vu_peak= vu_val2peak(vu_val);		//find the peak
		
		delay_ms(VU_DLY);					//waste some time
	}
}
