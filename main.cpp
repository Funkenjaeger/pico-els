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
#include "hardware/pio.h"
#include "hardware/uart.h"

//#include "SanityCheck.h"
#include "ControlPanel.h"
#include "StepperDrive.h"
#include "Encoder.h"
#include "Core.h"
#include "UserInterface.h"

#include "blink.pio.h"

bool core_timer_callback( repeating_timer *rt );

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5

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

// User interface
UserInterface userInterface(&controlPanel, &core, &feedTableFactory);

repeating_timer core_timer;


int main()
{
    stdio_init_all();

    // PIO Blinking example
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    int32_t pio_sm = pio_claim_unused_sm(pio, true);
    printf("Loaded program at %d\n", offset);
    
    blink_pin_forever(pio, pio_sm, offset, PICO_DEFAULT_LED_PIN, 3);

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");

    // Initialize peripherals and pins
    spiBus.initHardware();  
    controlPanel.initHardware();
    stepperDrive.initHardware();
    encoder.initHardware();    

    add_repeating_timer_us(STEPPER_CYCLE_US, core_timer_callback, NULL, &core_timer);

    int i = 0;

    while (true) {
        if(++i > UI_REFRESH_RATE_HZ)
        {
            //printf("Hello, world!\n");
            printf("%lu counts, %ld steps, %ld backlog\n", encoder.getPosition(), stepperDrive.currentPosition, abs(stepperDrive.desiredPosition - stepperDrive.currentPosition));
            i=0;
        }

        // check for step backlog and panic the system if it occurs
        /*if( stepperDrive.checkStepBacklog() ) {
            userInterface.panicStepBacklog();
        }*/

        // service the user interface
        userInterface.loop();

        // delay
        sleep_us(1000000 / UI_REFRESH_RATE_HZ);
    }
}

bool core_timer_callback( repeating_timer *rt )
{
    core.ISR();
    return true;
}
