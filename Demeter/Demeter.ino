/*
    Name:       DEMETER.ino
                (Dirt- & Earth-Monitoring Enclosed Terrestrial Ecological Recorder -- D.E.M.E.T.E.R.) 
    Created:	11/1/2020 3:09:00 AM
    Author:     Garrett Miller
    Notes:      This project is meant to be run on an Arduino-based board, written especially for a
                Arduino-Mega type board with a Bluno Accessory Shield attatched.
*/

#include <BlunoShield.h>
#include "OLEDMenu.h"
#include "SoilSampler.h"
#include "ReptileMister.h"

#define DEBUG_VALUES

#pragma region Local Custom Classes

////////////////////////////////////////////
// Plant Monitors (DEMETERs)
# if 0
class PlantMonitor : public IArduinoDevice
{
    PlantMonitor();
    PlantMonitor(PlantMonitor&& ctrArg);
    PlantMonitor(const PlantMonitor& ctrArg);

    SoilSampler soilSample;
    Mister localMister;

    float soilMoistureMin = 0.2f;
    float soilMoistureMax = 0.7f;

public:
    PlantMonitor(uint8_t soilSamplerPin, const char* _name, unsigned long misterTimer = 5 TMINUTES)
        : soilSample(soilSamplerPin, _name),
            localMister(soilSamplerPin + 1, _name, misterTimer)
    {
    }

    virtual uint8_t Initialize()
    {
        if (soilSample.Initialize() != 0)
            return -1;
        if (localMister.Initialize() != 0)
            return -1;
        return 0;
    }

    virtual uint8_t Update()
    {
        soilSample.ReadSample();

        if (soilSample.GetSoilMoisture() < soilMoistureMin)
            TurnMisterOn();
        else if (soilSample.GetSoilMoisture() > soilMoistureMax)
            TurnMisterOff();

        localMister.Update();

        //delay timer length
    }

    void TurnMisterOff()
    {
        if (localMister.IsOn())
        {
            Serial.print("Turning mister \"");
            Serial.print(soilSample.GetName());
            Serial.println("\" off!");
            localMister.TurnOn(false);
        }
    }

    void TurnMisterOn()
    {
        if (!localMister.IsOn())
        {
            Serial.print("Turning mister \"");
            Serial.print(soilSample.GetName());
            Serial.println("\" on!");
            localMister.TurnOn();
        }
    }

    virtual void ProcessInput(PlainProtocol& input)
    {

    }
};
#endif
#pragma endregion

#pragma region Global Variables

ReptileMister mistingDevice(22, 23, 24, 25);

SoilMonitor umbrellaPlant(/*33,*/   *mistingDevice.GetMistingValveByIndex(0), "Umbrella Plant");
//SoilMonitor jadePlant(34,       *mistingDevice.GetMistingValveByIndex(1), "Jade Plant");
//SoilMonitor pothosIvy(35,       *mistingDevice.GetMistingValveByIndex(2), "Pothos Ivy");

#pragma endregion

void drawCallback(OLED& oled)
{
    //Just in case the menu system's pointer to the OLED is NULL, set it
    if (!menu.pOled)
        menu.pOled = &oled;

    menu.Draw();
}

////////////////////////////////////////////

// The setup() function runs once each time the micro-controller starts
void setup()
{
    Serial.begin(115200);

    while (!Serial);

    blunoShield.Initialize();

    blunoShield.SetDrawCallback(drawCallback);

    //Init menu system with reference to the Bluno shield
    menu.Init(blunoShield);

    //Initialize Bluno shield's built-in menu pages
    blunoShield.InitMenuPages(menu);

    mistingDevice.Initialize();

    umbrellaPlant.Initialize();
    //jadePlant.Init();
    //pothosIvy.Initialize();
}

// Add the main program code into the continuous loop() function
void loop()
{
    mistingDevice.Update();

    umbrellaPlant.Update();
    //jadePlant.Update();
    //pothosIvy.Update();

    blunoShield.Update();
}
