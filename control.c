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
static float EEMEM critial_point = 0;
static float temp = 0.0;
static int volt_level = 0;

void timer_init ()
{
	DDRD |= 0x40; //Set PD6 as output
	TCCR0A |= (1 << WGM00) | (1 << COM0A1); // Configure timer0 for phase-correct PWM on OC0A(PD6) pin
	TCCR0B |= (1 << CS01 ); //Set prescaler to 8
	OCR0A = 0; // Set init compare value
	TIMSK0 |= (1 << TOIE0 ); // Enable counter overflow interrupt
}

void check_temp ()
{
	static uint16_t adc = 0;
	static uint16_t adc_old = 0;
	static int check_counter = 0;
	
	// Get temperature
	adc_old = adc;
	adc = adc_read(0);
	adc = adc_emafilter(adc, adc_old);
	temp = ntctemp_getLookup(adc);
	
	// Call comparator after 5 checks
	if(check_counter++ > 5)
	{
		check_counter = 0;
		temp_comparator();
	}
}

void temp_comparator()
{
	float critial_goal = eeprom_read_float( &critial_point );
	
	
}

void adjust_volt (int offset)
{
	const static int volt_table[10] = {
		0, // 0V
		5,
		10,
		15,
		20,
		25,
		30,
		35,
		40,
		255 // 12V
	};
	
	if (volt_level + offset >= 0 && volt_level + offset < 10)
	{
		volt_level += offset;
		OCR0A = volt_table[volt_level];
	}
	
}

void show_info()
{
	// Avoid button mode
	if (is_button_mode == 0)
	{
		char printbuff[25];
		
		// Show temperature
		dub2str(temp, printbuff);
		sprintf(printbuff, "%s %s", "Temp:", printbuff);
		lcd_set_cursor(6, 0);
		lcd_putstr(printbuff);
		
		// Show voltage level
		int2str(volt_level, printbuff);
		sprintf(printbuff, "%s %s", "Volt level:", printbuff);
		lcd_set_cursor(7, 0);
		lcd_putstr(printbuff);
	}
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