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
#include <cstdint>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "Configuration.h"

#define GPIO_SET(pin) gpio_put(pin, true)
#define GPIO_CLEAR(pin) gpio_put(pin, false)

#ifdef INVERT_STEP_PIN
#define GPIO_SET_STEP GPIO_CLEAR(STEPPER_STEP_PIN)
#define GPIO_CLEAR_STEP GPIO_SET(STEPPER_STEP_PIN)
#else
#define GPIO_SET_STEP GPIO_SET(STEPPER_STEP_PIN)
#define GPIO_CLEAR_STEP GPIO_CLEAR(STEPPER_STEP_PIN)
#endif

#ifdef INVERT_DIRECTION_PIN
#define GPIO_SET_DIRECTION GPIO_CLEAR(STEPPER_DIRECTION_PIN)
#define GPIO_CLEAR_DIRECTION GPIO_SET(STEPPER_DIRECTION_PIN)
#else
#define GPIO_SET_DIRECTION GPIO_SET(STEPPER_DIRECTION_PIN)
#define GPIO_CLEAR_DIRECTION GPIO_CLEAR(STEPPER_DIRECTION_PIN)
#endif

#ifdef INVERT_ENABLE_PIN
#define GPIO_SET_ENABLE GPIO_CLEAR(STEPPER_ENABLE_PIN)
#define GPIO_CLEAR_ENABLE GPIO_SET(STEPPER_ENABLE_PIN)
#else
#define GPIO_SET_ENABLE GPIO_SET(STEPPER_ENABLE_PIN)
#define GPIO_CLEAR_ENABLE GPIO_CLEAR(STEPPER_ENABLE_PIN)
#endif

#ifdef INVERT_ALARM_PIN
#define GPIO_GET_ALARM (gpio_get(STEPPER_ALARM_PIN) == 0)
#else
#define GPIO_GET_ALARM (gpio_get(STEPPER_ALARM_PIN) != 0)
#endif


class StepperDrive
{
private:
    //
    // Current position of the motor, in steps
    //
    //int32_t currentPosition;

    //
    // Desired position of the motor, in steps
    //
    //int32_t desiredPosition;

    //
    // current state-machine state
    // bit 0 - step signal
    // bit 1 - direction signal
    //
    uint16_t state;

    //
    // Is the drive enabled?
    //
    bool enabled;

public:

    int32_t currentPosition; // DEBUG
    int32_t desiredPosition; // DEBUG

    StepperDrive();
    void initHardware(void);

    void setDesiredPosition(int32_t steps);
    void incrementCurrentPosition(int32_t increment);
    void setCurrentPosition(int32_t position);

    bool checkStepBacklog();

    void setEnabled(bool);

    bool isAlarm();

    void ISR(void);
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
    if( this->enabled ) {
        GPIO_SET_ENABLE;
    }
    else
    {
        GPIO_CLEAR_ENABLE;
    }
}

inline bool StepperDrive :: isAlarm()
{
#ifdef USE_ALARM_PIN
    return GPIO_GET_ALARM;
#else
    return false;
#endif
}


inline void StepperDrive :: ISR(void)
{
    if(enabled) {

        switch( this->state ) {

        case 0:
            // Step = 0; Dir = 0
            if( this->desiredPosition < this->currentPosition ) {
                GPIO_SET_STEP;
                this->state = 2;
            }
            else if( this->desiredPosition > this->currentPosition ) {
                GPIO_SET_DIRECTION;
                this->state = 1;
            }
            break;

        case 1:
            // Step = 0; Dir = 1
            if( this->desiredPosition > this->currentPosition ) {
                GPIO_SET_STEP;
                this->state = 3;
            }
            else if( this->desiredPosition < this->currentPosition ) {
                GPIO_CLEAR_DIRECTION;
                this->state = 0;
            }
            break;

        case 2:
            // Step = 1; Dir = 0
            GPIO_CLEAR_STEP;
            this->currentPosition--;
            this->state = 0;
            break;

        case 3:
            // Step = 1; Dir = 1
            GPIO_CLEAR_STEP;
            this->currentPosition++;
            this->state = 1;
            break;
        }

    } else {
        // not enabled; just keep current position in sync
        this->currentPosition = this->desiredPosition;
    }
}

#endif // __STEPPERDRIVE_H
