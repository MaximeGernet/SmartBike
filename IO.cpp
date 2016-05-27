#include <Arduino.h>

#include "IO.h"

void initIO()
{
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);

	pinMode(B1, INPUT);
	pinMode(B2, INPUT_PULLUP);

	pinMode(A0, INPUT);
	analogReference(DEFAULT);
}

void getBatVoltage(float* bat_lvl)
{
	uint16_t bat_adc;

	bat_adc = analogRead(BAT_PIN);
	//*bat_lvl = (float)bat_adc*5.0/1023;	// à vérifier
	*bat_lvl = (float)bat_adc*3.3*2.0/1023;	// à vérifier
	//*bat_lvl = 44.22;
}
