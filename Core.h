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


#ifndef __CORE_H
#define __CORE_H

#include <cstdint>
#include "StepperDrive.h"
#include "Encoder.h"
#include "ControlPanel.h"
#include "Tables.h"
#include "multicore.h"

#define NULL_FEED 0.0

typedef struct {
    bool isAlarm;
    bool powerOn;
    float rpm;
} corestatus_t;

extern queue_t feed_queue; // TODO: maybe pass this into constructor instead
extern queue_t corestatus_queue; // TODO: maybe pass this into constructor instead
extern queue_t poweron_queue; // TODO: maybe pass this into constructor instead
//extern int doorbell_core_command; // TODO: maybe pass this into constructor instead
extern int doorbell_core_status; // TODO: maybe pass this into constructor instead

class Core
{
private:
    Encoder *encoder;
    StepperDrive *stepperDrive;

    float feed;
    float previousFeed;

    int16_t feedDirection;
    int16_t previousFeedDirection;

    int32_t previousSpindlePosition;

    int32_t feedRatio(int32_t count);

    corestatus_t status;

public:
    Core( Encoder *encoder, StepperDrive *stepperDrive );

    void setFeed(float);
    void setReverse(bool reverse);

    void setPowerOn(bool);

    void pollStatus();
    void checkQueues();

    void ISR( void );
};

inline void Core :: setFeed(float feed)
{
    this->feed = feed;
}

inline void Core :: pollStatus( void )
{
    status.rpm = encoder->getRPM();
    status.isAlarm = stepperDrive->isAlarm();
    bool rv = queue_try_add(&corestatus_queue, &status);
    multicore_doorbell_set_other_core(doorbell_core_status);
}

inline int32_t Core :: feedRatio(int32_t count)
{
    return ((float)count) * this->feed * feedDirection;
}

#endif // __CORE_H
