# ESP32NTPClock2 — agent instructions

## Project
IDF-native NTP clock engine: SNTP, display/animation framework, scene manager,
weather client, and geo-TZ detection. Parallel to the Arduino ESP32NTPClock
library (https://github.com/v01dma1n/ESP32NTPClock).

This is a component library, not an application — there is no `main.cpp`
and no standalone `idf.py build` target. Build and test it inside a project
that has an IDF build system (e.g. MoodWhisperer at
https://github.com/v01dma1n/MoodWhisperer).

WiFi connectivity and NVS preferences live in the `esp32_wifi` component
(ESP32WiFi2, https://github.com/v01dma1n/ESP32WiFi2), which this component
depends on. Display drivers are supplied separately via ESP32NTPClockDrivers2
(https://github.com/v01dma1n/ESP32NTPClockDrivers2), which REQUIRES this
component for its IDisplayDriver interface.

## Using in an IDF project

Add both `esp32_wifi` (ESP32WiFi2) and `esp32_ntp_clock` (this repo) as git
submodules under `components/`, then declare `REQUIRES esp32_ntp_clock` in the
app component's CMakeLists.txt — `esp32_wifi` is pulled in transitively.
Include the umbrella header:

    #include "ESP32NTPClock.h"

For a weather-capable app also add `esp32_ntp_clock_drivers` (for the
display driver) and `esp32_ntp_clock` provides everything else transitively.

## Extension pattern

Apps subclass, not modify. Minimum viable clock:

    class MyApp : public BaseNtpClockApp {
        void setupHardware() override;   // init SPI/I2C, call display.begin()
        // IBaseClock pure virtuals:
        const char* getAppName()   const override { return "MyApp"; }
        const char* getSsid()      const override { return _prefs->getConfig().ssid; }
        const char* getPassword()  const override { return _prefs->getConfig().password; }
        const char* getTimezone()  const override { return _prefs->getConfig().timezone; }
        IDisplayDriver& getDisplay() override     { return _driver; }
        DisplayManager& getClock()   override     { return _displayMgr; }
        bool isOkToRunScenes() const override;
        void formatTime(...) override;
        void activateAccessPoint() override;
    };

Weather-capable apps additionally implement `IWeatherClock`:

    class MyApp : public BaseNtpClockApp, public virtual IWeatherClock { ... };

Preferences subclassing follows the same pattern as ESP32WiFi2:

    struct AppConfig : public BaseConfig { ... app fields ... };
    class  AppPreferences : public BasePreferences { ... overrides ... };
    class  AppApManager  : public BaseAccessPointManager {
        void initializeFormFields() override;   // call base first
    };

`initializeFormFields()` must call `BaseAccessPointManager::initializeFormFields()`
first so WiFi/TZ/logLevel fields are always at the top of the portal form.

## Timing

`anim_time.h` provides `app_millis()` as a drop-in for Arduino's `millis()`.
All animation and scene timing goes through `app_millis()`. Never call
`millis()` directly — it doesn't exist in IDF.

## Display threading

`DisplayManager` owns a FreeRTOS mutex. The normal pattern is two tasks:
- **AppTask** (main loop): calls `setAnimation()` to queue a new animation.
- **DisplayTask** (high-priority, Core 1): calls `update()` on the
  DisplayManager and `writeNextDigit()` on multiplexed drivers (MAX6921).

`DisplayManager::update()` is the only place that flushes to hardware and
it suppresses redundant SPI bursts when the frame hasn't changed. Don't
call `writeDisplay()` on the driver directly from application code.

## Types

Clock/display types (`DisplayScene`, `AnimationType`) live in `enc_types.h`.
WiFi/preferences types (`FormField`, `PrefSelectOption`, `AppLogLevel`,
`PrefType`, `FieldValidation`) live in `wifi_types.h` (ESP32WiFi2) and are
re-exported via `enc_types.h`'s `#include "wifi_types.h"`.
`IDisplayDriver`, `IAnimation`, `IBaseClock`, `IWeatherClock` are each in
their own header under `include/`.

`UNSET_VALUE` (-999.0f) is the sentinel `getDataValue()` returns when data
is not yet available; the scene manager renders `---` in that case.

## Comments and logs

Match the existing casual, specific voice. Use `LOGINF` / `LOGERR` /
`LOGDBG` from `logging.h`. No emoji, no marketing words.

## Do not
- Don't add `#include <Arduino.h>`, `millis()`, `delay()`, `SimpleFSM`,
  `RTC_DS1307`, `Wire.h`, or any other Arduino dependency.
- Don't add display driver implementations here. Drivers (HT16K33, MAX6921,
  HCS12SS59T, PT6315) live in ESP32NTPClockDrivers2.
- Don't add I2C or SPI bus initialization here. Each driver initializes its
  own bus in its `begin()`.
- Don't change `MAX_SCENE_TEXT_LEN` (64) without auditing every format
  string in every app — overflow silently truncates on the display.
- Don't change `MAX_PREF_STRING_LEN` without coordinating across every app
  that has existing NVS entries — old strings would be silently truncated.
- Don't move WiFi/prefs types out of `wifi_types.h` (ESP32WiFi2); they are
  referenced by both layers and any app that uses the captive portal form.
- Don't touch `.git/` or rewrite history.
