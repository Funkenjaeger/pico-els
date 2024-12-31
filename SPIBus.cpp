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

#include "SPIBus.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "Configuration.h"


SPIBus :: SPIBus( void )
{

}

void SPIBus :: initHardware(void)
{
    gpio_init(CONTROL_PANEL_DO_PIN);
    gpio_init(CONTROL_PANEL_DI_PIN);
    gpio_init(CONTROL_PANEL_CLK_PIN);    
    gpio_put(CONTROL_PANEL_DO_PIN, false);
    gpio_put(CONTROL_PANEL_CLK_PIN, false);    
    gpio_set_dir(CONTROL_PANEL_DI_PIN, GPIO_IN);
    gpio_set_dir(CONTROL_PANEL_DO_PIN, GPIO_OUT);
    gpio_set_dir(CONTROL_PANEL_CLK_PIN, GPIO_OUT);
    gpio_pull_up(CONTROL_PANEL_DI_PIN);
}

void SPIBus :: sendWord(uint8_t data)
{
    uint8_t i;
    // Note: inverted output logic for NPN open-collector drivers
    for (i = 0; i < 8; i++)  {
        gpio_put(CONTROL_PANEL_DO_PIN, !bool(data & (1 << i)));  
        gpio_put(CONTROL_PANEL_CLK_PIN, true);
        busy_wait_us(CONTROL_PANEL_CLK_CYCLE_US);
        gpio_put(CONTROL_PANEL_CLK_PIN, false);
        busy_wait_us(CONTROL_PANEL_CLK_CYCLE_US);
    }
    gpio_put(CONTROL_PANEL_DO_PIN, false); // release bus
}

uint8_t SPIBus :: receiveWord(void) 
{
    uint8_t rxword = 0;
    uint8_t i = 0;

    for(i = 0; i < 8; ++i) {
        
        gpio_put(CONTROL_PANEL_CLK_PIN, true);
        busy_wait_us(CONTROL_PANEL_CLK_CYCLE_US);
        rxword |= !gpio_get(CONTROL_PANEL_DI_PIN) << i;
        gpio_put(CONTROL_PANEL_CLK_PIN, false);
        busy_wait_us(CONTROL_PANEL_CLK_CYCLE_US);
    }
    return rxword;
}




