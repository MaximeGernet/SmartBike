#ifndef _FILE_MNGT
#define _FILE_MNGT

#define FILE_NAME_SIZE 64
#define BUF_SIZE 1024

void initSD(unsigned int* track_number, bool init_SD);

void saveFile(File* logFile, File* tracksFile, char* file_name, unsigned long acq_cnt);

//File newFile(unsigned int* track_number, unsigned int* sub_track_number, char* file_name);
void newFile(File* logFile, unsigned int* track_number, unsigned int* sub_track_number, char* file_name);

void initFile(File* logFile);

void fwriteAcq(File* logFile, ACQ_s* acq);

#endif
