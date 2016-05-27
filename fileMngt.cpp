#include <SD.h>

#include "IO.h"
#include "sensors.h"
#include "fileMngt.h"

void initSD(unsigned int* track_number, bool init_SD)
{
	char file_name[FILE_NAME_SIZE];
	pinMode(CS, OUTPUT);

	if(init_SD)
	{
		while(!SD.begin(CS))
		{
			delay(1000);
		}
	}

	*track_number = 0;
	do
	{
		sprintf(file_name, "T_%d_0.TRK", *track_number);
		*track_number = (*track_number) + 1;
	}while(SD.exists(file_name));
	*track_number = (*track_number) - 1;
}

void saveFile(File* logFile, File* tracksFile, char* file_name, unsigned long acq_cnt)
{
	char buf[BUF_SIZE];

	logFile->close();

	*tracksFile = SD.open("Tracks.txt", FILE_WRITE);
	tracksFile->seek(tracksFile->size());
	sprintf(buf, "%s,%lu", file_name, acq_cnt);
	tracksFile->println(buf);
	tracksFile->close();
}

//File newFile(unsigned int* track_number, unsigned int* sub_track_number, char* file_name)
void newFile(File* logFile, unsigned int* track_number, unsigned int* sub_track_number, char* file_name)
{
	sprintf(file_name, "T_%d_%d.TRK", *track_number, *sub_track_number);
	*sub_track_number = (*sub_track_number) + 1;

	*logFile = SD.open(file_name, FILE_WRITE);
	/*
	if(*logFile)
	{
		Serial1.print(file_name);
		Serial1.println(" created");
	}
	else
	{
		Serial1.print("Error opening ");
		Serial1.println(file_name);
	}
	*/
	
}

void fwriteAcq(File* logFile, ACQ_s* acq)
{
	logFile->write((char*)&(acq->header), sizeof(header_s));

	if(acq->header.sensors & GPS_MASK)
		logFile->write((char*)&(acq->gps), sizeof(GPS_s));

	if(acq->header.sensors & BAR_MASK)
		logFile->write((char*)&(acq->bar), sizeof(BAR_s));

	if(acq->header.sensors & ACC_MASK)
		logFile->write((char*)&(acq->acc), sizeof(ACC_s));

	if(acq->header.sensors & GYR_MASK)
		logFile->write((char*)&(acq->gyr), sizeof(GYR_s));

	if(acq->header.sensors & MAG_MASK)
		logFile->write((char*)&(acq->mag), sizeof(MAG_s));

	if(acq->header.sensors & BAT_MASK)
		logFile->write((char*)&(acq->voltage), sizeof(float));
}
