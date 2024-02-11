/*
	Name:       Coolmeleon.ino	(Chameleon Cage Monitoring System)
	Created:	6/30/2020 5:56:00 PM
	Author:     Garrett Miller
	Notes:		This code heavily uses Editor Outlining as used in Visual Studio, with
				frequent use of "#pragma region (Region Label/Name)\n ... \n ... \n #pragma endregion"
				to allow collapse of regions of code and comments into logical groupings.
				For those unfamiliar, Ctrl+M, Ctrl+L toggles all outlining open/closed
				while Ctrl+M, Ctrl+O collapses all outlining to closed. Handy for seeing related
				groups of code and temporarily "hiding" other code.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes

#include "SHT30Sensor.h"
#include "DS18B20Probe.h"
#include "BlunoShield.h"
#include "OLEDMenu.h"
#include "ReptileFogger.h"
#include "ReptileMister.h"
#include "PCFan.h"
#include <Adafruit_SHT31.h>

#pragma region Global Variables

bool isDaytime = true;

#pragma region Temperature & Humidity Ranges

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//		The ranged values that represent acceptable temperature and humidity ranges
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Temperature

///////////////////////////////////////////////////////////////////////////////////////////////////
// Temperature range
///////////////////////////////////////////////////////////////////////////////////////////////////
//	Daytime temps need to range from 90-ish+ at the basking spot down to 70 at the bottom in the "shade"
//											name				initial		min			max
RangedValueU8 tempMinDayBasking = RangedValueU8(	80,		70,		85);	//deg F
RangedValueU8 tempMaxDayBasking = RangedValueU8(	90,		70,		95);	//deg F

RangedValueU8 tempMinDayShade = RangedValueU8(		70,		65,		75);	//deg F
RangedValueU8 tempMaxDayShade = RangedValueU8(		80,		70,		85);	//deg F

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Night-time temps need to stay at or above 65 ideally, but not much higher than 70+ (no basking at night)
//											name				initial		min			max
RangedValueU8 tempMinNightBasking = RangedValueU8(	60,		60,		65);	//deg F
RangedValueU8 tempMaxNightBasking = RangedValueU8(	70,		65,		70);	//deg F

RangedValueU8 tempMinNightShade = RangedValueU8(	60,		60,		65);	//deg F
RangedValueU8 tempMaxNightShade = RangedValueU8(	70,		65,		70);	//deg F

#pragma endregion

#pragma region Humidity

///////////////////////////////////////////////////////////////////////////////////////////////////
// Humidity range
///////////////////////////////////////////////////////////////////////////////////////////////////
// Minimum (non-stressful) survivable humidity: ~40%
// Maximum (approaching unhealthy levels, but definitely lower): 90%
// Closer to 50% actually better and greatly reduces bacterial growth/spread
//											name				initial		min			max
RangedValueU8 humMinDayBasking = RangedValueU8(		40,		40,		60);	//%
RangedValueU8 humMaxDayBasking = RangedValueU8(		60,		60,		90);	//%

RangedValueU8 humMinDayShade = RangedValueU8(		40,		40,		60);	//%
RangedValueU8 humMaxDayShade = RangedValueU8(		60,		60,		90);	//%

///////////////////////////////////////////////////////////////////////////////////////////////////
// Night humidity minimum: ~70%		(Need to research this number more!!)
// Night humidity maximum: 100%		(Again, ??)
//											name				initial		min			max
RangedValueU8 humMinNightBasking = RangedValueU8(	80,		50,		60);	//%
RangedValueU8 humMaxNightBasking = RangedValueU8(	100,	60,		100);	//%

RangedValueU8 humMinNightShade = RangedValueU8(		80,		50,		60);	//%
RangedValueU8 humMaxNightShade = RangedValueU8(		100,	60,		100);	//%

#pragma endregion

#pragma endregion

#pragma region Devices

#pragma region CPU Fans

//I've got two so far
PCFan fan1(32, "Fan 1");
PCFan fan2(33, "Fan 2");

#pragma endregion

#pragma region Temperture/Humidity Probes

#pragma region DS18B20's (temperature only)

//two of them at least
DS18B20Probe tempProbe(0);
DS18B20Probe tempProbe2(1);

#pragma endregion

#pragma region SHT30's

Adafruit_SHT31 shtBaskingSpot = Adafruit_SHT31();
//Adafruit_SHT31 shtShadeSpot = Adafruit_SHT31();

#pragma endregion

#pragma endregion

#pragma region Water Heater

//WaterHeatingDevice waterHeater(25, 26);

#pragma endregion

#pragma region Fogger (You mother-fogger!)

///////////////////////////////////////////////////////////////////////////////////////////////////
// Reptile Fogger
ReptileFogger fogger(22, NULL/*waterHeater*/);
//SwitchDevice fogger(22, "Fogger", 5 TMINUTES);

