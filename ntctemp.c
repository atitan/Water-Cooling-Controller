/*
ntctemp 0x02

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include "ntctemp.h"

#if NTCTEMP_B == 1 ||  NTCTEMP_SH == 1
#include <math.h>
#endif

#if NTCTEMP_LKP == 1
#include <avr/pgmspace.h>
#endif

//define lookup method variables
#if NTCTEMP_LKP == 1
#define ntctemp_lookupadcfirst 225 //adc first value of lookup table
#define ntctemp_lookupadcstep 5 //step between every table point
#define ntctemp_lookuptablesize 103 //size of the lookup table
const float PROGMEM ntctemp_lookuptable[ntctemp_lookuptablesize] = {
	-1.27,  -0.73,  -0.2,  0.32,  0.84,  1.35,  1.86,  2.36,  2.86,  3.35,  3.84,  4.33,  4.81,  5.29,  5.76,  6.24,  6.7,  7.17,  7.63,  8.1,  8.55,  9.01,  9.46,  9.92,  10.37,  10.81,  11.26,  11.71,  12.15,  12.59,  13.03,  13.47,  13.91,  14.35,  14.79,  15.22,  15.66,  16.1,  16.53,  16.96,  17.4,  17.83,  18.27,  18.7,  19.13,  19.57,  20,  20.44,  20.87,  21.31,  21.75,  22.18,  22.62,  23.06,  23.5,  23.94,  24.38,  24.82,  25.27,  25.71,  26.16,  26.61,  27.06,  27.51,  27.96,  28.42,  28.88,  29.34,  29.8,  30.26,  30.73,  31.2,  31.67,  32.15,  32.62,  33.11,  33.59,  34.08,  34.57,  35.06,  35.56,  36.06,  36.57,  37.08,  37.6,  38.12,  38.64,  39.17,  39.7,  40.24,  40.79,  41.34,  41.89,  42.46,  43.03,  43.6,  44.18,  44.77,  45.37,  45.98,  46.59,  47.21,  47.84
};
#endif

/*
 * get temperature using Beta Model Equation
 *
 * "adcresistance" adc resistence read
 * "beta" beta value
 * "adctref" temperature reference for the measuread value
 * "adcrref" resistance reference for the measured value
 */
#if NTCTEMP_B == 1
float ntctemp_getB(long adcresistence, int beta, float adctref, int adcrref) {
	// use the Beta Model Equation
	// temperature (kelvin) = beta / ( beta / tref + ln ( R / rref ) )
	float t;
	t = beta / ( beta / (float)(adctref + 273.15) + log ( adcresistence / (float)adcrref ) );
	t = t - 273.15; // convert Kelvin to Celcius
	//t = (t * 9.0) / 5.0 + 32.0; // convert Celcius to Fahrenheit
	return t;
}
#endif

/*
 * get temperature using the Steinhart-Hart Thermistor Equation
 *
 * "adcresistance" adc resistence read
 * "A", "B", "C" equation parameters
 */
#if NTCTEMP_SH == 1
float ntctemp_getSH(long adcresistence, float A, float B, float C) {
	// use the Steinhart-Hart Thermistor Equation
	// temperature (Kelvin) = 1 / (A + B*ln(R) + C*(ln(R)^3))
	float t;
	t = log( adcresistence );
	t = 1 / (A + (B * t) + (C * t * t * t));
	t = t - 273.15; // convert Kelvin to Celcius
	//t = (t * 9.0) / 5.0 + 32.0; // convert Celcius to Fahrenheit
	return t;
}
#endif

/*
 * get temperature using a lookup table
 */
#if NTCTEMP_LKP == 1
float ntctemp_getLookup(uint16_t adcvalue) {
	float t = 0.0;
	float mint = 0;
	float maxt = 0;

	//return error for invalid adcvalues
	if(adcvalue<ntctemp_lookupadcfirst || adcvalue>ntctemp_lookupadcfirst+ntctemp_lookupadcstep*(ntctemp_lookuptablesize-1)) {
			return NTCTEMP_LOOKUPRETERROR;
	}

	uint8_t i = 0;
	uint16_t a = ntctemp_lookupadcfirst;
	for(i=0; i<ntctemp_lookuptablesize; i++) {
		if(adcvalue < a)
			break;
		a += ntctemp_lookupadcstep;
	}

	maxt = pgm_read_float(&ntctemp_lookuptable[i]); //highest interval value
	if(i==0)
		mint = maxt;
	else
		mint = pgm_read_float(&ntctemp_lookuptable[i-1]); //smallest interval value

	//do interpolation
	a = a-ntctemp_lookupadcstep;
	t = mint + ((maxt-mint)/ntctemp_lookupadcstep) * (adcvalue-a);

	return t;
}
#endif


