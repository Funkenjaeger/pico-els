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

#define USE_MULTICORE

#include <stdio.h>
#include <cstdint>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"

#include "ControlPanel.h"
#include "StepperDrive.h"
#include "Encoder.h"
#include "Core.h"
#include "UserInterface.h"
#include "Gearbox.h"
#include "SanityCheck.h"

#ifdef USE_MULTICORE
#include "CoreProxy.h"
#include "CrossCoreMessaging.h"
#include "MulticoreCore.h"
#endif

#include "blink.pio.h"

#ifdef USE_MULTICORE
bool core1_core_motion_timer_callback( repeating_timer*);
bool core1_status_timer_callback( repeating_timer*);
bool pollcorestatus_timer_callback( repeating_timer*);
void core1_entry(void);

void doorbell_core0_isr(void);
void doorbell_core1_isr(void);

repeating_timer core1_core_motion_timer;
repeating_timer core1_status_timer;
#else
bool core_motion_timer_callback( repeating_timer*);
repeating_timer core_motion_timer;
#endif

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

// Gearbox
Gearbox* gearbox;

#ifdef USE_MULTICORE
CrossCoreMessaging* xCore;
// Core engine
MulticoreCore* core;
CoreProxy* coreProxy;
#else
// Core engine
Core* core;
#endif

// User Interface
UserInterface* userInterface;

int main()
{
    stdio_init_all();

    // PIO Blink
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    int32_t pio_sm = pio_claim_unused_sm(pio, true);    
    blink_pin_forever(pio, pio_sm, offset, PICO_DEFAULT_LED_PIN, 3);

    #ifdef USE_MULTICORE
    xCore = new CrossCoreMessaging();

    // Set up doorbell interrupt handlers
    uint32_t irq = xCore->getDoorbellIrqNum();
    irq_add_shared_handler(irq, doorbell_core0_isr, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY );
    irq_add_shared_handler(irq, doorbell_core1_isr, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY );
    // Unmask doorbell IRQ in Core 0
    irq_set_enabled(irq, true); 
    #endif    

    // Instantiate objects
    feedTableFactory = new FeedTableFactory();
    spiBus = new SPIBus();
    controlPanel = new ControlPanel(spiBus);
    encoder = new Encoder();
    stepperDrive = new StepperDrive();
    gearbox = new Gearbox();

    #ifdef USE_MULTICORE
    core = new MulticoreCore(encoder, stepperDrive, xCore);
    coreProxy = new CoreProxy(xCore);
    userInterface = new UserInterface(controlPanel, coreProxy, feedTableFactory, gearbox);
    #else
    core = new Core(encoder, stepperDrive);
    userInterface = new UserInterface(controlPanel, core, feedTableFactory);
    #endif    

    // Initialize peripherals and pins
    stepperDrive->initHardware();  
    encoder->initHardware();    
    spiBus->initHardware();  
    controlPanel->initHardware(); 

    #ifdef USE_MULTICORE
    multicore_launch_core1(core1_entry);  
    #else
    add_repeating_timer_us(STEPPER_CYCLE_US, core_motion_timer_callback, NULL, &core_motion_timer);
    #endif

    printf("Initialized...\n");

    while (true) {
        // check for step backlog and panic the system if it occurs
        if( coreProxy->getIsPanic() ) {
            userInterface->panicStepBacklog();
        }

        // service the user interface
        userInterface->loop();

        // delay
        sleep_us(1000000 / UI_REFRESH_RATE_HZ);
    }
}

#ifdef USE_MULTICORE
void core1_entry(void)
{   
    // Create alarm pool for Core 1 (default alarm pool always interrupts Core 0)
    alarm_pool_t* core1_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(16);
    alarm_pool_add_repeating_timer_us(core1_alarm_pool, 1000, core1_status_timer_callback, NULL, &core1_status_timer);
    alarm_pool_add_repeating_timer_us(core1_alarm_pool, STEPPER_CYCLE_US, core1_core_motion_timer_callback, NULL, &core1_core_motion_timer);  

    // Unmask doorbell IRQ in Core 1
    irq_set_enabled(xCore->getDoorbellIrqNum(), true);  

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
        coreProxy->checkStatus(); 
    }
}

void doorbell_core1_isr(void) {
    if(get_core_num() == 1) {
        core->checkQueues();
    } 
}
#else
bool core_motion_timer_callback( repeating_timer *rt )
{
    core->ISR();
    return true;
}
#endif