/// <summary>
/// A callback function that the below RangedValue (RangedValueB) calls to update
/// the fogger's on/off state according to that particular RangedValue
/// </summary>
/// <param name="on">Whether or not the fogger is to be turned on (<c>true</c>) or off (<c>false</c>) </param>
/// <returns>Whether the fogger is off (0) or on (non-zero)</returns>
uint8_t TurnFoggerOn(bool on)
{
	return fogger.TurnOn(on);
}

/// <summary>
/// The RangedValue that represents the local fogger on/off state.
/// </summary>
RangedValueB foggerRelayState = RangedValueB("Fogger State", TurnFoggerOn);

#pragma endregion

#pragma region Heat Lamp

///////////////////////////////////////////////////////////////////////////////////////////////////
// Heat Lamp
SwitchDevice heatLamp(23, "Heat Lamp", 12 THOURS);

uint8_t TurnHeatLampOn(bool on)
{
	heatLamp.TurnOn(on);
}

RangedValueB heatLampRelayState = RangedValueB("Heat Lamp", TurnHeatLampOn);

#pragma endregion

#pragma region UV Light

///////////////////////////////////////////////////////////////////////////////////////////////////
// UV Light
SwitchDevice uvLight(24, "UV Light", 12 THOURS);

uint8_t TurnUVLightOn(bool on)
{
	uvLight.TurnOn(on);
}

RangedValueB uvLightRelayState = RangedValueB("UV Light", TurnUVLightOn);

#pragma endregion

#pragma region Mister

ReptileMister mister(40/*, waterHeater, 45, 0*/);

uint8_t TurnMisterOn(bool on)
{
	mister.TurnOn(on);
}

RangedValueB misterRelayState = RangedValueB("Mister", TurnMisterOn);

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region Menu System

