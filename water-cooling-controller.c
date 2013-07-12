/*
 * water_cooling_controller.c
 *
 * Created: 2013/6/28 上午 11:05:00
 *  Author: ATI
 */ 

#define F_CPU 8000000UL

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "control.h"
#include "lcd.h"
#include "adc.h"
#include "util.h"

int intr_num = 0;
int counting = 0;

int main(void){
	// initialize everything
	timer_init();
	lcd_init();
	adc_setchannel(0);
	adc_init();
  
	// set lcd text
	lcd_set_cursor(0, 0);
	lcd_putstr("Hello World from AVR!");
	lcd_set_cursor(2, 0);
	lcd_putstr("QAQ");
  
	while(1) 
	{
    
	
	}
  
	return 0;
}


ISR (TIMER0_OVF_vect)
{
	char z[64];
	
	if(++intr_num >= 3906){
		intr_num=0;
		counting++;
		lcd_set_cursor(4, 0);
		int2str(counting, z);
		lcd_putstr(z);
	}
	check_temp();
}