//pic24f project with 3wi-lcd module

#include <string.h>                     //we use strcpy
#include "config.h"					//configuration words - for C30. Use config.h for XC16
#include "gpio.h"
#include "delay.h"						//we use software delays
#include "lcd_3wi.h"                    //we use lcd_3wi module
#include "adc.h"						//we use adc

//hardware configuration
//#define LED_PORT			GPIO
//#define LED_DDR				TRISIO
//#define LED					(1<<5)

#define VU_INR				ADC_0		//analog input, for channel R
#define VU_INL				ADC_1		//analog input, for channel L
#define VU_CHAR				VU_BAR		//black bar char
#define VU_DASH				'-'			//'-'
#define VU_BAR				0xff		//black bar char
//end hardware configuration

//global defines

//global variables
uint8_t vRAM[17];                       //lcd display buffer
uint8_t *vu_tab;						//vu_table pointer
const uint8_t str0[]="VU LCD16x2C v0.1";
const uint8_t strR[]="R:              ";
const uint8_t strL[]="L:              ";

//vu 14-table - linear
const uint8_t vu_lin[]={
	  9,  27,  45,  64,  82, 100, 118, 137,
	155, 173, 192, 210, 228, 246

};

const uint8_t vu_log[]={
	  1,   2,   3,   4,   7,  10,  15,  23,
	 34,  51,  76, 113, 168, 250
};

//convert adc value to bar in str
uint8_t vu_adc2val(uint8_t adc) {
	uint8_t i=0;

	for (i=0; i<14; i++) if (adc < vu_tab[i]) return i;
	return 14;
}

void vu_update(char * str, uint8_t val) {
	uint8_t i;

	for (i=0; i<val; i++) str[2+i]=VU_CHAR; 		//fill up the display buffer
}
//initialize vu meter
void vu_init(void) {
	vu_tab = vu_log;					//default to log table
	adc_init();							//reset the adc module
	lcd_init();							//initialize the lcd
}

int main(void) {
    uint32_t cnt=0, tmp;
    uint8_t val_L, val_R;				//val for r/l channels

	mcu_init();							//reset the mcu
	//IO_OUT(LED_DDR, LED);				//led as output
	vu_init();                         	//update the vu meter
	strcpy(vRAM, str0); lcd_display(LCD_Line0, vRAM);   //display something
	delay_ms(500);						//wait for the display to show
	while (1) {
		//Right channel
        //cnt = adc_read(VU_INR) >> 2;	//read the adc, 8-bit
        cnt = rand() & 0x00ff;			//for debugging only
        val_R=vu_adc2val(cnt);          //display cnt
        strcpy(vRAM, strR);             //format the display string
        vu_update(vRAM, val_R);			//update vRAM
        lcd_display(LCD_Line0, vRAM);   //display vram

		//left channel
        //cnt = adc_read(VU_INL) >> 2;	//read the adc, 8-bit only
        cnt = rand() & 0x00ff;			//for debugging only
        val_L=vu_adc2val(cnt);          //display cnt
        strcpy(vRAM, strL);             //format the display string
        vu_update(vRAM, val_L);			//update vRAM
        lcd_display(LCD_Line1, vRAM);   //display vram

        //blink the led
        //IO_FLP(LED_PORT, LED);
        //delay_ms(1);
	}
}