#pragma region Temperature

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// The menu options on the "Ranges" page that allows adjustment of min/max temperatures, both day and night
MenuOption temperatureRangePageOptions[] =
{
	MenuOption("",		tempMinDayBasking),	//The name to display on-screen and then a reference to the variable to change, in this case, the RangedValue8 defined above called tempMin
	MenuOption("",		tempMaxDayBasking),
	MenuOption("",	tempMinNightBasking),
	MenuOption("",	tempMaxNightBasking),
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//The menu page for the min/max temp options
OLEDPage temperatureRangesPage("Temp Ranges", MenuOptionCount(temperatureRangePageOptions), temperatureRangePageOptions);

#pragma endregion

#pragma region Humidity

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// The menu options for humidity
MenuOption humidityRangesPageOptions[] =
{
	MenuOption("",		humMinDayBasking),
	MenuOption("",		humMaxDayBasking),
	MenuOption("",		humMinNightBasking),
	MenuOption("",		humMaxNightBasking),
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// The menu page for the min/max humidity
OLEDPage humidityRangesPage("Hum Ranges", MenuOptionCount(humidityRangesPageOptions), humidityRangesPageOptions);

#pragma endregion

#pragma region Device Page

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// The menu options for both of the fans and the fogger
MenuOption devicesPageOptions[] =
{
	MenuOption("", foggerRelayState),
	MenuOption("", heatLampRelayState),
	MenuOption("", uvLightRelayState),
	MenuOption("", misterRelayState)
};

OLEDPage devicesPage("Local Devices", MenuOptionCount(devicesPageOptions), devicesPageOptions);

#pragma endregion

#pragma endregion

#pragma region Callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// The callback function for the Bluno shield OLED screen to draw what you want (the menu system).
/// This is necessary because drawing to the OLED can only be done at a very specific point in the
/// code, namely the oled.firstPage()/oled.nextPage() loop found in BlunoShield::UpdateOLED(), that
/// is called only ONCE PER LOOP()
/// </summary>
/// <param name="oled">A reference to the Bluno shield OLED screen</param>
void drawCallback(OLED& oled)
{
	//Just in case the menu system's pointer to the OLED is NULL, set it
	if (!menu.pOled)
		menu.pOled = &oled;

	menu.Draw();

	/******************************************************/
	
	//oled.setFont(u8g_font_helvB24);
	//oled.disableCursor();
	//oled.drawRFrame(0, 0, 127, 34, 5);
	//oled.setPrintPos(3, 29);	//set the print position
	//oled.print(analogRead(A8) * 5.0 / 1023.0);

	if (blunoShield.GetJoystickValue() == eJoy_Left)
		blunoShield.SetDrawMode(BlunoShield::eDM_TempAndHumidity);
	/******************************************************/
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// The callback function for the Bluno Shield PlainProtocol (technically not the shield itself but
/// an actual DFRobot Bluno-board)
/// </summary>
/// <param name="input">The PlainProtocol object (reference)</param>
void processInputCallback(PlainProtocol& input)
{
	//if (input.receiveFrame())
	{
		//Serial.println("HERE");
		//do your own BLE processing if needed

		fogger.ProcessInput(input);
		mister.ProcessInput(input);
		uvLight.ProcessInput(input);
		heatLamp.ProcessInput(input);

		fan1.ProcessInput(input);
		fan2.ProcessInput(input);
	}
}

#pragma endregion

#pragma region Main Sub Routines

#pragma region Setup Routines

/// <summary>
/// Initializes all local devices
/// </summary>
void InitializeDevices()
{
	//waterHeater.Initialize();
	
	//Init DS18B20's!!

	mister.Initialize();

	uvLight.Initialize();

	fogger.SetTimerOnPeriod(10 TMINUTES);
	fogger.SetSwitchMode(SwitchDevice::eSDM_ManualOnly);
	fogger.Initialize();

	heatLamp.Initialize();

	fan1.Initialize();
	fan2.Initialize();

	shtBaskingSpot.begin();

	//Set var names from PROGMEM strings
	tempMinDayBasking.SetName(F("Day Temp Min Basking"));
	tempMaxDayBasking.SetName(F("Day Temp Max Basking"));
	tempMinDayShade.SetName(F("Day Temp Min Shade"));
	tempMaxDayShade.SetName(F("Day Temp Max Shade"));

	humMinDayBasking.SetName(F("Day Hum Min Basking"));
	humMaxDayBasking.SetName(F("Day Hum Max Basking"));
	humMinDayShade.SetName(F("Day Hum Min Shade"));
	humMaxDayShade.SetName(F("Day Hum Max Shade"));
	
	tempMinNightBasking.SetName(F("Night Temp Min Basking"));
	tempMaxNightBasking.SetName(F("Night Temp Max Basking"));
	tempMinNightShade.SetName(F("Night Temp Min Shade"));
	tempMaxNightShade.SetName(F("Night Temp Max Shade"));

	humMinNightBasking.SetName(F("Night Hum Min Basking"));
	humMaxNightBasking.SetName(F("Night Hum Max Basking"));
	humMinNightShade.SetName(F("Night Hum Min Shade"));
	humMaxNightShade.SetName(F("Night Hum Max Shade"));
}

/// <summary>
/// Initializes the BlunoShield accessory completely
/// </summary>
void InitializeBlunoShield()
{
	//Initialize Bluno shield
	blunoShield.Initialize();

	//Set the Bluno shield to do internal (default) PlainProtocol processing,
	//but also our external, callback function too
	//(processInputCallback(PlainProtocol& input))
	blunoShield.SetInputProcessingMode(BlunoShield::eIPM_Both);
	blunoShield.SetInputCallback(processInputCallback);

	//Set Bluno shield callbacks for drawing
	blunoShield.SetDrawCallback(drawCallback);	
}

/// <summary>
/// Sets the menu system up
/// </summary>
void SetupMenuSystem()
{
	//Init menu system with reference to the Bluno shield
	menu.Init(blunoShield);

	////////////////////////////////////////////////
	// ADD ALL MENU OPTIONS HERE
	//**********************************************

	//Add the menu pages containing the temp and humidity range options to the main page
	menu.CurrentPage()->AddOption(MenuOption(F("Temperature Ranges"), &temperatureRangesPage));
	menu.CurrentPage()->AddOption(MenuOption(F("Humidity Ranges"), &humidityRangesPage));

	//Add the devices page
	menu.CurrentPage()->AddOption(MenuOption(F("Devices"), &devicesPage));

	//**********************************************
	// NO MORE MENU OPTIONS ADDED AFTER HERE
	////////////////////////////////////////////////

	//Initialize Bluno shield's built-in menu pages (AFTER we have added all potential pages)
	blunoShield.InitMenuPages(menu);

	//Initialize local menu pages
	temperatureRangesPage.Init(menu);
	humidityRangesPage.Init(menu);
	devicesPage.Init(menu);
}

#pragma region SERIAL Routines

/// <summary>
/// Initializes serial 0
/// </summary>
void BeginSerial0(unsigned long baud = 115200)
{
	//static ArduinoTimer errorTimer(1 TSECONDS, true);

	//Begin serial
	Serial.begin(baud);

	while (!Serial /*&& errorTimer.IsReady()*/)
	{
		Serial.println(F("Can't initialize Serial 0"));
	};//Wait for Serial to initialize
}

/// <summary>
/// Initializes serial 1
/// </summary>
void BeginSerial1(unsigned long baud = 9600)
{
	//static ArduinoTimer errorTimer(1 TSECONDS, true);

	Serial1.begin(baud);

	while (!Serial1 /*&& errorTimer.IsReady()*/)
	{
		Serial.println(F("Can't initialize Serial 1"));
	};//Wait for Serial1 to initialize
}

/// <summary>
/// Initializes serial 2
/// </summary>
void BeginSerial2(unsigned long baud = 9600)
{
	//static ArduinoTimer errorTimer(1 TSECONDS, true);

	Serial2.begin(baud);

	while (!Serial2 /*&& errorTimer.IsReady()*/)
	{
		Serial.println(F("Can't initialize Serial 2"));
	};//Wait for Serial1 to initialize
}

/// <summary>
/// Initializes serial 3
/// </summary>
void BeginSerial3(unsigned long baud = 9600)
{
	//static ArduinoTimer errorTimer(1 TSECONDS, true);

	Serial3.begin(baud);

	while (!Serial3 /*&& errorTimer.IsReady()*/)
	{
		Serial.println(F("Can't initialize Serial 3"));
	};//Wait for Serial1 to initialize
}

/// <summary>
/// Initializes all (necessary) Serial connections
/// </summary>
void InitializeAllSerial()
{
	//Initialize the main serial (USB<-->Arduino)
	BeginSerial0();

	//BeginSerial1();

	//BeginSerial2();

	//Initialize the 3rd serial (Arduino<-->ESP8266)
	BeginSerial3();

	Serial.println(F("All Serial initialized!"));
}

#pragma endregion

#pragma endregion

/// <summary>
/// Set the heater (and other related things) on or off, based on temperature
/// </summary>
void ProcessTemperature()
{
	if (isDaytime)
	{
		float baskTemp = shtBaskingSpot.readTemperature();

		if (baskTemp > tempMaxDayBasking)
		{
			if (heatLamp.IsOn())
				heatLamp.TurnOn(false);
			else
				Serial.println(F("******TOO HOT IN CAGE!!"));
		}
		else if (baskTemp < tempMinDayBasking)
		{
			if (!heatLamp.IsOn())
				heatLamp.TurnOn();
			else
				Serial.println(F("******TOO COLD IN CAGE!!"));
		}

		//shade spot
		float shadeTemp = tempProbe.GetTemperature();

		if (shadeTemp > tempMaxDayShade)
		{
			fan1.TurnOn();
			fan2.TurnOn();
		}
		else
		{
			fan1.TurnOn(false);
			fan2.TurnOn(false);
		}
	}
	else //is night time
	{
		float baskTemp = shtBaskingSpot.readTemperature();
		float shadeTemp = tempProbe.GetTemperature();

		if (baskTemp > tempMaxNightBasking || shadeTemp > tempMaxNightShade)
		{
			//heatLamp.TurnOn(false);
			fan1.TurnOn();
			fan2.TurnOn();
		}
		else if (baskTemp < tempMinNightBasking)
		{
			//heater on
		}
	}
}

/// <summary>
/// Set the fogger (and other related things) on or off, based on humidity
/// </summary>
void ProcessHumidity()
{
	float humidityBasking = shtBaskingSpot.readHumidity();// blunoShield.humidity;

	if (humidityBasking < (isDaytime ? humMinDayShade : humMinNightShade)
		&& !fogger.IsOn())
		fogger.TurnOn();
	else if (humidityBasking > (isDaytime ? humMaxDayShade : humMaxNightShade)
		&& fogger.IsOn())
		fogger.TurnOn(false);
}

/// <summary>
/// Set the Bluno shield's default message text, based on the status of both the fogger and heater
/// </summary>
void SetBlunoMessageText()
{

}

/// <summary>
/// Update the menu system
/// </summary>
void UpdateMenuSystem()
{
	//if the Bluno Accessory Shield screen draw mode is set to custom, indicating
	//WE (end-user, here, in the main Arduino sketch) tell it what to draw,
	//update the menu system (which essentially handles input and drawing the appropriate page).
	if (blunoShield.GetDrawMode() == BlunoShield::eDM_Custom)
		menu.Update();

	//otherwise, if we are in the default, initial (or "root") screen of the menu system (defaulted to display temperature and humidity),
	//AND then the joystick is pushed, set the Bluno Shield draw mode to custom, so WE can draw what we want, (which will be the actual menu system and pages)
	else if (blunoShield.GetJoystickValue() == eJoy_Push && blunoShield.GetDrawMode() == BlunoShield::eDM_TempAndHumidity)
		blunoShield.SetDrawMode(BlunoShield::eDM_Custom);
}

/// <summary>
/// Send data over the serial port (every second, by default) to any device that may be listening
/// </summary>
void SendData()
{
	//Create a local, static (persistent) timer, set to 1000 ms (or 1 second)
	static ArduinoTimer serialTimer(1 TSECONDS, true);
	if (serialTimer.IsReady())	//The timer auto-resets by default if IsReady() is true
	{
		//Serial.println("Sending data to ESP...");
		String send = F("");// "T:";
		send = send + blunoShield.temperature;
		//Serial3.println(send);

		send = send + F("~");
		send = send + blunoShield.humidity;
		//Serial.println(send);

		Serial3.println(send);
	}
}

/// <summary>
/// Update the lights, the fogger, and the Bluno shield itself
/// </summary>
void UpdateObjects()
{
	//Update the bluno shield itself
	blunoShield.Update();

	//Serial.print("UpdateObjects: ");
	//Serial.println(int(&menu), HEX);

	/*static ArduinoTimer timer(10 TSECONDS, true);

	if (timer.IsReady())
	{
		fogger.Toggle();
		heatLamp.Toggle();
		uvLight.Toggle();
		mister.Toggle();
		fan1.Toggle();
	}*/
	
	//DS18B20Probe::UpdateSensors();

	//waterHeater.Update();

	fogger.Update();
	heatLamp.Update();
	uvLight.Update();
	mister.Update();

	fan1.Update();
	fan2.Update();
}

/// <summary>
/// Ensure that minimums are smaller than maximums
/// </summary>
void EnforceMinSmallerThanMax()
{
	//Enforce that the minimum temp is the smaller of the two between it and the maximum
	if (tempMinDayBasking > tempMaxDayBasking)
		Swap(RangedValueU8, tempMinDayBasking, tempMaxDayBasking);

	if (tempMinNightBasking > tempMaxNightBasking)
		Swap(RangedValueU8, tempMinNightBasking, tempMaxNightBasking);

	//Enforce that the minimum humidity is the smaller of the two between it and the maximum
	if (humMinDayBasking > humMaxDayBasking)
		Swap(RangedValueU8, humMinDayBasking, humMaxDayBasking);

	if (humMinNightBasking > humMaxNightBasking)
		Swap(RangedValueU8, humMinNightBasking, humMaxNightBasking);
}

/// <summary>
/// Eventually, this function will send a customizable phone notification when the user needs an alert
/// </summary>
/// <param name="alertText"></param>
void SendAlert(const char* alertText)
{
	Serial.print(F("AlertText: "));
	Serial.println(alertText);

	//DO ACTUAL ALERT CALLS
}

#pragma endregion

#pragma region MAIN _ARDUINO_ FUNCTIONS

/// <summary>
/// THE MAIN SETUP function for Arduino programs
/// </summary>
void setup()
{
	//First, and foremost, initialize all necessary Serial connections
	InitializeAllSerial();

	//Set up the Bluno accessory shield
	InitializeBlunoShield();

	//Create and initialize the menu system
	//SetupMenuSystem();

	//Initialize all local devices
	InitializeDevices();
}

/// <summary>
/// THE MAIN ARDUINO LOOP. 'Nuff said
/// </summary>
void loop()
{
	//For sanity's sake, ensure that all minimums are
	//smaller than all maximums, and vice versa
	EnforceMinSmallerThanMax();

	//Update the menu system (handles drawing and input)
	//UpdateMenuSystem();

	//Check temperature values and adjust accordingly
	//ProcessTemperature();

	//Check humidity levels and adjust accordingly
	//ProcessHumidity();

	//Set the message text, if any
	//SetBlunoMessageText();

	//Send any data we need to ESP8266
	//SendData();

	//Update all of our objects (BlunoShield, lights,
	//fogger, mister, heater, fans, etc)
	UpdateObjects();

	byte val = map(blunoShield.GetKnobValue(), 0, 1023, 0, 255);
	analogWrite(46, val);
}

#pragma endregion