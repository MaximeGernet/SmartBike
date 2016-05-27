#include <HardwareSerial.h>

#include "com.h"

void initComs(HardwareSerial* Serial)
{
	Serial->begin(115200);
}
