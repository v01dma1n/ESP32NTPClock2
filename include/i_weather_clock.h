#pragma once

#include "i_base_clock.h"

// Optional interface layered on top of IBaseClock for apps that display
// weather data. The app provides the OWM credentials; WeatherManager
// handles polling. See i_base_clock.h for the IRtcClock analogue.
class IWeatherClock : public virtual IBaseClock {
public:
    virtual const char* getOwmApiKey() const = 0;
    // City query passed to OWM, e.g. "Chicago,IL,US" or "Warsaw,PL".
    // Apps with separate city/state/country fields compose the string here.
    virtual const char* getOwmCity() const = 0;
};
