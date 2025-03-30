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

#include "CrossCoreMessaging.h"

CrossCoreMessaging :: CrossCoreMessaging( void ) {
    // Set up queues & doorbells for interfacing between cores
    queue_init(&feed_queue, sizeof(FEED_THREAD), 4);
    queue_init(&poweron_queue, sizeof(bool), 4);
    queue_init(&reverse_queue, sizeof(bool), 4);
    queue_init(&corestatus_queue, sizeof(corestatus_t), 4);
    queue_init(&driveratio_queue, sizeof(float), 4);  
    doorbell_core_command = multicore_doorbell_claim_unused((1 << NUM_CORES) - 1, true);
    doorbell_core_status = multicore_doorbell_claim_unused((1 << NUM_CORES) - 1, true);
}

bool CrossCoreMessaging :: checkCoreStatus( uint16_t *rpm, bool *isAlarm, bool *powerOn, bool *isPanic ) {
    
    corestatus_t coreStatus;
    if(queue_try_remove(&corestatus_queue, &coreStatus)) {
        *rpm = coreStatus.rpm;
        *isAlarm = coreStatus.isAlarm;
        *powerOn = coreStatus.powerOn;
        *isPanic = coreStatus.isPanic;
        if (queue_is_empty(&corestatus_queue)) {
            multicore_doorbell_clear_current_core(doorbell_core_status);
        }
        return true;
    } else {
        return false;
    }
}