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
#include "util.h"

int intr_num = 0;
int counting = 0;

int main(void){
  //uint8_t x, y, z = 0;
  timer_init();
  lcd_init();
  
  
  // print some text
  lcd_set_cursor(0, 0);
  lcd_putstr("Hello World from AVR!");
  lcd_set_cursor(2, 0);
  lcd_putstr("QAQ");
  //lcd_putstr("_");
  /*lcd_setbit(0, 0, 1);
  lcd_setbit(0, 1, 1);
  lcd_setbit(0, 2, 1);
  lcd_setbit(0, 3, 1);
  lcd_flush();*/
  
  
  while(1) {
    // draw an animated barberpole on the left side of the screen
    /*for(x = 0; x < 64; ++x) {
      for(y = 0; y < 64; ++y) {
        if ((y+x+z) % 8 < 4) {
          lcd_setbit(x, y, 1);
        } else {
          lcd_setbit(x, y, 0);
        }
      }
    }
    if (++z == 8) z = 0;*/
	
	
	
		
	
  }
  
  return 0;
}


ISR (TIMER0_OVF_vect)
{
	char z[64];
	
	if(++intr_num >= 3906){
		intr_num=0;
		counting++;
		lcd_set_cursor(5, 0);
		int2str(counting, z);
		lcd_putstr(z);
	}
}