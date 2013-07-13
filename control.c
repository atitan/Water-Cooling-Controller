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
static float EEMEM ee_critial_point = 0;
static float temp = 0.0;
static int volt_level = 0;

void timer_init ()
{
	DDRD |= 0x40; //Set PD6 as output
	TCCR0A |= (1 << WGM00) | (1 << COM0A1); // Configure timer0 for phase-correct PWM on OC0A(PD6) pin
	TCCR0B |= (1 << CS01 ); //Set prescaler to 8
	OCR0A = 5; // Set init compare value
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
	static int ratio = 0;
	static int ratio_old = 0;
	float critial_point = eeprom_read_float( &ee_critial_point );
	
	if (temp - critial_point > 0)
	{
		ratio_old = ratio;
		ratio = (temp - critial_point) * 10 / critial_point;
		if (ratio - ratio_old > 5)
		{
			adjust_volt(1);
		}
	} 
	else
	{
		ratio_old = ratio;
		ratio = (critial_point - temp) * 10 / temp;
		if (ratio - ratio_old > 5)
		{
			adjust_volt(-1);
		}
	}
	
}

void adjust_volt (int offset)
{
	const static int volt_table[10] = {
		5, // 2V
		5, // 2V
		10, //
		15, //
		20, //
		25, //
		30, //
		35, //
		40, //
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
		float critial_point = eeprom_read_float( &ee_critial_point );
		
		// Clear lcd first
		lcd_clear();
		
		// Show temperature
		dub2str(temp, printbuff);
		sprintf(printbuff, "%s %s", "Curr-temp:", printbuff);
		lcd_set_cursor(5, 0);
		lcd_putstr(printbuff);
		
		// Show critical point
		dub2str(critial_point, printbuff);
		sprintf(printbuff, "%s %s", "Criti-point:", printbuff);
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
	
	// Press to continue
	
	
	// exit button mode
	is_button_mode = 0;
	show_info();
}