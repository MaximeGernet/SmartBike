#ifndef _IO
#define _IO


#define CS 4		// SPI slave select pin
#define LED1 29		// PA5
#define LED2 28		// PA4
#define B1 12		// PD4, pull up
#define B2 30		// PA6, pull down
#define BAT_PIN A0 	// Battery voltage, à changer


void initIO();

void getBatVoltage(float* bat_lvl);


#endif
