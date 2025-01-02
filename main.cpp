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

#include <stdio.h>
#include <cstdint>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"

//#include "SanityCheck.h"
#include "ControlPanel.h"
#include "StepperDrive.h"
#include "Encoder.h"
#include "Core.h"
#include "UserInterface.h"
#include "multicore.h"

#include "blink.pio.h"

bool core_motion_timer_callback( repeating_timer*);
bool core1_timer_callback( repeating_timer*);
bool pollcorestatus_timer_callback( repeating_timer*);
void core1_entry(void);


void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

//
// DEPENDENCY INJECTION
//
// Declare all of the main ELS components and wire them together
//

// Feed table factory
FeedTableFactory feedTableFactory;

// Common SPI Bus driver
SPIBus spiBus;

// Control Panel driver
ControlPanel controlPanel(&spiBus);

// Encoder driver
Encoder encoder;

// Stepper driver
StepperDrive stepperDrive;

// Core engine
Core core(&encoder, &stepperDrive);
CoreProxy coreProxy;



repeating_timer core_motion_timer;
repeating_timer core1_timer;
repeating_timer coreproxy_timer;

int main()
{
    stdio_init_all();

    // PIO Blink
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    int32_t pio_sm = pio_claim_unused_sm(pio, true);    
    blink_pin_forever(pio, pio_sm, offset, PICO_DEFAULT_LED_PIN, 3);

    // Initialize peripherals and pins
    spiBus.initHardware();  
    controlPanel.initHardware();

    add_repeating_timer_us(500, pollcorestatus_timer_callback, NULL, &coreproxy_timer);    

    while (true) {
        // check for step backlog and panic the system if it occurs
        if( stepperDrive.checkStepBacklog() ) { // TODO: do this check on core1, this isn't safe
            userInterface.panicStepBacklog();
        }

        // service the user interface
        userInterface.loop();

        // delay
        sleep_us(1000000 / UI_REFRESH_RATE_HZ);
    }
}

void core1_entry(void)
{
    stepperDrive.initHardware();
    encoder.initHardware(); 
    add_repeating_timer_us(1000, core1_timer_callback, NULL, &core1_timer);
    add_repeating_timer_us(STEPPER_CYCLE_US, core_motion_timer_callback, NULL, &core_motion_timer);

    while(true) {

    }
}

bool core1_timer_callback( repeating_timer *rt )
{
    core.pollStatus();
    core.checkQueues();
    return true;
}

bool core_motion_timer_callback( repeating_timer *rt )
{
    core.ISR();
    return true;
}

bool pollcorestatus_timer_callback( repeating_timer *rt) {
    coreProxy.checkStatus();
    return true;
}
