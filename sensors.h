#ifndef _SENSORS
#define _SENSORS

#include <Adafruit_GPS.h>


// I2C address
#define MPU_ADDR 		0x68//0b110100X
#define MAG_ADDR 		0x0C
#define BAR_ADDR 		0x77//0xBA ?

// Accel and gyro egisters
#define CONFIG 			0x1A
#define GYRO_CONFIG 	0x1B
#define ACCEL_CONFIG 	0x1C
#define ACCEL_CONFIG2 	0x1D
#define ACCEL_XOUT_H 	0x3B
#define GYRO_XOUT_H 	0x43
#define XA_OFFSET_H 	0x77
#define USR_CTRL 		0x6A
#define PWR_MGMT1 		0x6B
#define PWR_MGMT1 		0x6C
#define XG_OFFSET_H 	0x13

// Commands to set sensors full scale
#define GYRO_FS_250 	0x00	// +-250 °/s
#define ACCEL_FS_2 		0x00	// +-2 G

// Sensors sensitivity
#define ACC_SENS 		16384	// LSB/g for +-2g full scale
#define GYR_SENS 		131		// LSB/(°/s) for +-250°/s full scale
#define MAG_SENS 		0.6		// uT/LSB

// Magnetometer registers
#define CNTL1 			0x0A	// continuous measurement mode 1 : 0x02
#define ST2				0x09
#define HXL 			0x03

// Setting the MPU9250 to work with the magnetometer
#define BYPASS 			0x37	// Configuration register for the MPU (value : 0x02 = pass-through mode ?)

// Barometer commands
#define BAR_RESET 		0x1E
#define CONV_PRES_4096 	0x48	// Initiates a pressure conversion (OSR = 4096, takes 9 ms)
#define CONV_TEMP_4096 	0x58	// Initiates a temperature conversion (OSR = 4096, takes 9 ms)
#define ADC_READ 		0x00	// Reads the result of a conversion

// Barometer esolution
#define BAR_RES_4096 	0.012	// mbar, 9 ms conversion time
#define TMP_RES_4096 	0.002	// °C, 9 ms conversion time

// Physics constants
#define SEA_LVL_P 		102750	// Pressure at sea level in Pa
#define GRAV			9.80665 // Gravitational constant in m/s²

// Masks
#define GPS_MASK 		0x20
#define BAR_MASK		0X10
#define ACC_MASK		0x08
#define GYR_MASK		0X04
#define MAG_MASK		0x02
#define BAT_MASK		0X01



// Structures utilisées pour mettre en forme les données acquises :

typedef struct 
{
	unsigned char sensors;			// Flags indiquant les capteurs utilisés lors de l'acquisition : | XXX | XXX | GPS | BAR | ACC | GYR | MAG | BAT |
	//uint32_t time_stamp;		// temps écoulé au moment de l'acquisition, en millisenconde
	float time_stamp;
	//uint8_t time_stamp;
}header_s;

typedef struct
{
	unsigned char year, month, day, hour, min, sec;
	char lat, lon; 					// 'E'/'W' and 'N'/'S'
	float latitude, longitude; 		// °
	unsigned char nb_sat;			// Number of satellites
	float alt_gps;					// Altitude (cm)
	char data_valid;				// 'A' or 'B'
	float speed;					// m/s
	float angle;					// °
}GPS_s;

typedef struct
{
	float pressure;					// hPa
	float temperature;				// °C
	float altitude;					// m
}BAR_s;

typedef struct
{
	float ax, ay, az;				// m/s²
}ACC_s;

typedef struct
{
	float gx, gy, gz;				// °/s
}GYR_s;

typedef struct
{
	float hx, hy, hz;				// °
}MAG_s;


typedef struct
{
	// header
	header_s header;
	// GPS
	GPS_s gps;
	// Barometer
	BAR_s bar;
	// Accelerometer
	ACC_s acc;
	// Gyroscope
	GYR_s gyr;
	// Magnetometer
	MAG_s mag;
	// Battery
	float voltage;	// V
}ACQ_s;


typedef struct 	// Barometer PROM fields
{
	uint8_t CRC;
	uint16_t factory;
	uint16_t C1, C2, C3, C4, C5, C6;
}PROM_s;


extern Adafruit_GPS GPS;

extern PROM_s prom;

void initGyr();

void initAcc();

void initMag();

void initBar();

void initGPS();

void initSensors();



void getAccValues(ACC_s* acc);

void getGyrValues(GYR_s* gyr);

void getMagValues(MAG_s* mag);

void getBarValues(BAR_s* bar);

void getGPSData(GPS_s* gps);



void printGPSData(GPS_s* gps);

void printVoltage(float* voltage);


void writeAcq(ACQ_s* acq);


#endif
