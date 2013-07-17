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
	-0.74,  -0.21,  0.31,  0.83,  1.34,  1.85,  2.35,  2.85,  3.35,  3.84,  4.32,  4.8,  5.28,  5.75,  6.22,  6.69,  7.16,  7.62,  8.08,  8.53,  8.99,  9.44,  9.89,  10.34,  10.78,  11.23,  11.67,  12.11,  12.55,  12.99,  13.43,  13.86,  14.3,  14.73,  15.16,  15.6,  16.03,  16.46,  16.89,  17.32,  17.75,  18.18,  18.61,  19.04,  19.47,  19.9,  20.33,  20.76,  21.19,  21.62,  22.05,  22.48,  22.92,  23.35,  23.78,  24.22,  24.66,  25.1,  25.54,  25.98,  26.42,  26.86,  27.31,  27.75,  28.2,  28.65,  29.11,  29.56,  30.02,  30.48,  30.94,  31.4,  31.87,  32.34,  32.81,  33.29,  33.77,  34.25,  34.74,  35.22,  35.72,  36.21,  36.71,  37.22,  37.73,  38.24,  38.76,  39.28,  39.81,  40.34,  40.88,  41.43,  41.98,  42.53,  43.09,  43.66,  44.24,  44.82,  45.41,  46.01,  46.61,  47.23,  47.85
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


