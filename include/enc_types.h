// enc_types.h — display-specific enums and structs used by the clock engine.
//
// WiFi/preferences types (FormField, PrefSelectOption, AppLogLevel, …) now
// live in ESP32WiFi2's wifi_types.h, which is included here so existing
// code that includes enc_types.h directly keeps compiling unchanged.

#pragma once

#include "wifi_types.h"   // re-exports FormField, AppLogLevel, PrefType, etc.

#include <cstdint>

// Marker value emitted by scene data getters when data is not (yet) valid.
// The scene manager substitutes a placeholder when it sees this.
static constexpr float UNSET_VALUE = -999.0f;

static constexpr int MAX_SCENE_TEXT_LEN = 64;

// The built-in animation types the engine ships with. Application code picks
// one per scene. Add new types by extending AnimationType *and* the switch
// in scene_manager.cpp.
enum AnimationType {
    STATIC_TEXT,
    SLOT_MACHINE,
    MATRIX,
    SCROLLING
};

// One entry in an application's scene playlist. The scene manager cycles
// through these and, for each one, calls getDataValue() to resolve %-tokens
// in format_string via strftime() (if it looks like a time format) or
// snprintf() (everything else).
struct DisplayScene {
    const char* scene_name;
    const char* format_string;
    AnimationType animation_type;
    bool dots_with_previous;   // "3.14" — dot attaches to previous glyph
    bool isLiveUpdate;         // re-render every tick (e.g. seconds)
    unsigned long duration_ms;
    unsigned long anim_param_1;
    unsigned long anim_param_2;
    float (*getDataValue)();   // may be nullptr for pure time-format scenes
};
