// LEDGroup4.h

#ifndef _LEDGROUP4_h
#define _LEDGROUP4_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "LED_RGB.h"

class LEDGroup4 : public ArduinoDevice
{
public:
	enum ELEDGroupMode
	{
		eLGM_Individual,
		eLGM_Stack
	};

protected:
	LED ledRed;
	LED ledOrange;
	LED ledYellow;
	LED ledGreen;

public:
	LEDGroup4(uint8_t pinRed, uint8_t pinOrange, uint8_t pinYellow, uint8_t pinGreen, String name = "")
		: ArduinoDevice(name),
			ledRed(pinRed), ledOrange(pinOrange), ledYellow(pinYellow), ledGreen(pinGreen)
	{
		if (name == "")
			SetName(F("LED Group"));
	}
	
	virtual uint8_t Initialize()
	{
		ledRed.Initialize();
		ledOrange.Initialize();
		ledYellow.Initialize();
		ledGreen.Initialize();

		return ArduinoDevice::Initialize();
	}

	virtual uint8_t Update()
	{
		ledRed.Update();
		ledOrange.Update();
		ledYellow.Update();
		ledGreen.Update();

		return ArduinoDevice::Update();
	}

	virtual void ProcessInput(PlainProtocol& input)
	{
		ArduinoDevice::ProcessInput(input);

		ledRed.ProcessInput(input);
		ledOrange.ProcessInput(input);
		ledYellow.ProcessInput(input);
		ledGreen.ProcessInput(input);
	}
};

//extern LEDGroup4 ledGroup4;

#endif

