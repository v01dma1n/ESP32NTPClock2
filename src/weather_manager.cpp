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
        return;
    }

    LOGINF("Fetching weather for: %s", city);
    _data = fetchWeather(apiKey, city);
}
