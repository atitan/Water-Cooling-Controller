﻿/*
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
	button_init();
	lcd_init();
	adc_setchannel(0);
	adc_init();
	
	// variables
	int button_detector = 0;
	
	// force setting critical point
	if (eeprom_read_word( &is_eeprom_inited  ) == 0)
	{
		eeprom_update_word ( &is_eeprom_inited , 1 );
		set_critical_temp();
	}
	
	sei(); //Enable global interrupt
	check_temp(); //Check temp
	
	while (1) 
	{
			if (((PINC >> 3) & 0x01) == 0)
			{
				button_detector++;

				if (button_detector>50)
				{
					button_detector = 0;
					set_critical_temp();
				}
			}
			else
			{
				button_detector = 0;
			}
	}
  
	return 0;
}


ISR (TIMER0_OVF_vect)
{
	static int intr_num = 0;
	
	if(++intr_num >= 4000)
	{
		intr_num = 0;
		check_temp();
	}
}