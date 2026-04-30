# ESP32 NTP Clock engine component.
#
# IDF-native port of https://github.com/v01dma1n/ESP32NTPClock.
# Provides the base clock app (BaseNtpClockApp), the IDisplayDriver /
# IAnimation interfaces, the animation library, the FSM, the scene
# manager, NTP/geo helpers, and weather client.
#
# WiFi connectivity and NVS preferences live in the esp32_wifi component
# (ESP32WiFi2), which this component depends on.

idf_component_register(
    SRCS
        "src/base_ntp_clock_app.cpp"
        "src/clock_fsm_manager.cpp"
        "src/display_manager.cpp"
        "src/scene_manager.cpp"
        "src/sntp_client.cpp"
        "src/geo_tz_client.cpp"
        "src/weather_client.cpp"
        "src/anim_static_text.cpp"
        "src/anim_scrolling_text.cpp"
        "src/anim_slot_machine.cpp"
        "src/anim_matrix.cpp"
        "src/i_display_driver.cpp"
    INCLUDE_DIRS
        "include"
    REQUIRES
        esp32_wifi
        nvs_flash
        esp_wifi
        esp_http_client
        esp_netif
        esp_event
        esp_timer
        esp_hw_support
        lwip
        json
        driver
)

# Enable C++17 for std::unique_ptr / std::string_view / structured bindings.
target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++17)
