/*
    Name:       CoolmeleonWaterPump.ino
    Created:	1/8/2024 6:37:12 PM
    Author:     Garrett Miller
*/


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//#define USE_U8G2

#include "ToggleSwitch.h"
#include "BlunoShield.h"
#include "OLEDMenu.h"
#include "BiDiWaterPump.h"
#include "HeatingDevice.h"
//#include "SHT30Sensor.h"
#include "DS18B20Probe.h"
#include "LEDToggleSwitch.h"

#pragma region Global Variables

#pragma region Water Pump

//1st pin should have PWM support to control strength
//2nd and 3rd pins determine direction of flow
BiDiWaterPump waterPump(44, 50, 51);

uint8_t TurnPumpOn(bool on)
{
	return waterPump.TurnOn(on);
}

RangedValueB pumpRelayState = RangedValueB("Pump State", TurnPumpOn);

void SetPumpStrength(uint8_t newStrength)
{
	/*return*/ waterPump.SetPWMStrength(newStrength);
}

RangedValueU8 pumpStrengthValue = RangedValueU8("Pump Strength", SetPumpStrength);

#pragma endregion

#pragma region Temp

DS18B20Probe tempProbe;

//SHT30Sensor sht;

#pragma endregion

#pragma region Cricket Heater

HeatingDevice heater1(22, &tempProbe, 70, 75);

#pragma endregion

LEDToggleSwitch heaterToggle(24, 25, "Heater Switch");

#pragma endregion

#pragma region Menu System

#pragma region Water Pump Page

MenuOption waterPumpPageOptions[] =
{
	MenuOption("Enabled", pumpRelayState),
	MenuOption("Strength", pumpStrengthValue)
};

OLEDPage waterPumpPage("Water Pump", MenuOptionCount(waterPumpPageOptions), waterPumpPageOptions);

#pragma endregion

#pragma region Device Page

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// The menu options for both of the fans and the fogger
MenuOption devicesPageOptions[] =
{
	MenuOption("Water Pump", &waterPumpPage)

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

	//menu.Draw();

	/******************************************************/
	oled.setFont(u8g_font_helvB24);
	oled.disableCursor();
	oled.drawRFrame(0, 0, 127, 34, 5);
	oled.setPrintPos(3, 29);	//set the print position
	oled.print(tempProbe.GetTemperature(), 0);
	oled.write(0xB0);			//degree symbol

	//oled.setPrintPos(61, 29);
	//oled.print(sht.GetHumidity(), 0);		//show the humidity on pOled
	//oled.print(F("%"));

	String messageText = "Heater: ";
	messageText += heater1.IsOn() ? "ON" : "OFF";
	oled.setFont(u8g_font_helvB10);
	oled.setPrintPos(0, 60);
	oled.print(messageText);
	//oled.setFont(u8g_font_helvB24);
	//oled.disableCursor();
	////oled.drawRFrame(0, 0, 127, 34, 5);
	//oled.setPrintPos(3, 29);	//set the print position
	//oled.print(blunoShield.GetKnobValue());

	//oled.setPrintPos(3, 58);	//set the print position
	//oled.print(digitalRead(22) == HIGH ? "ON" : "OFF");

	//if (blunoShield.GetJoystickValue() == eJoy_Left)
	//	blunoShield.SetDrawMode(BlunoShield::eDM_TempAndHumidity);
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

		//fogger.ProcessInput(input);
		//mister.ProcessInput(input);
		//uvLight.ProcessInput(input);
		//heatLamp.ProcessInput(input);

		//fan1.ProcessInput(input);
		//fan2.ProcessInput(input);
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
	//pinMode(22, INPUT_PULLUP);
	waterPump.Initialize();

	heater1.Initialize();

	blunoShield.SetDrawMode(BlunoShield::eDM_Custom);

	tempProbe.Initialize();

	//Set var names from PROGMEM strings
	/*tempMinDayBasking.SetName(F("Day Temp Min Basking"));
	tempMaxDayBasking.SetName(F("Day Temp Max Basking"));

	humMinDayBasking.SetName(F("Day Hum Min Basking"));
	humMaxDayBasking.SetName(F("Day Hum Max Basking"));*/
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

	//Add the devices page
	menu.CurrentPage()->AddOption(MenuOption(F("Devices"), &devicesPage));

	//**********************************************
	// NO MORE MENU OPTIONS ADDED AFTER HERE
	////////////////////////////////////////////////

	//Initialize Bluno shield's built-in menu pages (AFTER we have added all potential pages)
	blunoShield.InitMenuPages(menu);

	//Initialize local menu pages
	waterPumpPage.Init(menu);

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

	//otherwise, if we are in the default, initial (or "root") screen of the menu system
	//(defaulted to display temperature and humidity), AND then the joystick is pushed,
	//set the Bluno Shield draw mode to custom, so WE can draw what we want,
	//(which will be the actual menu system and pages)
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

	//if (blunoShield.GetJoystickValue() == eJoy_Up)//digitalRead(22) == LOW)
	//	waterPump.TurnOn();
	//else if (blunoShield.GetJoystickValue() == eJoy_Down)
	//	waterPump.TurnOn(false);
	
	DS18B20Probe::UpdateSensors();

	//waterPump.SetPWMStrength(blunoShield.GetKnobValue() / 4);
	waterPump.Update();

	heater1.Update();

	float temp = tempProbe.GetTemperature();

	if (heater1.IsOn())
		blunoShield.GetLED().SetColor(255, 0, 0);
	else if (temp > heater1.GetMaxTemp())
		blunoShield.GetLED().SetColor(0, 255, 0);

	static ArduinoTimer tooHotAlarmTimer(10 TSECONDS, true);

	if (temp > 90 && tooHotAlarmTimer.IsReady())
	{
		blunoShield.GetBuzzer().Play(220, 1000);
	}

	/*if (temp < heater1.GetMaxTemp())
		blunoShield.GetLED().SetColor(255, 0, 0);
	else if (temp > heater1.GetMaxTemp())
		blunoShield.GetLED().SetColor(0, 0, 255);
	else
		blunoShield.GetLED().SetColor(0, 255, 0);*/
}

/// <summary>
/// Ensure that minimums are smaller than maximums
/// </summary>
void EnforceMinSmallerThanMax()
{
	//Enforce that the minimum temp is the smaller of the two between it and the maximum
	/*if (tempMinDay > tempMaxDay)
		Swap(RangedValueU8, tempMinDay, tempMaxDay);

	if (tempMinNight > tempMaxNight)
		Swap(RangedValueU8, tempMinNight, tempMaxNight);*/

	//Enforce that the minimum humidity is the smaller of the two between it and the maximum
	/*if (humMinDay > humMaxDay)
		Swap(RangedValueU8, humMinDay, humMaxDay);

	if (humMinNight > humMaxNight)
		Swap(RangedValueU8, humMinNight, humMaxNight);*/
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
	SetupMenuSystem();

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
	UpdateMenuSystem();

	//Set the message text, if any
	//SetBlunoMessageText();

	//Send any data we need to ESP8266
	//SendData();

	//Update all of our objects (BlunoShield, lights,
	//fogger, mister, heater, fans, etc)
	UpdateObjects();
}

#pragma endregion