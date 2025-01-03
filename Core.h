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

#define USE_FLOATING_POINT

#ifdef USE_FLOATING_POINT
    #define NULL_FEED 0.0
#else
    #define NULL_FEED NULL
#endif


class Core
{
private:
    Encoder *encoder;
    StepperDrive *stepperDrive;

#ifdef USE_FLOATING_POINT
    float feed;
    float previousFeed;
#else
    const FEED_THREAD *feed;
    const FEED_THREAD *previousFeed;
#endif // USE_FLOATING_POINT

    int16_t feedDirection;
    int16_t previousFeedDirection;

    int32_t previousSpindlePosition;

    int32_t feedRatio(int32_t count);

    bool powerOn;

public:
    Core( Encoder *encoder, StepperDrive *stepperDrive );

    void setFeed(const FEED_THREAD *feed);
    void setReverse(bool reverse);
    uint16_t getRPM(void);
    bool isAlarm();

    bool isPowerOn();
    void setPowerOn(bool);

    void ISR( void );
};

inline void Core :: setFeed(const FEED_THREAD *feed)
{
#ifdef USE_FLOATING_POINT
    this->feed = (float)feed->numerator / feed->denominator;
#else
    this->feed = feed;
#endif // USE_FLOATING_POINT
}

inline uint16_t Core :: getRPM(void)
{
    return encoder->getRPM();
}

inline bool Core :: isAlarm()
{
    return this->stepperDrive->isAlarm();
}

inline bool Core :: isPowerOn()
{
    return this->powerOn;
}

inline int32_t Core :: feedRatio(int32_t count)
{
#ifdef USE_FLOATING_POINT
    return ((float)count) * this->feed * feedDirection;
#else // USE_FLOATING_POINT
    return ((int64_t)count * (int64_t)(feed->numerator) / (int64_t)(feed->denominator)) * feedDirection;
#endif // USE_FLOATING_POINT
}

#endif // __CORE_H
