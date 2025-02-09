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

#include <cmath>
#include "Encoder.h"
#include "Configuration.h"

Encoder :: Encoder( void )
{
    previous = 0;
    rpm = 0;
}

void Encoder :: initHardware(void)
{
    gpio_set_dir(QUADRATURE_A_PIN, GPIO_IN);
    gpio_set_dir(QUADRATURE_B_PIN, GPIO_IN);
    gpio_pull_up(QUADRATURE_A_PIN);
    gpio_pull_up(QUADRATURE_B_PIN);
    gpio_set_function(QUADRATURE_A_PIN, GPIO_FUNC_PIO0);    
    gpio_set_function(QUADRATURE_B_PIN, GPIO_FUNC_PIO0);
    pio = pio0;
    pio_offset = pio_add_program(pio, &quadratureA_program);
    pio_sm = pio_claim_unused_sm(pio, true);
    quadratureA_program_init(pio, pio_sm, pio_offset, QUADRATURE_A_PIN, QUADRATURE_B_PIN);

    add_repeating_timer_us(1e6/_ENCODER_RPM_CALC_HZ, encoder_timer_callback, this, &timer);
}

int32_t Encoder :: getPosition(void)
{
    pio_sm_exec_wait_blocking(pio, pio_sm, pio_encode_in(pio_x, 32));
    //return static_cast<int32_t>(pio_sm_get_blocking(pio, pio_sm));
    return (int32_t)pio_sm_get_blocking(pio, pio_sm);
}

bool encoder_timer_callback(repeating_timer *rt)
{
    Encoder * encoder = static_cast<Encoder *>(rt->user_data);
    int32_t position = encoder->getPosition();

    encoder->rpm = uint16_t(abs(position - encoder->previous) * _ENCODER_RPM_CALC_HZ * 60 / ENCODER_RESOLUTION);

    encoder->previous = position;
    return true;
}