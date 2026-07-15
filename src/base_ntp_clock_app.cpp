#include "base_ntp_clock_app.h"

#include "base_preferences.h"
#include "base_access_point_manager.h"
#include "clock_fsm_manager.h"
#include "scene_manager.h"
#include "boot_manager.h"

#include "logging.h"

#include "esp_timer.h"

// Double-reset detection window. A second reset within this many
// microseconds of boot requests AP mode; after it elapses the boot
// counts as stable and the flag is cleared.
static constexpr int64_t DOUBLE_RESET_WINDOW_US = 10 * 1000 * 1000;

BaseNtpClockApp::BaseNtpClockApp()
    : _prefs(nullptr), _apManager(nullptr) {}

BaseNtpClockApp::~BaseNtpClockApp() = default;

void BaseNtpClockApp::setup() {
    // Preferences first, so setupHardware() sees the saved brightness /
    // log-level / etc.
    if (_prefs) {
        _prefs->setup();
        _prefs->getPreferences();
        _prefs->dumpPreferences();

        // Propagate loaded log level to the global, and raise esp_log's
        // per-tag level to match — LOGDBG maps to ESP_LOGD, which the
        // IDF-side runtime filter drops at its default INFO level even
        // when g_appLogLevel says DEBUG.
        extern AppLogLevel g_appLogLevel;
        g_appLogLevel = _prefs->getConfig().logLevel;
        esp_log_level_set(LOG_TAG, g_appLogLevel >= APP_LOG_DEBUG
                                       ? ESP_LOG_DEBUG : ESP_LOG_INFO);
    }

    // App-specific hardware init (SPI, display, I2C...).
    setupHardware();

    // Engine managers. The FSM and scene manager need a reference back
    // to this->IBaseClock, which is available because we are already
    // inside a concrete subclass at this point in the chain.
    _bootManager  = std::make_unique<BootManager>();
    _fsmManager   = std::make_unique<ClockFsmManager>(*this);
    _sceneManager = std::make_unique<SceneManager>(*this);

    // If the user double-tapped reset, skip the happy path and jump to
    // the captive portal before even trying to connect to WiFi.
    if (_bootManager->checkForForceAPMode()) {
        _fsmManager->requestApMode();
    }

    _fsmManager->setup();

    LOGINF("Base clock engine setup complete");
}

void BaseNtpClockApp::loop() {
    if (_fsmManager)   _fsmManager->update();
    if (_sceneManager) _sceneManager->update();

    // Clear the "recent boot" flag once the double-reset window has
    // elapsed, or as soon as the FSM commits to AP mode — whichever
    // comes first. Clearing used to wait for RUNNING_NORMAL, which
    // meant any boot that never got there (WiFi outage, NTP timeout,
    // crash) left the flag armed and forced every subsequent power-up
    // straight back into AP mode. The AP_MODE case matters because
    // the captive portal loop blocks forever; this is the last loop()
    // pass that can run before it does.
    static bool s_bootMarkedStable = false;
    if (_bootManager && !s_bootMarkedStable) {
        bool windowElapsed = esp_timer_get_time() >= DOUBLE_RESET_WINDOW_US;
        bool committedToAp = _fsmManager && _fsmManager->isInState("AP_MODE");
        if (windowElapsed || committedToAp) {
            s_bootMarkedStable = true;
            _bootManager->markBootStable();
        }
    }
}
