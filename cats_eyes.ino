/*		CAT'S EYES

Authors: Maxime Gernet, Karim Kharboush

Project: Cat's Eyes

Target: ATmega 1284P

*/


// Essayer de changer le delay dans getBarValues()
// Essayer d'augmenter le temps entre deux envois
// Mon hypothèse : le fichier est fermé car trop d'écritures ou je ne sais quoi

// A faire :
// newFile (anciennement initFile)
// saveFile
// Ne pas oublier la gestion de Tracks.txt
// Par exemple : Track_0_0.track, Track_0_1.track, Track_0_2.track

// Afficher le nombre d'acquisitions lues, le nombre de points, et le temps divisé par la période d'acquisition


#include <SD.h>
#include <Wire.h>
#include <Adafruit_GPS.h>
#include <string.h>

#include "IO.h"
#include "sensors.h"
#include "fileMngt.h"
#include "com.h"

#define SLEEP			0
#define LIVE_VISION		1
#define TRACK			2

//#define SERIAL_DEBUG

File logFile;
File tracksFile;

PROM_s prom;

Adafruit_GPS GPS(&Serial1);

char mode;

void setup()
{
	initIO();

	//initComs(&Serial);
	Serial.begin(115200);
	Serial1.begin(9600);

    delay(1000);
    Wire.begin();

	delay(1000);
		
	initSensors();
		
	//initFile(&logFile);

	mode = SLEEP; 
}

void loop()
{
	unsigned long t_ref_LED1 = 0;
	unsigned long t_ref_LED2 = 0;
	unsigned long t_ref_GPS = 0;
	unsigned long t_ref_ACC = 0;
	unsigned long t_ref_BAR = 0;
	unsigned long t_ref_GYR = 0;
	unsigned long t_ref_MAG = 0;
	unsigned long t_ref_BAT = 0;
	unsigned long t_ref_ACQ = 0;
	unsigned long t_ref_File = 0;
	unsigned long t_ref_Flush = 0;

	unsigned long t_B1 = millis();
	unsigned long t_B2 = millis();

	unsigned char sensors = 0x00;

	unsigned int sub_track_number = 0;
	unsigned int track_number = 0;
	unsigned long acq_cnt = 0;
	bool init_SD = 1;

	char file_name[FILE_NAME_SIZE];

	ACQ_s acq;
	acq.header.sensors = 0;

	int test = 0;
	char buf_test[16];
	int k;

	while(1)
	{
		if(millis() - t_ref_GPS > 1000 && (sensors & GPS_MASK))
		{
			getGPSData(&acq.gps);
			acq.header.sensors |= GPS_MASK;
			t_ref_GPS = millis();
		}

                
		if(millis() - t_ref_BAT > 10 && (sensors & BAT_MASK))
		{
			getBatVoltage(&acq.voltage);
			acq.header.sensors |= BAT_MASK;
			t_ref_BAT = millis();
		}


		if(millis() - t_ref_GYR > 10 && (sensors & GYR_MASK))
		{
			getGyrValues(&acq.gyr);
			acq.header.sensors |= GYR_MASK;
			t_ref_GYR = millis();
		}


		if(millis() - t_ref_ACC > 10 && (sensors & ACC_MASK))
		{
			getAccValues(&acq.acc);
			acq.header.sensors |= ACC_MASK;
			t_ref_ACC = millis();
		}

		if(millis() - t_ref_MAG > 10 && (sensors & MAG_MASK))
		{
			getMagValues(&acq.mag);
			acq.header.sensors |= MAG_MASK;
			t_ref_MAG = millis();
		}
		
		if(millis() - t_ref_BAR > 400 && (sensors & BAR_MASK))
		{
			getBarValues(&acq.bar);
			acq.header.sensors |= BAR_MASK;
			t_ref_BAR = millis();
		}
		


		if(millis() - t_ref_ACQ > 20)
		{
			if(mode == SLEEP)
			{
				if(Serial.available() > 3)
				{
					for(k = 0; k < 4; k++)
						buf_test[k] = Serial.read();
					buf_test[4] = 0;
					//Serial1.print("RECEIVED: "); Serial1.write(buf_test, 5); Serial1.print("\n");
					if(strncmp(buf_test, "BURS", 4) == 0)
					{
						mode = LIVE_VISION;
					}
					else if(strncmp(buf_test, "ECHO", 4) == 0)
					{
						Serial.write("ECHO", 4);
					}
					else if(strncmp(buf_test, "SENS", 4) == 0)
					{
						if(Serial.available() > 0)
							sensors = Serial.read();
					}
					else
					{
						while(Serial.available() > 0)
							Serial.read();
					}
				}

				if(millis() - t_B1 > 500)
				{
					mode = TRACK;
					sensors = 0xFF;
					
					// Ouverture du fichier
					initSD(&track_number, init_SD);
					init_SD = 0;
					sub_track_number = 0;
					acq_cnt = 0;
					newFile(&logFile, &track_number, &sub_track_number, file_name);
					t_ref_File = millis();
					t_ref_Flush = millis();
				}
			}
			else if(mode == LIVE_VISION)
			{
				acq.header.time_stamp = (float)millis()/1000;
				writeAcq(&acq);
				acq.header.sensors = 0x00;
			}
			else if(mode == TRACK)
			{
				acq.header.time_stamp = millis()/1000;
				fwriteAcq(&logFile, &acq);
				acq.header.sensors = 0x00;
				acq_cnt++;

				if((millis() - t_ref_File)/1000 > (20 * 60))
				{
					saveFile(&logFile, &tracksFile, file_name, acq_cnt);
					newFile(&logFile, &track_number, &sub_track_number, file_name);
					acq_cnt = 0;
					t_ref_File = millis();
					t_ref_Flush = millis();
				}

				if(millis() - t_ref_Flush > 30000)
				{
					logFile.flush();
					t_ref_Flush = millis();
				}

				if(millis() - t_B2 > 500)
				{
					saveFile(&logFile, &tracksFile, file_name, acq_cnt);
					mode = SLEEP;
				}
			}
			t_ref_ACQ = millis();
		}

		if(digitalRead(B1) == 0)
		{
			if(mode == SLEEP)
			{
				digitalWrite(LED1, HIGH);
			}
		}
		else
		{
			t_B1 = millis();
			if(mode == SLEEP)
			{
				if(millis() - t_ref_LED1 > 1000)
				{
					digitalWrite(LED1, HIGH);
					t_ref_LED1 = millis();
				}
				else if(millis() - t_ref_LED1 > 50)
				{
					digitalWrite(LED1, LOW);
				}
			}
			else if(mode == LIVE_VISION)
			{
				if(millis() - t_ref_LED1 > 200)
				{
					digitalWrite(LED1, HIGH);
					t_ref_LED1 = millis();
				}
				else if(millis() - t_ref_LED1 > 50)
				{
					digitalWrite(LED1, LOW);
				}
			}
			else if(mode == TRACK)
			{
				if(millis() - t_ref_LED1 > 5000)
				{
					digitalWrite(LED1, HIGH);
					t_ref_LED1 = millis();
				}
				else if(millis() - t_ref_LED1 > 500)
				{
					digitalWrite(LED1, LOW);
				}
			}
		}

		if(digitalRead(B2) == 0)
		{
			digitalWrite(LED2, HIGH);
		}
		else
		{
			t_B2 = millis();
			digitalWrite(LED2, LOW);
		}
	}
}
