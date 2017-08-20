//pic24f project with 3wi-lcd module

#include <string.h>                     //we use strcpy
#include "config_c30.h"					//configuration words - for C30. Use config.h for XC16
#include "gpio.h"
#include "delay.h"						//we use software delays
#include "lcd_3wi.h"                    //we use lcd_3wi module
#include "adc.h"						//we use adc

//hardware configuration
#define LED_PORT			PORTB
#define LED_DDR				TRISB
#define LED					(1<<5)

#define VU_INR				ADC_AN0		//analog input, for channel R
#define VU_INL				ADC_AN1		//analog input, for channel L
#define VU_CHAR				VU_BAR		//black bar char
#define VU_DASH				'-'			//'-'
#define VU_BAR				0xff		//black bar char
//end hardware configuration

//global defines

//global variables
uint8_t vRAM[17];                       //lcd display buffer
uint8_t *vu_tab;						//vu_table pointer
const uint8_t str0[]="VU LCD16x2G v0.1";
const uint8_t strR[]="R:              ";
const uint8_t strL[]="L:              ";
const uint8_t bars[][8]={
	( 1<<4), ( 1<<4), ( 1<<4), ( 1<<4), ( 1<<4), ( 1<<4), ( 1<<4), ( 1<<4),		//left most bar = 1
	( 3<<3), ( 3<<3), ( 3<<3), ( 3<<3), ( 3<<3), ( 3<<3), ( 3<<3), ( 3<<3),	//2nd to the left most bar = 1
	( 7<<2), ( 7<<2), ( 7<<2), ( 7<<2), ( 7<<2), ( 7<<2), ( 7<<2), ( 7<<2),		//middle bar = 1
	(15<<1), (15<<1), (15<<1), (15<<1), (15<<1), (15<<1), (15<<1), (15<<1),		//2nd to the right most bar = 1
	(31<<0), (31<<0), (31<<0), (31<<0), (31<<0), (31<<0), (31<<0), (31<<0),	//right most bar = 1
};

//vu 14-table - linear
const uint8_t vu_lin[]={
	1, 		5, 		9, 		12, 	16, 	20, 	23, 	27, 	31, 	34,
	38, 	42, 	45, 	49, 	53, 	56, 	60, 	64, 	67, 	71,
	74,		78,		82,		85,		89,		93,		96,		100,	104,	107,
	111,	115,	118,	122,	126,	129,	133,	137,	140,	144,
	148,	151,	155,	159,	162,	166,	170,	173,	177,	181,
	184,	188,	192,	195,	199,	202,	206,	210,	213,	217,
	221,	224,	228,	232,	235,	239,	243,	246,	250,	255
};

const uint8_t vu_log[]={
	1,		2,		3,		4,		5,		6,		7,		8,		9,		10,
	11,		12,		13,		14,		15,		16,		17,		18,		19,		20,
	21,		22,		23,		24,		25,		26,		27,		28,		29,		30,
	31,		32,		33,		34,		35,		36,		37,		38,		39,		40,
	41,		42,		43,		44,		45,		46,		47,		48,		49,		51,
	55,		60,		65,		70,		76,		82,		89,		97,		104,	113,
	122,	133,	143,	155,	168,	182,	197,	213,	231,	250
};

//convert adc value to bar in str
uint8_t vu_adc2val(uint8_t adc) {
	uint8_t i=0;

	for (i=0; i<70; i++) if (adc < vu_tab[i]) return i;
	return 70;
}

void vu_update(char * str, uint8_t val) {
	uint8_t i;

#if 0								//less efficient but more obvious
	for (i=0; i<val/5; i++) str[2+i]=VU_CHAR; 				//fill up the display buffer
	//now val is 0..5
	str[2+i]=val % 5;
#else								//faster but less obvious
	for (i=0; val >= 5; i+=1) {str[2+i]=VU_CHAR; val-=5;}
	str[2+i]=val;
#endif
}
//initialize vu meter
void vu_init(void) {
	vu_tab = vu_log;					//default to log table
	adc_init();							//reset the adc module
	lcd_init();							//initialize the lcd
	//define bars in cgram
	lcd_cgram(1, bars[0]);					//load cgram
	lcd_cgram(2, bars[1]);					//load cgram
	lcd_cgram(3, bars[2]);					//load cgram
	lcd_cgram(4, bars[3]);					//load cgram
	lcd_cgram(5, bars[4]);					//load cgram
}

int main(void) {
    uint32_t cnt=0, tmp;
    uint8_t val_L, val_R;				//val for r/l channels

	mcu_init();							//reset the mcu
	IO_OUT(LED_DDR, LED);				//led as output
	vu_init();                         	//update the vu meter
	strcpy(vRAM, str0); lcd_display(LCD_Line0, vRAM);   //display something
	delay_ms(500);						//wait for the display to show
	while (1) {
		//Right channel
        //cnt = adc_read(VU_INR) >> 2;	//read the adc, 8-bit
        cnt = rand() & 0x00ff;			//for debugging only
        //cnt = 1;
        val_R=vu_adc2val(cnt);          //display cnt
        strcpy(vRAM, strR);             //format the display string
        vu_update(vRAM, val_R);			//update vRAM
        lcd_display(LCD_Line0, vRAM);   //display vram

		//left channel
        //cnt = adc_read(VU_INL) >> 2;	//read the adc, 8-bit only
        cnt = rand() & 0x00ff;			//for debugging only
        //cnt = 8;
        val_L=vu_adc2val(cnt);          //display cnt
        strcpy(vRAM, strL);             //format the display string
        vu_update(vRAM, val_L);			//update vRAM
        lcd_display(LCD_Line1, vRAM);   //display vram

        //blink the led
        IO_FLP(LED_PORT, LED);
        delay_ms(100);
	}
}
