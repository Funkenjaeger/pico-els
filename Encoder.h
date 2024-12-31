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

#ifndef __ENCODER_H
#define __ENCODER_H

#include <cstdint>
#include "Configuration.h"
#include "hardware/pio.h"
#include "quadrature.pio.h"
#include "pico/stdlib.h"

#define _ENCODER_MAX_COUNT 0x00ffffff
#define _ENCODER_RPM_CALC_HZ 10

class Encoder
{
private:
    uint32_t previous;
    uint16_t rpm;
    PIO pio;
    uint32_t pio_offset;
    uint32_t pio_sm;
    repeating_timer timer;
    friend bool encoder_timer_callback( repeating_timer *rt );

public:
    Encoder( void );
    void initHardware( void );

    uint16_t getRPM( void );
    uint32_t getPosition( void );
    uint32_t getMaxCount( void );
};

inline uint32_t Encoder :: getMaxCount(void)
{
    return _ENCODER_MAX_COUNT;
}

inline uint16_t Encoder :: getRPM(void)
{
    return rpm;
}

bool encoder_timer_callback( repeating_timer *rt );

#endif // __ENCODER_H