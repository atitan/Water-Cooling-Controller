/*
 * water_cooling_controller.c
 *
 * Created: 2013/6/28 上午 11:05:00
 *  Author: ATI
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "control.h"
#include "lcd.h"
#include "adc.h"
#include "util.h"

static uint16_t EEMEM is_eeprom_inited = 0;

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
	
	// variables
	int button_detector = 0;
	
	// force setting critical point
	if (eeprom_read_word( &is_eeprom_inited  ) == 0)
	{
		eeprom_update_word ( &is_eeprom_inited , 1 );
		set_critical_temp();
	}
	
	sei(); //Enable global interrupt
	
	while (1) 
	{
		// Reserved for button control
	}
  
	return 0;
}


ISR (TIMER0_OVF_vect)
{
	static int intr_num = 0;
	static int counting = 0;
	char z[64];
	
	if(++intr_num >= 3906)
	{
		intr_num = 0;
		counting++;
		int2str(counting, z);
		lcd_set_cursor(4, 0);
		lcd_putstr(z);
	}

	// check_temp();
	// show_info();
}