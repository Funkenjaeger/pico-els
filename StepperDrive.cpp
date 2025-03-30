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


#include "StepperDrive.h"


StepperDrive :: StepperDrive(void)
{
    //
    // Set up global state variables
    //
    currentPosition = 0;
    desiredPosition = 0;
}

void StepperDrive :: initHardware(void)
{    
    gpio_init(STEPPER_DIRECTION_PIN);
    gpio_init(STEPPER_ENABLE_PIN);
    gpio_init(STEPPER_ALARM_PIN);
    gpio_put(STEPPER_DIRECTION_PIN, false);
    gpio_put(STEPPER_ENABLE_PIN, false);
    gpio_set_dir(STEPPER_DIRECTION_PIN, GPIO_OUT);
    gpio_set_dir(STEPPER_ENABLE_PIN, GPIO_OUT);
    gpio_set_dir(STEPPER_ALARM_PIN, GPIO_IN);
    gpio_pull_up(STEPPER_ALARM_PIN);

    // stepper pio
    pio = pio0;
    pio_sm = pio_claim_unused_sm(pio, true);
    int stm_offset = pio_add_program(pio, &stepper_program);
    stepper_program_init(pio, pio_sm, stm_offset, STEPPER_STEP_PIN, 6e6);
    #ifdef INVERT_STEP_PIN
        gpio_set_outover(STEPPER_STEP_PIN, GPIO_OVERRIDE_INVERT);
    #endif
    #ifdef INVERT_DIRECTION_PIN
        gpio_set_outover(STEPPER_DIRECTION_PIN, GPIO_OVERRIDE_INVERT);
    #endif
    #ifdef INVERT_ENABLE_PIN
        gpio_set_outover(STEPPER_ENABLE_PIN, GPIO_OVERRIDE_INVERT);
    #endif
    #ifdef INVERT_ALARM_PIN
        gpio_set_inover(STEPPER_ALARM_PIN, GPIO_OVERRIDE_INVERT);
    #endif

    setEnabled(true);
}






