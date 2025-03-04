// Pico Electronic Leadscrew
// https://github.com/funkenjaeger/pico-els
//
// MIT License
//
// Copyright (c) 2025 Evan Dudzik
//
// This software is based on the Clough42 Electronic Leadscrew project under the MIT license
// https://github.com/clough42/electronic-leadscrew
// Leveraged portions of this software are Copyright (c) 2019 James Clough
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#ifndef __GEARBOX_H
#define __GEARBOX_H

#include "ControlPanel.h"

enum GearboxDirection {
    FORWARD,
    REVERSE
};

enum GearboxFeedThread {
    FEED,
    THREAD
};

enum GearboxGear : int {
    A = 0,
    B = 1,
    C = 2
};

const MESSAGE GEAR_MESSAGE[3] = {
    {
        .message = { BLANK, LETTER_G, LETTER_E, LETTER_A, LETTER_R, BLANK, LETTER_A, BLANK },
        .displayTime = uint16_t(UI_REFRESH_RATE_HZ * 1.5)
    }, 
    {
        .message = { BLANK, LETTER_G, LETTER_E, LETTER_A, LETTER_R, BLANK, LETTER_B, BLANK },
        .displayTime = uint16_t(UI_REFRESH_RATE_HZ * 1.5)
    },
    {
        .message = { BLANK, LETTER_G, LETTER_E, LETTER_A, LETTER_R, BLANK, LETTER_C, BLANK },
        .displayTime = uint16_t(UI_REFRESH_RATE_HZ * 1.5)
    }
};

typedef struct {
    GearboxDirection direction;
    GearboxFeedThread feed_thread;
    GearboxGear gear;
    float finalDriveRatio;
} GearboxState;

class Gearbox
{
private:
    GearboxState state;

public:
    Gearbox(void);
    bool getState(GearboxState*);
};

#endif // __GEARBOX_H