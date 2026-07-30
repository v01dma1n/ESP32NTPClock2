#include "anim_slider.h"
#include "anim_utils.h"
#include "anim_time.h"

#include "esp_random.h"

SliderAnimation::SliderAnimation(std::string targetText,
                                 uint32_t dwellMs,
                                 uint32_t spinDelay,
                                 bool dotsWithPreviousChar)
    : _targetText(std::move(targetText)),
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _dwellMs(dwellMs),
      _spinDelay(spinDelay),
      _cursorPos(0),
      _cursorStartTime(0),
      _lastSpinTime(0),
      _done(false),
      _rngState(esp_random() | 1u) {}

void SliderAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    parseTextAndDots(_targetText, _dotsWithPreviousChar,
                     _parsedText, _dotStates);

    int cells = _display->getDisplaySize();
    if (static_cast<int>(_parsedText.size()) < cells) {
        int pad = cells - _parsedText.size();
        _parsedText.append(pad, ' ');
        _dotStates.insert(_dotStates.end(), pad, 0);
    } else if (static_cast<int>(_parsedText.size()) > cells) {
        _parsedText.resize(cells);
        _dotStates.resize(cells);
    }

    // Blank the whole row up front. Unlike Matrix/SlotMachine, cells here
    // aren't re-touched every tick — only the cursor cell changes between
    // reveals — so anything ahead of the cursor needs a real blank
    // character now, not IAnimation::setup()'s raw zero word (which some
    // drivers would render as a NUL byte instead of a space).
    for (int i = 0; i < cells; ++i) setChar(i, ' ', false);

    _cursorPos = 0;
    uint32_t now = app_millis();
    _cursorStartTime = now;
    _lastSpinTime    = now;
    _done = (cells == 0);

    if (!_done) setChar(_cursorPos, randomChar(), false);
}

char SliderAnimation::randomChar() {
    uint32_t x = _rngState;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    _rngState = x;
    return static_cast<char>(0x30 + (x % ('Z' - '0' + 1)));
}

void SliderAnimation::update() {
    if (_done) return;

    uint32_t now  = app_millis();
    int      cells = static_cast<int>(_parsedText.size());

    if (now - _cursorStartTime >= _dwellMs) {
        // Reveal the real character as the cursor leaves this cell.
        setChar(_cursorPos, _parsedText[_cursorPos], _dotStates[_cursorPos] != 0);
        ++_cursorPos;
        _cursorStartTime = now;
        _lastSpinTime    = now;

        if (_cursorPos >= cells) {
            _done = true;
        } else {
            setChar(_cursorPos, randomChar(), false);
        }
        return;
    }

    if (now - _lastSpinTime >= _spinDelay) {
        setChar(_cursorPos, randomChar(), false);
        _lastSpinTime = now;
    }
}

bool SliderAnimation::isDone() { return _done; }
