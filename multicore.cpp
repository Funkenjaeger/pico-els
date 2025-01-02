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

#include "multicore.h"

queue_t feed_queue;
queue_t poweron_queue;
queue_t reverse_queue;
queue_t corestatus_queue;

CoreProxy :: CoreProxy ( void ) {

}

void CoreProxy :: setFeed(const FEED_THREAD *feed) {
    float _feed = (float)feed->numerator / (float) feed->denominator;
    queue_add_blocking(&feed_queue, &_feed);
}

void CoreProxy :: setReverse(bool reverse) {
    queue_add_blocking(&reverse_queue, &reverse);
}

uint16_t CoreProxy :: getRPM(void) {
    return _rpm;
}

bool CoreProxy :: isAlarm(void) {
    return _isAlarm;
}

bool CoreProxy :: isPowerOn(void) {
    return _powerOn;
}

void CoreProxy :: setPowerOn(bool state) {
    queue_add_blocking(&poweron_queue, &state);
}

void CoreProxy :: checkStatus(void) {
    corestatus_t entry;
    if(queue_try_remove(&corestatus_queue, &entry)) {
        _rpm = entry.rpm;
        _isAlarm = entry.isAlarm;
        _powerOn = entry.powerOn;
    }
}