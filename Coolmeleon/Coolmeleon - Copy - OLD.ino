/*
	Name:       Coolmeleon.ino
				(Chameleon Cage Monitoring System)
	Created:	6/30/2020 5:56:00 PM
	Author:     Garrett Miller
*/

#include "BlunoShield.h"

////////////////////////////////////////////
// Temperature

// Main Unit

// Waterproof sensors (x5?)

////////////////////////////////////////////
// Humidity

// Main Unit

////////////////////////////////////////////
// UV

// UV sensor

////////////////////////////////////////////
// Plant Monitors (D.E.M.E.T.E.R.)

////////////////////////////////////////////
// Dubia Roach Colony Monitor (D.R.A.C.U.L.A.)

////////////////////////////////////////////

void setup()
{
	Serial.begin(115200);

	//while (!Serial);

	Serial.println("1");

	//blunoShield.Initialize();

	Serial.println("2");

	//Init menu system with reference to the Bluno shield
	//menu.Init(blunoShield);

	Serial.println("3");
}

void loop()
{
	Serial.println("4");
	//blunoShield.Update();
}