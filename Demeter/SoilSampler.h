// SoilSampler.h

#ifndef _SOILSAMPLER_h
#define _SOILSAMPLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SwitchDevice.h"
#include "Adafruit_seesaw.h"
#include "RangedValue.h"
#include "ReptileMister.h"

#define DEBUG_VALUES

class SoilMonitor// : public SwitchDevice //SwitchDevice is hooked to the particular mister "applied" to that plant
{
#pragma region Fields

    //according to adafruit (https://www.adafruit.com/product/4026):
    //"We use the built in capacitive touch measurement system built into the ATSAMD10 chip,
    // which will give you a reading ranging from about 200 (very dry) to 2000 (very wet)."
    static const int adafruitLowerBound = 200;
    static const int adafruitUpperBound = 2000;

    /// <summary>
    /// The soil-sampler, itself
    /// </summary>
    Adafruit_seesaw soilSampler;

    /// <summary>
    /// The current soil moisture, as measured by the soil sampler
    /// </summary>
    uint8_t soilMoisture;

    /// <summary>
    /// The current temperature, as measured by the soil sampler
    /// </summary>
    uint8_t temperature;

    /// <summary>
    /// The valve that determines, when a mister is turned on, which
    /// misting heads actually get water-flow. This determines which
    /// plants (and other objects) get watered when the mister has to
    /// be switched on.
    /// </summary>
    MistingValve* mistingValve;

    /// <summary>
    /// The minimum and maximum moisture levels measured during the
    /// current sketch "lifetime"
    /// </summary>
    uint8_t minH2O, maxH2O;
    
    /// <summary>
    /// The allowable moisture ranges, for normal settings
    /// </summary>
    RangedValueU8 moistureRange;

#pragma endregion

public:
#pragma region Ctors

    /// <summary>
    /// The only allowable constructor for a SoilMonitor device.
    /// </summary>
    /// <param name="mistingValvePin">The pin that this particular misting valve is connected to.</param>
    /// <param name="soilSensorPin">The pin that this particular soil sensor is connected to.</param>
    /// <param name="_name">The name of this soil-sensor/misting-head.</param>
    SoilMonitor(/*uint8_t mistingValvePin,*/ MistingValve& _pMistingValve, const char* _name) :
        //: MistingValve(mistingValvePin, _name),
            mistingValve(&_pMistingValve),
            moistureRange(0, adafruitLowerBound, adafruitUpperBound)
    {
    }

#pragma endregion

    virtual uint8_t Initialize()
    {
        if (!soilSampler.begin(0x36))
        {
            Serial.println("Soil sensor not setup properly!");
            return -1;
        }

        Serial.print("Seesaw soil sampler properly setup! Version: ");
        Serial.println(soilSampler.getVersion(), HEX);

        return 0;
    }

    virtual uint8_t Update()
    {
        UpdateSerial();
        return ReadSample();
    }

    uint8_t GetSoilMoisture() const
    {
        return soilMoisture;
    }

    uint8_t GetSoilTemperature() const
    {
        return temperature;
    }

protected:

    bool ReadSample()
    {
        float tempMVar = soilSampler.touchRead(0);

#ifdef DEBUG_VALUES
        //according to adafruit (https://www.adafruit.com/product/4026):
        //"We use the built in capacitive touch measurement system built into the ATSAMD10 chip,
        // which will give you a reading ranging from about 200 (very dry) to 2000 (very wet)."
        if (tempMVar < adafruitLowerBound)     
        {
            Serial.print("Moisture Level < ");
            Serial.print(adafruitLowerBound);
            Serial.print("!!, Value=");
            Serial.println(tempMVar);
        }
        else if (tempMVar > adafruitUpperBound)
        {
            Serial.print("Moisture Level > ");
            Serial.print(adafruitUpperBound);
            Serial.print("!!, Value=");
            Serial.println(tempMVar);
        }
#endif
        
        //Ideally, this converts a number from the range of [200-2000](~ [1-10]) to a percentage, "as written" -- i.e. 1.0 == 1% 
        soilMoisture = map(tempMVar, adafruitLowerBound, adafruitUpperBound, 0, 100);
        temperature = soilSampler.getTemp();

        //convert to F
        temperature = (temperature * (9.0f / 5.0f)) + 32.0f;

#ifdef DEBUG_VALUES
        Serial.print("Soil Moisture: ");
        Serial.println(soilMoisture);
        Serial.print("Soil temperature: ");
        Serial.println(temperature);
#endif

        return true;
    }

    void UpdateSerial()
    {
        //Do we need to monitor the soil moisture sensor temperature?
        //More data never hurts, but the complexity?
        /*if (temperature < minTemp)
        {
            Serial.print("***New min temp: "); Serial.println(temperature);
            minTemp = temperature;
        }

        if (temperature > maxTemp)
        {
            Serial.print("***New max temp: "); Serial.println(temperature);
            maxTemp = temperature;
        }*/

        //If the soil is too dry, set new "lowest moisture"
        if (soilMoisture < moistureRange.GetMinValue())
        {
            Serial.print("***New min H2O: "); Serial.println(soilMoisture);
            minH2O = soilMoisture;
        }

        //If the soil is too wet, set new "highest moisture"
        if (soilMoisture > moistureRange.GetMaxValue())
        {
            Serial.print("***New max H2O: "); Serial.println(soilMoisture);
            maxH2O = soilMoisture;
        }

#ifdef DEBUG_VALUES
        Serial.print("Current min H2O: ");
        Serial.println(minH2O);
        Serial.print("Current max H2O: ");
        Serial.println(maxH2O);

        /*Serial.print("Current min temp: ");
        Serial.println(minTemp);
        Serial.print("Current max temp: ");
        Serial.println(maxTemp);*/
#endif
    }
};

#endif

