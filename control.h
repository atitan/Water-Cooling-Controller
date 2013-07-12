/*
 * timer.h
 *
 * Created: 2013/6/28 上午 11:11:03
 *  Author: ATI
 */ 

// function prototypes
extern void timer_init ();
extern void check_temp ();
extern void temp_comparator (float);
extern void adjust_volt ();
extern void set_critical_temp ();