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
static float temp = 20.0;
static float temp_old = 20.0;
static int volt_level = 0;
static uint8_t EEMEM volt_table [10] = {
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50
};

void timer_init ()
{
	DDRD |= 0x40; //Set PD6 as output
	TCCR0A |= (1 << WGM00) | (1 << COM0A1); // Configure timer0 for phase-correct PWM on OC0A(PD6) pin
	TCCR0B |= (1 << CS01 ); //Set prescaler to 8
	OCR0A = eeprom_read_byte (& volt_table[0] ); // Set init compare value
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
	
	if (is_button_mode == 1)
	{
		return;
	}
	
	// Get temperature
	adc_old = adc;
	adc = adc_read(0);
	adc = adc_emafilter(adc, adc_old);
	if(++check_counter > 5){
		temp_old = temp;
	}
	temp = ntctemp_getLookup(adc);
	
	if (temp > 10000 || temp < -10000)
	{
		temp = 99.99;
	}
	
	// Show info
	show_info();
	
	// Call comparator after 5 checks
	if(check_counter > 5)
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
	uint8_t table[10];
	eeprom_read_block (( void *) table , ( const void *) volt_table , 10);
	
	if (value >= 0 && value < 10)
	{
		volt_level = value;
		OCR0A = table[volt_level];
	}
	
}

void show_info()
{
	// Avoid button mode
	if (is_button_mode != 1)
	{
		char printbuff[25];
		char printbuff2[25];
		float critial_point = eeprom_read_float( &ee_critial_point );
		
		// Clear lcd if exited from button mode
		if (is_button_mode == 2)
		{
			is_button_mode = 0;
			lcd_clear();
		}
		
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
		
		int2str(OCR0A, printbuff);
		if (OCR0A < 10)
		{
			strcat (printbuff, "  ");
		}
		else if (OCR0A < 100 && OCR0A >= 10)
		{
			strcat (printbuff, " ");
		}
		
		strcpy (printbuff2, "PWM: ");
		strcat (printbuff2, printbuff);
		lcd_set_cursor(6, 0);
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
	while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
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
	while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
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
	is_button_mode = 2;
	while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering main
}

void set_volt_table ()
{
		uint8_t table[10];
		eeprom_read_block (( void *) table , ( const void *) volt_table , 10);
		char printbuff[25];
		char printbuff2[25];
		char printbuff3[25];
		int button_counter = 0;
		int long_press = 0;
		int ctr = 0;
		
		// initial button mode
		is_button_mode = 1;
		
		Volt_Set:
		// Setup
		lcd_clear();
		lcd_set_cursor(0, 0);
		lcd_putstr("==volt table setup==");
		lcd_set_cursor(3, 0);
		lcd_putstr("Do not set too high!");
		lcd_set_cursor(5, 0);
		lcd_putstr("press 1 to decrease");
		lcd_set_cursor(6, 0);
		lcd_putstr("press 2 to increase");
		lcd_set_cursor(7, 0);
		lcd_putstr("press 3 to confirm");
		
		while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
		
		// loop
		for (ctr = 0; ctr < 10; ctr++)
		{
			while (1)
			{
				/* Display Start*/
				int2str(table[ctr], printbuff);
				if (table[ctr] < 10)
				{
					strcpy (printbuff2, printbuff);
					strcat (printbuff2, "  ");
				} 
				else if (table[ctr] < 100 && table[ctr] >= 10)
				{
					strcpy (printbuff2, printbuff);
					strcat (printbuff2, " ");
				}
				else
				{
					strcpy (printbuff2, printbuff);
				}
			
				// level label
				int2str(ctr, printbuff);
				strcpy (printbuff3, "level ");
				strcat (printbuff3, printbuff);
				strcat (printbuff3, ": ");
				
				// cat these two string
				strcpy (printbuff, printbuff3);
				strcat (printbuff, printbuff2);
				
				// display
				lcd_set_cursor(1, 0);
				lcd_putstr(printbuff);
				/* Display End*/
				
				/* Button Start*/
				if (((PINC >> 1) & 0x01) == 0) // decrement
				{
					button_counter++;
					if (button_counter > 20)
					{
						button_counter = 0;
							
						if (table[ctr] > 0)
						{
							if (long_press++ > 3 && table[ctr] > 3)
							{
								table[ctr] -= 3;
							}
							else
							{
								table[ctr]--;
							}
							OCR0A = table[ctr]; //Synchronize
						}
					}
				}
				else if (((PINC >> 2) & 0x01) == 0) // increment
				{
					button_counter++;
					if (button_counter > 20)
					{
						button_counter = 0;
					 
						if (table[ctr] < 255)
						{
							if (long_press++ > 3 && table[ctr] < 253)
							{
								table[ctr] += 3;
							}
							else
							{
								table[ctr]++;
							}
							OCR0A = table[ctr]; //Synchronize
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
				/* Button End */
			} //End while
			while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
		} // End loop
		
		// Confirmation
		lcd_clear();
		
		lcd_set_cursor(0, 0);
		lcd_putstr("==confirmation==");
		
		lcd_set_cursor(1, 0);
		int2str(table[0], printbuff);
		strcpy (printbuff3, "level 0: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(2, 0);
		int2str(table[1], printbuff);
		strcpy (printbuff3, "level 1: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(3, 0);
		int2str(table[2], printbuff);
		strcpy (printbuff3, "level 2: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(4, 0);
		int2str(table[3], printbuff);
		strcpy (printbuff3, "level 3: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(5, 0);
		int2str(table[4], printbuff);
		strcpy (printbuff3, "level 4: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(7, 0);
		lcd_putstr("press 1 to continue");
		
		while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
			
		while (1)
		{
			if (((PINC >> 1) & 0x01) == 0) // confirmation
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
			}
		}
		
		lcd_clear();
		
		lcd_set_cursor(0, 0);
		lcd_putstr("==confirmation==");
		
		lcd_set_cursor(1, 0);
		int2str(table[5], printbuff);
		strcpy (printbuff3, "level 5: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(2, 0);
		int2str(table[6], printbuff);
		strcpy (printbuff3, "level 6: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(3, 0);
		int2str(table[7], printbuff);
		strcpy (printbuff3, "level 7: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(4, 0);
		int2str(table[8], printbuff);
		strcpy (printbuff3, "level 8: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(5, 0);
		int2str(table[9], printbuff);
		strcpy (printbuff3, "level 9: ");
		strcat (printbuff3, printbuff);
		lcd_putstr(printbuff3);
		
		lcd_set_cursor(6, 0);
		lcd_putstr("press 2 to edit");
		lcd_set_cursor(7, 0);
		lcd_putstr("press 3 to confirm");
		
		while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering setup
		while (1)
		{
			if (((PINC >> 2) & 0x01) == 0) // edit
			{
				button_counter++;
				if (button_counter > 20)
				{
					button_counter = 0;
					goto Volt_Set;
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
		
		eeprom_update_block ( (const void *) table , (void *) volt_table , 10); // update eeprom
		OCR0A = table[0];
		
		// exit button mode
		is_button_mode = 2;
		while ( ( ((PINC >> 3) & 0x01) & ((PINC >> 2) & 0x01) & ((PINC >> 1) & 0x01) )  == 0 ){} // prevent button being pressed before entering main
}