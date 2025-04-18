// Pico Electronic Leadscrew
// https://github.com/funkenjaeger/pico-els
//
// MIT License
//
// Copyright (c) 2025 Evan Dudzik
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

#ifndef __MULTICORE_H
#define __MULTICORE_H

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "Core.h"
#include "CrossCoreMessaging.h"

class CoreProxy : public Core
{
private:
    uint16_t rpm;
    bool isAlarm;
    bool powerOn;
    bool isPanic;
    CrossCoreMessaging* xCore;

public:
    CoreProxy( CrossCoreMessaging* );

    void setFeed(const FEED_THREAD*) override;
    void setReverse(bool) override;
    void setPowerOn(bool) override;
    void setDriveRatio(float) override;

    uint16_t getRPM(void) override;
    bool getIsAlarm() override;
    bool getIsPowerOn() override;
    bool getIsPanic() override;
    
    void checkStatus(void);
};

inline uint16_t CoreProxy :: getRPM(void) {
    return rpm;
}

inline bool CoreProxy :: getIsAlarm(void) {
    return isAlarm;
}

inline bool CoreProxy :: getIsPowerOn(void) {
    return powerOn;
}

inline bool CoreProxy :: getIsPanic(void) {
    return isPanic;
}

inline void CoreProxy :: checkStatus(void) {
    xCore->checkCoreStatus(&rpm, &isAlarm, &powerOn, &isPanic);
}

inline void CoreProxy :: setFeed(const FEED_THREAD* feed) {
    xCore->pushFeedCommand(feed);
}

inline void CoreProxy :: setReverse(bool reverse) {
    xCore->pushReverseCommand(reverse);
}

inline void CoreProxy :: setPowerOn(bool state) {
    xCore->pushPowerOnCommand(state);
}

inline void CoreProxy :: setDriveRatio(float driveRatio) {
    xCore->pushDriveRatioCommand(driveRatio);
}

#endif