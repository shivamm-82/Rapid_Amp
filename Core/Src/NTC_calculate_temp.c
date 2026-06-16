/*
 * NTC_calculate_temp.c
 *
 *  Created on: May 19, 2026
 *      Author: Shivam.Maurya
 */
#include <math.h>
#include <stdint.h>

float Calculate_temprature(uint32_t ADC_val){
    //temprature conversion
	       float voltage = ADC_val * 3.3f / 4095.0f;

	       float R_FIXED = 5100.0f;


	    float   r_ntc =
	       (5100.0f * voltage) /
	       (3.3f - voltage);

	     float  temperature =
	       1.0f /
	       (
	           (1.0f / 298.15f) +
	           (1.0f / 3800.0f) *
	           log(r_ntc / 10000.0f)
	       );
	       temperature -= 275.15f;

//	       float ntcResistance =
//	           (voltage * R_FIXED) / (3.3f - voltage);
//
//	       float BETA = 24700.0f;
//	       float R0 = 10000.0f;
//	       float T0 = 298.15f;
//
//	       float tempK = 1.0f /
//	       (
//	           (1.0f / T0) +
//	           (1.0f / BETA) *
//	           log(ntcResistance / R0)
//	       );
//
//	      float  tempC = tempK - 273.15f;

	      return temperature;
}
