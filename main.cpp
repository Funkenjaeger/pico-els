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

bool core1_core_motion_timer_callback( repeating_timer*);
bool core1_status_timer_callback( repeating_timer*);
bool pollcorestatus_timer_callback( repeating_timer*);
void core1_entry(void);

int doorbell_core_status;
int doorbell_core_command;

void doorbell_core0_isr(void);
void doorbell_core1_isr(void);

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
// Declare all of the main ELS components and connect them together
//

// Feed table factory
FeedTableFactory* feedTableFactory;

// Common SPI Bus driver (used for Control Panel)
SPIBus* spiBus;

// Control Panel driver
ControlPanel* controlPanel;

// Encoder driver
Encoder* encoder;

// Stepper driver
StepperDrive* stepperDrive;

// Core engine
Core* core;
CoreProxy* coreProxy;

// User Interface
UserInterface* userInterface;

repeating_timer core1_core_motion_timer;
repeating_timer core1_status_timer;

int main()
{
    stdio_init_all();

    // PIO Blink
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    int32_t pio_sm = pio_claim_unused_sm(pio, true);    
    blink_pin_forever(pio, pio_sm, offset, PICO_DEFAULT_LED_PIN, 3);

    // Set up queues & doorbells for interfacing between cores
    queue_init(&feed_queue, sizeof(float), 2);
    queue_init(&poweron_queue, sizeof(bool), 2);
    queue_init(&reverse_queue, sizeof(bool), 2);
    queue_init(&corestatus_queue, sizeof(corestatus_t), 2);    
    doorbell_core_command = multicore_doorbell_claim_unused((1 << NUM_CORES) - 1, true);
    doorbell_core_status = multicore_doorbell_claim_unused((1 << NUM_CORES) - 1, true);

    // Set up doorbell interrupt handlers
    uint32_t irq = multicore_doorbell_irq_num(doorbell_core_status);
    irq_add_shared_handler(irq, doorbell_core0_isr, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY );
    irq_add_shared_handler(irq, doorbell_core1_isr, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY );
    irq_set_enabled(irq, true);  

    // Instantiate objects
    feedTableFactory = new FeedTableFactory();
    spiBus = new SPIBus();
    controlPanel = new ControlPanel(spiBus);
    encoder = new Encoder();
    stepperDrive = new StepperDrive();
    core = new Core(encoder, stepperDrive);
    coreProxy = new CoreProxy();
    userInterface = new UserInterface(controlPanel, coreProxy, feedTableFactory);

    // Initialize peripherals and pins
    stepperDrive->initHardware();  
    encoder->initHardware();    
    spiBus->initHardware();  
    controlPanel->initHardware(); 

    multicore_launch_core1(core1_entry);  

    while (true) {
        // check for step backlog and panic the system if it occurs
        if( stepperDrive->checkStepBacklog() ) { // TODO: do this check on core1, this isn't safe
            userInterface->panicStepBacklog();
        }

        // service the user interface
        userInterface->loop();

        // delay
        sleep_us(1000000 / UI_REFRESH_RATE_HZ);
    }
}

void core1_entry(void)
{   
    // Create alarm pool for Core 1 (default alarm pool always interrupts Core 0)
    alarm_pool_t* core1_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(16);
    alarm_pool_add_repeating_timer_us(core1_alarm_pool, 1000, core1_status_timer_callback, NULL, &core1_status_timer);
    alarm_pool_add_repeating_timer_us(core1_alarm_pool, STEPPER_CYCLE_US, core1_core_motion_timer_callback, NULL, &core1_core_motion_timer);  

    // Unmask core1 doorbell interrupt
    uint32_t irq = multicore_doorbell_irq_num(doorbell_core_command);
    irq_set_enabled(irq, true);  

    while(true) {
        tight_loop_contents();
    }
}

bool core1_status_timer_callback( repeating_timer *rt )
{
    core->pollStatus();
    return true;
}

bool core1_core_motion_timer_callback( repeating_timer *rt )
{
    core->ISR();
    return true;
}

// Note: due to rp2350 only having a single IRQ# for doorbells, 
// both of the following ISRs will be called upon doorbell interrupt - 
// but only on the core that received the doorbell interrupt
void doorbell_core0_isr(void) {    
    if(get_core_num() == 0) {
        multicore_doorbell_clear_current_core(doorbell_core_status);
        coreProxy->checkStatus(); 
    }
}

void doorbell_core1_isr(void) {
    if(get_core_num() == 1) {
        multicore_doorbell_clear_current_core(doorbell_core_command);
        core->checkQueues();
    } 
}
