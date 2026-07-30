// anim_slot_machine.h — each cell independently redraws a random char every
// tick and locks the moment its own draw happens to match the target — a
// real slot-machine reel, not a scripted left-to-right sweep. Since pure
// chance has no upper bound, any cell that hasn't matched by maxSpinMs is
// force-locked so the whole reveal still finishes in bounded time.

#pragma once

#include "i_animation.h"

#include <string>
#include <vector>
#include <cstdint>

class SlotMachineAnimation : public IAnimation {
public:
    SlotMachineAnimation(std::string targetText,
                         uint32_t maxSpinMs = 800,
                         uint32_t spinDelay = 50,
                         bool dotsWithPreviousChar = false);

    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _targetText;
    std::string _parsedText;
    std::vector<uint8_t> _dotStates;
    std::vector<bool>    _locked;

    uint32_t _maxSpinMs;   // safety net: force-lock stragglers past this
    uint32_t _spinDelay;
    bool     _dotsWithPreviousChar;

    uint32_t _startTime;
    uint32_t _lastSpinTime;
    bool     _done;

    uint32_t _rngState;   // fast xorshift32 RNG

    char randomChar();
};
