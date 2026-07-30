// anim_split_flap.h — electromechanical split-flap board (airport/train
// departure display) style reveal.
//
// All cells start blank and step through the printable-ASCII range in
// lockstep, one character per tick, each cell independently stopping the
// moment it reaches its own target character — unlike SlotMachineAnimation,
// nothing is ever forced to the target; the cell is simply already showing
// it once the cycle reaches that point. Cells whose target is a space are
// already correct at the start and never advance.

#pragma once

#include "i_animation.h"

#include <string>
#include <vector>
#include <cstdint>

class SplitFlapAnimation : public IAnimation {
public:
    SplitFlapAnimation(std::string targetText,
                       uint32_t stepDelay = 35,
                       bool dotsWithPreviousChar = false);

    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _targetText;
    bool        _dotsWithPreviousChar;

    std::string          _parsedText;
    std::vector<uint8_t> _dotStates;
    std::vector<char>    _currentChar;
    std::vector<bool>    _locked;

    uint32_t _stepDelay;
    uint32_t _lastStepTime;
    bool     _done;
};
