#include <Wire.h>
#include <Adafruit_GPS.h>

#include "sensors.h"

void initGyr()
{
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(GYRO_CONFIG);
	Wire.write(GYRO_FS_250); 
	Wire.endTransmission();
}

void initAcc()
{
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(ACCEL_CONFIG);
	Wire.write(ACCEL_FS_2); 
	Wire.endTransmission();	
}

void initMag()
{
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(BYPASS);
	Wire.write(0x02); 					// Pass-through mode ?
	Wire.endTransmission();

	Wire.beginTransmission(MAG_ADDR);
	Wire.write(CNTL1);
	Wire.write(0x06);					// Continuous measurement mode, 100 Hz
	Wire.endTransmission();
}

void initBar()
{
	// reset command
	do
	{
		Wire.beginTransmission(BAR_ADDR);
		Wire.write(0x1E);
	}while(Wire.endTransmission() != 0);

	// reading PROM
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xA0);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	prom.CRC = Wire.read();
	prom.factory = (((uint16_t)prom.CRC) << 12) | ((uint16_t)Wire.read());
	prom.CRC = prom.CRC >> 4;
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xA2);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	while(Wire.available() < 1)
	{
		delay(100);
		Wire.beginTransmission(BAR_ADDR);
		Wire.write(0xA2);
		Wire.endTransmission();
		Wire.requestFrom(BAR_ADDR, 2);
	}
	prom.C1 = (Wire.read() << 8) | Wire.read();
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xA4);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	prom.C2 = (Wire.read() << 8) | Wire.read();
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xA6);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	prom.C3 = (Wire.read() << 8) | Wire.read();
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xA8);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	prom.C4 = (Wire.read() << 8) | Wire.read();
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xAA);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	prom.C5 = (Wire.read() << 8) | Wire.read();
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0xAC);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 2);
	prom.C6 = (Wire.read() << 8) | Wire.read();
}

void initGPS()
{
	GPS.begin(9600);

	// uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	// uncomment this line to turn on only the "minimum recommended" data
	//GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);

	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);	// 1 Hz update rate

	GPS.sendCommand(PGCMD_ANTENNA);

	// Configuring interrupt
	OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
}

void initSensors()
{
	initGyr();

	initAcc();

	initMag();

	initBar();

	initGPS();
}



void getAccValues(ACC_s* acc)
{
	int16_t ax_raw, ay_raw, az_raw;

	// Reading raw values
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(ACCEL_XOUT_H);
	Wire.endTransmission();
	Wire.requestFrom(MPU_ADDR, 6);

	ax_raw = Wire.read() << 8;
	ax_raw += Wire.read();
	ay_raw = Wire.read() << 8;
	ay_raw += Wire.read();
	az_raw = Wire.read() << 8;
	az_raw += Wire.read();

	// Converting raw values
	acc->ax = (float)ax_raw * GRAV / ACC_SENS;
	acc->ay = (float)ay_raw * GRAV / ACC_SENS;
	acc->az = (float)az_raw * GRAV / ACC_SENS;

}

void getGyrValues(GYR_s* gyr)
{
	int16_t gx_raw, gy_raw, gz_raw;

	// Reading raw values
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(GYRO_XOUT_H);
	Wire.endTransmission();
	Wire.requestFrom(MPU_ADDR, 6);

	gx_raw = Wire.read() << 8;
	gx_raw += Wire.read();
	gy_raw = Wire.read() << 8;
	gy_raw += Wire.read();
	gz_raw = Wire.read() << 8;
	gz_raw += Wire.read();

	// Converting raw values
	gyr->gx = (float)gx_raw / GYR_SENS;
	gyr->gy = (float)gy_raw / GYR_SENS;
	gyr->gz = (float)gz_raw / GYR_SENS;
}

void getMagValues(MAG_s* mag)
{
	int16_t hx_raw, hy_raw, hz_raw;
	uint8_t st2;

	// Reading raw values
	Wire.beginTransmission(MAG_ADDR);
	Wire.write(HXL);
	Wire.endTransmission();
	Wire.requestFrom(MAG_ADDR, 6);

	while(Wire.available() < 6)
		;

	hx_raw = Wire.read();
	hx_raw += (Wire.read() << 8);
	hy_raw = Wire.read();
	hy_raw += (Wire.read() << 8);
	hz_raw = Wire.read();
	hz_raw += (Wire.read() << 8);

	Wire.beginTransmission(MAG_ADDR);
	Wire.write(ST2);
	Wire.endTransmission();
	Wire.requestFrom(MAG_ADDR, 1);

	st2 = Wire.read();

	// Converting raw values
	mag->hx = (float)hx_raw * MAG_SENS;
	mag->hy = (float)hy_raw * MAG_SENS;
	mag->hz = (float)hz_raw * MAG_SENS;
}

