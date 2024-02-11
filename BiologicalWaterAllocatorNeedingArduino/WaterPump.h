// WaterPump.h

#ifndef _WATERPUMP_h
#define _WATERPUMP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SwitchDevice.h"

class WaterPump : public SwitchDevice
{
public:
	WaterPump(uint8_t pin, const char* pumpName)
		: SwitchDevice(pin, pumpName, 1 TMINUTES)
	{

	}


};

#endif