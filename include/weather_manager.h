#pragma once

#include "weather_client.h"
#include "i_weather_clock.h"

#include <cstdint>

// Polls OpenWeatherMap every 15 minutes on behalf of any IWeatherClock app.
class WeatherManager {
public:
    explicit WeatherManager(IWeatherClock& app);
    void update();
    WeatherData getWeatherData() const { return _data; }

private:
    IWeatherClock& _app;
    WeatherData    _data;
    int64_t        _lastFetchMs;
    static constexpr int64_t FETCH_INTERVAL_MS = 15LL * 60 * 1000;
};
