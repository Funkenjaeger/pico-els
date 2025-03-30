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

#ifndef __CROSSCOREMESSAGING_H
#define __CROSSCOREMESSAGING_H

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "Tables.h"

class CrossCoreMessaging
{
private:
    typedef struct {
        bool isAlarm;
        bool powerOn;
        uint16_t rpm;
        bool isPanic;
    } corestatus_t;

    bool commandQueuesEmpty(void);

public:
    CrossCoreMessaging(void);

    void pushFeedCommand(const FEED_THREAD*);
    void pushPowerOnCommand(bool);
    void pushReverseCommand(bool);
    void pushCoreStatus(uint16_t*, bool*, bool*, bool*);
    void pushGearRatioCommand(float);

    bool checkCoreStatus(uint16_t*, bool*, bool*, bool*);
    bool checkFeedCommand(FEED_THREAD*);
    bool checkPowerOnCommand(bool*);
    bool checkReverseCommand(bool*);
    bool checkGearRatioCommand(float*);

    uint getDoorbellIrqNum(void);

    queue_t feed_queue;
    queue_t poweron_queue;
    queue_t reverse_queue;
    queue_t corestatus_queue;
    queue_t gearratio_queue;
    int doorbell_core_command;
    int doorbell_core_status;
};

inline void CrossCoreMessaging :: pushFeedCommand( const FEED_THREAD *feed ) {
    queue_try_add(&feed_queue, feed);
    multicore_doorbell_set_other_core(doorbell_core_command);
}

inline void CrossCoreMessaging :: pushPowerOnCommand( bool powerOn ) {
    queue_try_add(&poweron_queue, &powerOn);
    multicore_doorbell_set_other_core(doorbell_core_command);
}

inline void CrossCoreMessaging :: pushReverseCommand( bool reverse ) {
    queue_try_add(&reverse_queue, &reverse);
    multicore_doorbell_set_other_core(doorbell_core_command);
}

inline void CrossCoreMessaging :: pushGearRatioCommand( float gearRatio ) {
    queue_try_add(&gearratio_queue, &gearRatio);
    multicore_doorbell_set_other_core(doorbell_core_command);
}

inline void CrossCoreMessaging :: pushCoreStatus( uint16_t *rpm, bool *isAlarm, bool *powerOn, bool *isPanic) {
    corestatus_t coreStatus = {};
    coreStatus.rpm = *rpm;
    coreStatus.isAlarm = *isAlarm;
    coreStatus.powerOn = *powerOn;
    coreStatus.isPanic = *isPanic;
    queue_try_add(&corestatus_queue, &coreStatus);
    multicore_doorbell_set_other_core(doorbell_core_status);
}

inline bool CrossCoreMessaging :: commandQueuesEmpty(void) {
    return queue_is_empty(&feed_queue) && 
        queue_is_empty(&poweron_queue) && 
        queue_is_empty(&reverse_queue);
}

inline bool CrossCoreMessaging :: checkFeedCommand(FEED_THREAD* feed) {
    bool rv = queue_try_remove(&feed_queue, feed);
    if(commandQueuesEmpty()) {
        multicore_doorbell_clear_current_core(doorbell_core_command);
    }
    return rv;
}

inline bool CrossCoreMessaging :: checkPowerOnCommand( bool* powerOn) {
    bool rv = queue_try_remove(&poweron_queue, powerOn);
    if(commandQueuesEmpty()) {
        multicore_doorbell_clear_current_core(doorbell_core_command);
    }
    return rv;
}

inline bool CrossCoreMessaging :: checkReverseCommand( bool* reverse) {
    bool rv = queue_try_remove(&reverse_queue, reverse);
    if(commandQueuesEmpty()) {
        multicore_doorbell_clear_current_core(doorbell_core_command);
    }
    return rv;
}

inline bool CrossCoreMessaging :: checkGearRatioCommand( float* gearRatio ) {
    bool rv = queue_try_remove(&gearratio_queue, gearRatio);
    if(commandQueuesEmpty()) {
        multicore_doorbell_clear_current_core(doorbell_core_command);
    }
    return rv;
}

inline uint CrossCoreMessaging :: getDoorbellIrqNum(void) {
    return multicore_doorbell_irq_num(doorbell_core_command);
}

#endif