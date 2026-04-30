// ESP32NTPClock.h — umbrella header for the MoodWhisperer clock engine.
//
// Matches the Arduino library's umbrella header so app code can port
// with minimal changes. Apps just:
//
//   #include "ESP32NTPClock.h"
//
// and get the full interface surface — including the WiFi/preferences
// layer from ESP32WiFi2.

#pragma once

#include "ESP32WiFi2.h"   // wifi_types, logging, base_preferences,
                          // base_access_point_manager, wifi_connector,
                          // boot_manager, tz_data

#include "enc_types.h"
#include "i_display_driver.h"
#include "i_animation.h"
#include "i_base_clock.h"

#include "base_ntp_clock_app.h"

#include "clock_fsm_manager.h"
#include "scene_manager.h"
#include "display_manager.h"

#include "wifi_connector.h"
#include "sntp_client.h"
#include "tz_data.h"
#include "geo_tz_client.h"

#include "anim_utils.h"
#include "anim_time.h"
#include "anim_static_text.h"
#include "anim_scrolling_text.h"
#include "anim_slot_machine.h"
#include "anim_matrix.h"

#include "quote_manager.h"
