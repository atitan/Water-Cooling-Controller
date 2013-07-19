/*
 * timer.c
 *
 * Created: 2013/6/28 上午 11:08:05
 *  Author: ATI
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include "control.h"
#include "lcd.h"
#include "adc.h"
#include "ntctemp.h"
#include "util.h"

// file scope variables
static int is_button_mode = 0;
static float EEMEM ee_critial_point = 20.0;
static float temp = 0.0;
static float temp_old = 0.0;
static int volt_level = 0;

void timer_init ()
{
	DDRD |= 0x40; //Set PD6 as output
	TCCR0A |= (1 << WGM00) | (1 << COM0A1); // Configure timer0 for phase-correct PWM on OC0A(PD6) pin
	TCCR0B |= (1 << CS01 ); //Set prescaler to 8
	OCR0A = 5; // Set init compare value
	TIMSK0 |= (1 << TOIE0 ); // Enable counter overflow interrupt
}

void button_init()
{
	DDRC |= 0x00;
	PORTC |= 0x0E;
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
	if(++check_counter > 10){
		temp_old = temp;
	}
	temp = ntctemp_getLookup(adc);
	
	// Show info
	show_info();
	
	// Call comparator after 5 checks
	if(check_counter > 10)
	{
		check_counter = 0;
		temp_comparator();
	}
}

void temp_comparator()
{
	static int indicator = 0;
	float critial_point = eeprom_read_float( &ee_critial_point );
	
	if (temp - critial_point > 0)
	{
		if (temp >= temp_old - 0.2)
		{
			adjust_volt(volt_level + 1);
		}
		
		indicator = 1;
	} 
	else
	{
		if (indicator == 1)
		{
			adjust_volt( (int)(volt_level / 2) );
		}
		else
		{
			if (temp <= temp_old + 0.2)
			{
				adjust_volt(volt_level - 1);
			}
		}
		
		indicator = 0;
	}
	
}

void adjust_volt (int value)
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
	
	if (value >= 0 && value < 10)
	{
		volt_level = value;
		OCR0A = volt_table[volt_level];
	}
	
}

void show_info()
{
	// Avoid button mode
	if (is_button_mode == 0)
	{
		char printbuff[25];
		char printbuff2[25];
		float critial_point = eeprom_read_float( &ee_critial_point );
		
		// Clear lcd first
		lcd_clear();
		
		// Show banner
		lcd_set_cursor(0, 0);
		lcd_putstr("== Status ==");
		
		// Show temperature
		dub2str(temp, printbuff);
		strcpy (printbuff2, "Current temp: ");
		strcat (printbuff2, printbuff);
		lcd_set_cursor(1, 0);
		lcd_putstr(printbuff2);
		
		// Show critical point
		dub2str(critial_point, printbuff);
		strcpy (printbuff2, "Critical point: ");
		strcat (printbuff2, printbuff);
		lcd_set_cursor(3, 0);
		lcd_putstr(printbuff2);
		
		// Show voltage level
		int2str(volt_level, printbuff);
		strcpy (printbuff2, "Voltage level: ");
		strcat (printbuff2, printbuff);
		lcd_set_cursor(5, 0);
		lcd_putstr(printbuff2);
	}
}

void set_critical_temp ()
{
	char printbuff[25];
	char printbuff2[25];
	float setup_point = eeprom_read_float( &ee_critial_point );
	int button_counter = 0;
	int long_press = 0;
	
	// initial button mode
	is_button_mode = 1;
	
	// Setup
	Setup:
	lcd_clear();
	lcd_set_cursor(0, 0);
	lcd_putstr("==criti-temp setup==");
	dub2str(setup_point, printbuff);
	strcpy (printbuff2, "Criti-temp: ");
	strcat (printbuff2, printbuff);
	lcd_set_cursor(1, 0);
	lcd_putstr(printbuff2);
	lcd_set_cursor(3, 0);
	lcd_putstr("press 1 to decrease");
	lcd_set_cursor(4, 0);
	lcd_putstr("press 2 to increase");
	lcd_set_cursor(5, 0);
	lcd_putstr("press 3 to confirm");
	while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
	while (1)
	{
		// Show current value
		dub2str(setup_point, printbuff);
		strcpy (printbuff2, "Criti-temp: ");
		strcat (printbuff2, printbuff);
		lcd_set_cursor(1, 0);
		lcd_putstr(printbuff2);
		
		if (((PINC >> 1) & 0x01) == 0) // decrement
		{
			button_counter++;
			if (button_counter > 20)
			{
				button_counter = 0;
				if (long_press++ > 3)
				{
					setup_point -= 0.5;
				} 
				else
				{
					setup_point -= 0.1;
				}
				
				if (setup_point < 0)
				{
					setup_point = 0;
				}
			}
		}
		else if (((PINC >> 2) & 0x01) == 0) // increment
		{
			button_counter++;
			if (button_counter > 20)
			{
				button_counter = 0;
				if (long_press++ > 3)
				{
					setup_point += 0.5;
				}
				else
				{
					setup_point += 0.1;
				}
				
				if (setup_point > 50)
				{
					setup_point = 50;
				}
			}
		}
		else if (((PINC >> 3) & 0x01) == 0) // confirmation
		{
			button_counter++;
			if (button_counter > 20)
			{
				button_counter = 0;
				break;
			}
		}
		else
		{
			button_counter = 0;
			long_press = 0;
		}
	}
	
	// Confirmation
	lcd_clear();
	lcd_set_cursor(0, 0);
	lcd_putstr("==confirmation==");
	dub2str(setup_point, printbuff);
	strcpy (printbuff2, "Criti-temp is ");
	strcat (printbuff2, printbuff);
	lcd_set_cursor(1, 0);
	lcd_putstr(printbuff2);
	lcd_set_cursor(3, 0);
	lcd_putstr("press 2 to edit");
	lcd_set_cursor(4, 0);
	lcd_putstr("press 3 to save");
	while ( ((PINC >> 3) & 0x01) == 0 ){} // prevent button being pressed before entering setup
	while (1)
	{
		if (((PINC >> 2) & 0x01) == 0) // edit
		{
			button_counter++;
			if (button_counter > 20)
			{
				button_counter = 0;
				goto Setup;
			}
			continue;
		}
		else if (((PINC >> 3) & 0x01) == 0) // save
		{
			button_counter++;
			if (button_counter > 20)
			{
				button_counter = 0;
				break;
			}
			continue;
		}
		else
		{
			button_counter = 0;
		}
	}
	
	eeprom_update_float( &ee_critial_point, setup_point ); // update eeprom
	
	// exit button mode
	is_button_mode = 0;
	show_info();
	while ( ((PINC >> 3) & 0x01) == 0 ){} // prevent button being pressed before entering main
}