void getBarValues(BAR_s* bar)
{
	int32_t dT, T, P, ALT;
	int64_t OFF, SENS;
	uint32_t D1, D2;
	int i;
	uint32_t OFFH, OFFL;

	// Read raw values, OSR = 4096 (9 ms)
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(CONV_PRES_4096);
	Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0x00);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 3);

	D1 = 0;
	for(i = 0; i < 3; i++)
	{
		D1 = D1 << 8;
		D1 += Wire.read();
	}
	
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(CONV_TEMP_4096);
	Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(BAR_ADDR);
	Wire.write(0x00);
	Wire.endTransmission();
	Wire.requestFrom(BAR_ADDR, 3);
	
	D2 = 0;
	for(i = 0; i < 3; i++)
	{
		D2 = D2 << 8;
		D2 += Wire.read();
	}

	// Calculate compensated values
	dT = D2 - (((uint32_t)prom.C5) << 8);
	//T = 2000 + ((dT * (uint32_t)prom.C6) >> 23);
	T = (int32_t)(((int32_t)2000) + ((int32_t)(((int32_t)dT >> 4) * ((uint32_t)prom.C6)) >> 19));
	OFF = (((uint32_t)prom.C2) << 16) + (((int64_t)(prom.C4*dT)) >> 7);
	SENS = (((uint64_t)prom.C1) << 15) + ((((uint64_t)prom.C3)*dT) >> 8);
	P = (((((uint64_t)D1) * SENS) >> 21) - OFF) >> 15;

	// Calculate altitude
	ALT = 4433000 * (1 - pow((double)P/SEA_LVL_P, 0.190294957));

	// Formating data
	bar->pressure = (float)P/100;
	bar->temperature = (float)T/100;
	bar->altitude = (float)ALT/100;
}

void getGPSData(GPS_s* gps)
{
	if (GPS.newNMEAreceived())
	{
		if (!GPS.parse(GPS.lastNMEA()))   		// this also sets the newNMEAreceived() flag to false
			return;  							// we can fail to parse a sentence in which case we should just wait for another
	}

	gps->hour = GPS.hour;
	gps->min = GPS.minute;
	gps->sec = GPS.seconds;
	gps->year = GPS.year;
	gps->month = GPS.month;
	gps->day = GPS.day;

	gps->lat = GPS.lat;
	gps->lon = GPS.lon;
	gps->latitude = GPS.latitudeDegrees;
	gps->longitude = GPS.longitudeDegrees;

	gps->nb_sat = GPS.satellites;
	if(GPS.fix)
		gps->data_valid = 'A';
	else
		gps->data_valid = 'B';

	gps->alt_gps = GPS.altitude;
	gps->speed = GPS.speed;
	gps->angle = GPS.angle;
}

SIGNAL(TIMER0_COMPA_vect)
{
	char c = GPS.read();
}




void printGPSData(GPS_s* gps)
{
	Serial.print("\n\t*** GPS ***\n");
	Serial.print(gps->day); Serial.print("/"); Serial.print(gps->month); Serial.print("/20"); Serial.println(gps->year);
	Serial.print(gps->hour); Serial.print(":"); Serial.print(gps->min); Serial.print("."); Serial.println(gps->sec);
	Serial.print("Fix: "); Serial.println(gps->data_valid);
	if(gps->data_valid == 'A')
	{
		Serial.print("Sat: "); Serial.println(gps->nb_sat);
		Serial.print("Lat: "); Serial.print(gps->latitude, 4); Serial.print(" "); Serial.println(gps->lat);
		Serial.print("Lon: "); Serial.print(gps->longitude, 4); Serial.print(" "); Serial.println(gps->lon);
	}
}

void printVoltage(float* voltage)
{
	Serial.print("\nVolatge: "); Serial.print(*voltage, 4); Serial.println("V");
}





void writeAcq(ACQ_s* acq)
{
	Serial.write((char*)&(acq->header), sizeof(header_s));

	if(acq->header.sensors & GPS_MASK)
		Serial.write((char*)&(acq->gps), sizeof(GPS_s));

	if(acq->header.sensors & BAR_MASK)
		Serial.write((char*)&(acq->bar), sizeof(BAR_s));

	if(acq->header.sensors & ACC_MASK)
		Serial.write((char*)&(acq->acc), sizeof(ACC_s));

	if(acq->header.sensors & GYR_MASK)
		Serial.write((char*)&(acq->gyr), sizeof(GYR_s));

	if(acq->header.sensors & MAG_MASK)
		Serial.write((char*)&(acq->mag), sizeof(MAG_s));

	if(acq->header.sensors & BAT_MASK)
		Serial.write((char*)&(acq->voltage), sizeof(float));
}































