#include "anim_split_flap.h"
#include "anim_utils.h"
#include "anim_time.h"

namespace {
    // Full printable-ASCII wheel: covers digits, upper/lowercase letters
    // (e.g. "Jul" from %b), and the punctuation our scenes use (/, -, :,
    // .) without needing a hand-curated flap set — any target character
    // a future scene might use falls somewhere in this range.
    constexpr char WHEEL_MIN  = ' ';   // 0x20
    constexpr char WHEEL_MAX  = '~';   // 0x7E
    constexpr int  WHEEL_SIZE = WHEEL_MAX - WHEEL_MIN + 1;

    char nextWheelChar(char c) {
        int idx = (static_cast<int>(c) - WHEEL_MIN + 1) % WHEEL_SIZE;
        return static_cast<char>(WHEEL_MIN + idx);
    }
}

SplitFlapAnimation::SplitFlapAnimation(std::string targetText,
                                       uint32_t stepDelay,
                                       bool dotsWithPreviousChar)
    : _targetText(std::move(targetText)),
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _stepDelay(stepDelay),
      _lastStepTime(0),
      _done(false) {}

void SplitFlapAnimation::setup(IDisplayDriver* display) {
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

    _currentChar.assign(cells, WHEEL_MIN);
    _locked.assign(cells, false);

    bool anyUnlocked = false;
    for (int i = 0; i < cells; ++i) {
        setChar(i, WHEEL_MIN, false);   // start dark (blank)
        if (_parsedText[i] == WHEEL_MIN) {
            _locked[i] = true;          // already correct — never spins
        } else {
            anyUnlocked = true;
        }
    }

    _lastStepTime = app_millis();
    _done = !anyUnlocked;
}

void SplitFlapAnimation::update() {
    if (_done) return;

    uint32_t now = app_millis();
    if (now - _lastStepTime < _stepDelay) return;
    _lastStepTime = now;

    bool allLocked = true;
    int  cells = static_cast<int>(_parsedText.size());

    for (int i = 0; i < cells; ++i) {
        if (_locked[i]) continue;

        char next = nextWheelChar(_currentChar[i]);
        _currentChar[i] = next;

        if (next == _parsedText[i]) {
            _locked[i] = true;
            setChar(i, next, _dotStates[i] != 0);
        } else {
            setChar(i, next, false);
            allLocked = false;
        }
    }

    if (allLocked) _done = true;
}

bool SplitFlapAnimation::isDone() { return _done; }
