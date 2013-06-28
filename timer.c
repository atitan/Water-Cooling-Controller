﻿/*
 * timer.c
 *
 * Created: 2013/6/28 上午 11:08:05
 *  Author: ATI
 */ 

#include "timer.h"

void timer_init()
{
	DDRD |= 0x40;
	TCCR0A |= (1 << WGM00)|(1 << COM0A1); // Configure timer0 for phase-correct PWM on OC0A pin
	TCCR0B |= (1 << CS01 ); //Set prescaler to 8
	OCR0A = 150; // Set init compare value
	TIMSK0 |= (1 << TOIE0 ); // Enable counter overflow interrupt
	sei(); //Enable global interrupt
}
