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


#include "Gearbox.h"
#include "Configuration.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "cppcrc.h"

Gearbox :: Gearbox(void) {
    i2c_init(i2c0, GEARBOX_I2C_BAUDRATE);
    gpio_set_function(GEARBOX_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(GEARBOX_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(GEARBOX_SDA_PIN);
    gpio_pull_up(GEARBOX_SCL_PIN);
    this->state.direction = FORWARD;
    this->state.feed_thread = FEED;
    this->state.gear = A;
    this->state.finalDriveRatio = FEED_GEAR_RATIO * GEARBOX_DRIVE_RATIO_A;
}

bool Gearbox :: getState(GearboxState* state) {
    int ret;
    uint8_t rxdata[4];

    ret = i2c_read_blocking(i2c_default, 0x55, rxdata, 4, false);

    uint8_t crc_value = CRC8::CRC8::calc(rxdata, sizeof(rxdata)-1);

    if(ret && crc_value == rxdata[3]) {
        switch(rxdata[0]){
            case 'F':
                this->state.direction = FORWARD;
                break;
            case 'R':
                this->state.direction = REVERSE;
                break;
            default:
                break;
        }

        
        switch(rxdata[1]){
            case 'A':
                this->state.gear = A;
                break;
            case 'B':
                this->state.gear = B;
                break;
            case 'C':
                this->state.gear = C;
        } 
        
        float gearRatio;
        switch(this->state.gear){
            case A:
                gearRatio = GEARBOX_DRIVE_RATIO_A;
                break;
            case B:
                gearRatio = GEARBOX_DRIVE_RATIO_B;
                break;
            case C:
                gearRatio = GEARBOX_DRIVE_RATIO_C;
                break;
            default:
                gearRatio = 0.0;
        }

        switch(rxdata[2]){
            case 'F':
                this->state.feed_thread = FEED;
                break;
            case 'T':
                this->state.feed_thread = THREAD;
        }

        float feedThreadDriveRatio;
        switch(this->state.feed_thread){
            case FEED:
                feedThreadDriveRatio = FEED_GEAR_RATIO;
                break;
            case THREAD:
                feedThreadDriveRatio = THREAD_GEAR_RATIO;
                break;
            default:
                feedThreadDriveRatio = 0.0;
        }

        this->state.finalDriveRatio = gearRatio * feedThreadDriveRatio;
    }

    *state = this->state;

    //printf("%d [%02d,%02d(%c),%02d,%03d] CRC %s *** ", ret, rxdata[0], rxdata[1], 'A'+state->gear, rxdata[2], rxdata[3], crc_value==rxdata[3] ? "OK" : "MISMATCH");
    return ret;
}