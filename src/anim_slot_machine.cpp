#include "anim_slot_machine.h"
#include "anim_utils.h"
#include "anim_time.h"

#include "esp_random.h"

SlotMachineAnimation::SlotMachineAnimation(std::string targetText,
                                           uint32_t maxSpinMs,
                                           uint32_t spinDelay,
                                           bool dotsWithPreviousChar)
    : _targetText(std::move(targetText)),
      _maxSpinMs(maxSpinMs),
      _spinDelay(spinDelay),
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _startTime(0),
      _lastSpinTime(0),
      _done(false),
      _rngState(esp_random() | 1u) {}

void SlotMachineAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    parseTextAndDots(_targetText, _dotsWithPreviousChar,
                     _parsedText, _dotStates);

    // Pad/truncate to the display width so each cell maps to one target.
    int cells = _display->getDisplaySize();
    if (static_cast<int>(_parsedText.size()) < cells) {
        int pad = cells - _parsedText.size();
        _parsedText.append(pad, ' ');
        _dotStates.insert(_dotStates.end(), pad, 0);
    } else if (static_cast<int>(_parsedText.size()) > cells) {
        _parsedText.resize(cells);
        _dotStates.resize(cells);
    }

    _locked.assign(cells, false);

    uint32_t now = app_millis();
    _startTime    = now;
    _lastSpinTime = now;
    _done = (cells == 0);
}

char SlotMachineAnimation::randomChar() {
    // xorshift32 — plenty random for this.
    uint32_t x = _rngState;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    _rngState = x;

    // Full printable ASCII (space through '~'). Since a cell only locks
    // when a draw actually matches its target, the pool has to cover
    // every character real scenes use — space, '/', '-', '.', lowercase
    // (e.g. "Jul") — not just digits/uppercase, or those targets could
    // never win by chance and would always fall back to maxSpinMs.
    constexpr char kMin = ' ';   // 0x20
    constexpr char kMax = '~';   // 0x7E
    constexpr int  kSize = kMax - kMin + 1;
    return static_cast<char>(kMin + (x % kSize));
}

void SlotMachineAnimation::update() {
    if (_done) return;

    uint32_t now = app_millis();
    if (now - _lastSpinTime < _spinDelay) return;
    _lastSpinTime = now;

    bool pastDeadline = (now - _startTime) >= _maxSpinMs;
    bool allLocked = true;

    for (size_t i = 0; i < _parsedText.size(); ++i) {
        if (_locked[i]) continue;

        char c = randomChar();
        if (c == _parsedText[i] || pastDeadline) {
            // Natural match, or the safety net ran out — either way this
            // cell is done; if it's the safety net, show the real target
            // instead of whatever the last random draw happened to be.
            setChar(static_cast<int>(i), _parsedText[i], _dotStates[i] != 0);
            _locked[i] = true;
        } else {
            setChar(static_cast<int>(i), c, false);
            allLocked = false;
        }
    }

    if (allLocked) _done = true;
}

bool SlotMachineAnimation::isDone() { return _done; }
