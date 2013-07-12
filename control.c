/*
 * timer.c
 *
 * Created: 2013/6/28 上午 11:08:05
 *  Author: ATI
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "control.h"
#include "lcd.h"
#include "adc.h"
#include "ntctemp.h"
#include "util.h"

// file scope variables
static int is_button_mode = 0;
float EEMEM critial_point = 0;

void timer_init ()
{
	DDRD |= 0x40; //Set PD6 as output
	TCCR0A |= (1 << WGM00) | (1 << COM0A1); // Configure timer0 for phase-correct PWM on OC0A(PD6) pin
	TCCR0B |= (1 << CS01 ); //Set prescaler to 8
	OCR0A = 5; // Set init compare value
	TIMSK0 |= (1 << TOIE0 ); // Enable counter overflow interrupt
	sei(); //Enable global interrupt
}

void check_temp ()
{
	static uint16_t adc = 0;
	static uint16_t adc_old = 0;
	double temp;
	char printbuff[10];
	
	// Get temperature
	adc_old = adc;
	adc = adc_read(0);
	adc = adc_emafilter(adc, adc_old);
	temp = ntctemp_getLookup(adc);
	
	// Show temperature on lcd
	if (is_button_mode == 0)
	{
		dub2str(temp, printbuff);
		lcd_set_cursor(6, 0);
		lcd_putstr(printbuff);
	}
	
	// Pass tempearture data to comparator
	temp_comparator(temp);
}

void temp_comparator (float temp)
{
	static float temp_old = 0;
}

void adjust_volt ()
{
	
}

void set_critical_temp ()
{
	// initial button mode
	is_button_mode = 1;
	lcd_clear();
	
	//
	
	
	// return
	is_button_mode = 0;
}