// anim_slider.h — one cell spins through random chars at a time; the lit
// cell then locks to its target and the cursor slides one position to the
// right, leaving revealed text behind it and dark (blank) cells ahead.

#pragma once

#include "i_animation.h"

#include <string>
#include <vector>
#include <cstdint>

class SliderAnimation : public IAnimation {
public:
    SliderAnimation(std::string targetText,
                    uint32_t dwellMs   = 250,
                    uint32_t spinDelay = 40,
                    bool dotsWithPreviousChar = false);

    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _targetText;
    bool        _dotsWithPreviousChar;

    std::string          _parsedText;
    std::vector<uint8_t> _dotStates;

    uint32_t _dwellMs;     // how long the cursor spins at each cell
    uint32_t _spinDelay;   // how often the spinning cell's char refreshes

    int      _cursorPos;
    uint32_t _cursorStartTime;
    uint32_t _lastSpinTime;
    bool     _done;

    uint32_t _rngState;   // fast xorshift32 RNG
    char     randomChar();
};
