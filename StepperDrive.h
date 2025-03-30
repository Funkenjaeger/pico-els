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


#ifndef __STEPPERDRIVE_H
#define __STEPPERDRIVE_H

#include <cmath>
#include <algorithm>
#include <cstdint>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "Configuration.h"
#include "hardware/pio.h"
#include "stepper.pio.h"


class StepperDrive
{
private:
    //
    // Current position of the motor, in steps
    //
    int32_t currentPosition;

    //
    // Desired position of the motor, in steps
    //
    int32_t desiredPosition;

    bool previousDir;

    //
    // Is the drive enabled?
    //
    bool enabled;

    PIO pio;
    uint32_t pio_sm;

public:
    StepperDrive();
    void initHardware(void);

    void setDesiredPosition(int32_t steps);
    void incrementCurrentPosition(int32_t increment);
    void setCurrentPosition(int32_t position);

    bool checkStepBacklog();

    void setEnabled(bool);

    bool isAlarm();

    void move(void);
    bool busy(void);
};

inline void StepperDrive :: setDesiredPosition(int32_t steps)
{
    this->desiredPosition = steps;
}

inline void StepperDrive :: incrementCurrentPosition(int32_t increment)
{
    this->currentPosition += increment;
}

inline void StepperDrive :: setCurrentPosition(int32_t position)
{
    this->currentPosition = position;
}

inline bool StepperDrive :: checkStepBacklog()
{
    if( abs(this->desiredPosition - this->currentPosition) > MAX_BUFFERED_STEPS ) {
        setEnabled(false);
        return true;
    }
    return false;
}

inline void StepperDrive :: setEnabled(bool enabled)
{
    this->enabled = enabled;
    gpio_put(STEPPER_ENABLE_PIN, enabled);
}

inline bool StepperDrive :: isAlarm()
{
#ifdef USE_ALARM_PIN
    return gpio_get(STEPPER_ALARM_PIN);
#else
    return false;
#endif
}

inline bool StepperDrive :: busy(void) {
    return !pio_interrupt_get(pio, 0);
}

inline void StepperDrive :: move(void)
{
    if(enabled) {
        int32_t delta = desiredPosition - currentPosition;
        uint32_t stepsToTake = std::min(abs(delta),32);
        bool dir = delta > 0;
        
        if(stepsToTake != 0 && !busy()) {
            if(dir != previousDir){
                gpio_put(STEPPER_DIRECTION_PIN, dir);
                previousDir = dir;
                busy_wait_us(STEPPER_CYCLE_US); 
            }
            pio_sm_put_blocking(pio, pio_sm, (uint32_t)0xFFFFFFFF >> (uint32_t)(32-stepsToTake));
            currentPosition += stepsToTake * (dir ? 1 : -1);
        }
    } else {
        // not enabled; just keep current position in sync
        this->currentPosition = this->desiredPosition;
    }
}

#endif // __STEPPERDRIVE_H
