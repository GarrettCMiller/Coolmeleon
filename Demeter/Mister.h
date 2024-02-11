// Mister.h

#ifndef _MISTER_h
#define _MISTER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SwitchDevice.h"

class Mister : public SwitchDevice
{
public:
    Mister(uint8_t pin, const char* _name, unsigned long timerLength)
        : SwitchDevice(pin, _name)
    {
    }

    Mister(uint8_t pin, const char* _name,
        SwitchDevice::ESwitchDeviceMode defaultDeviceMode = SwitchDevice::eSDM_EventOnEventOff)
        : SwitchDevice(pin, _name, false, defaultDeviceMode)
    {

    }

    /*virtual bool TurnOn(bool turnOn = true)
    {
        if (IsOn() != TurnOn())
        {
            Serial.println("Turning mister on...");
            return true;
        }

        return false;
    }

    virtual bool TurnOff()
    {
        if (IsOn() != TurnOn(false))
        {
            Serial.println("Turning mister off...");
            return false;
        }

        return true;
    }*/
};

#endif

