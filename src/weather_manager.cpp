#include "weather_manager.h"
#include "logging.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

WeatherManager::WeatherManager(IWeatherClock& app)
    : _app(app), _data{}, _lastFetchMs(-(FETCH_INTERVAL_MS))
{}

void WeatherManager::update() {
    if (!_app.isOkToRunScenes()) return;

    int64_t nowMs = (int64_t)xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (nowMs - _lastFetchMs < FETCH_INTERVAL_MS) return;
    _lastFetchMs = nowMs;

    const char* apiKey = _app.getOwmApiKey();
    const char* city   = _app.getOwmCity();

    if (!apiKey || !*apiKey || !city || !*city) {
        LOGINF("OWM not configured — skipping weather fetch");
        // Explicitly invalidate rather than leaving _data untouched: if a
        // key was configured and fetched successfully at some point in
        // this boot and then cleared (or the city was blanked), _data
        // would otherwise keep reporting stale valid=true readings
        // forever, since update() only reruns fetchWeather() when both
        // fields are non-empty.
        _data.valid = false;
        return;
    }

    LOGINF("Fetching weather for: %s", city);
    _data = fetchWeather(apiKey, city);
}
