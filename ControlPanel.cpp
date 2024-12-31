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


#include "ControlPanel.h"

// Time delay to allow CS (STB) line to reach high state and be registered
#define CS_RISE_TIME_US 10

// Time delay after sending read command, before clocking in data
#define DELAY_BEFORE_READING_US 3

// Number of times a key state must be read consecutively to be considered stable
#define MIN_CONSECUTIVE_READS 3


// Lower the TM1638 CS (STB) line
#define CS_ASSERT gpio_put(CONTROL_PANEL_STB_PIN, true)

// Raise the TM1638 CS (STB) line
#define CS_RELEASE gpio_put(CONTROL_PANEL_STB_PIN, false)


ControlPanel :: ControlPanel(SPIBus *spiBus)
{
    this->spiBus = spiBus;
    this->rpm = 0;
    this->value = NULL;
    this->leds.all = 0;
    this->keys.all = 0;
    this->stableKeys.all = 0;
    this->stableCount = 0;
    this->message = NULL;
    this->brightness = 3;
}

void ControlPanel :: initHardware(void)
{
    gpio_init(CONTROL_PANEL_STB_PIN);
    gpio_put(CONTROL_PANEL_STB_PIN, true); // active low
    gpio_set_dir(CONTROL_PANEL_STB_PIN, GPIO_OUT);
}

uint8_t ControlPanel :: lcd_char(uint8_t x)
{
    static const uint8_t table[] = {
        ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, POINT
    };
    if( x < sizeof(table) ) {
        return table[x];
    }
    return table[sizeof(table)-1];
}

void ControlPanel :: sendData()
{
    int i;
    uint16_t ledMask = this->leds.all;
    uint16_t briteVal = 0x80;
    if( this->brightness > 0 ) {
        briteVal = 0x87 + this->brightness;
    }

    CS_ASSERT;
    spiBus->sendWord(briteVal);       // brightness
    CS_RELEASE;
    busy_wait_us(CS_RISE_TIME_US);              // give CS line time to register high

    CS_ASSERT;
    spiBus->sendWord(0x40);           // auto-increment
    CS_RELEASE;
    busy_wait_us(CS_RISE_TIME_US);              // give CS line time to register high

    CS_ASSERT;
    spiBus->sendWord(0xc0);           // display data
    for( i=0; i < 8; i++ ) {
        if( this->message != NULL )
        {
            spiBus->sendWord(this->message[i]);
        }
        else
        {
            spiBus->sendWord(this->sevenSegmentData[i]);
        }
        spiBus->sendWord( (ledMask & 0x80) ? 0xff : 0x00 );
        ledMask <<= 1;
    }
    CS_RELEASE;
    busy_wait_us(CS_RISE_TIME_US);              // give CS line time to register high
}

void ControlPanel :: decomposeRPM()
{
    uint16_t rpm = this->rpm;
    int i;

    for(i=3; i>=0; i--) {
        this->sevenSegmentData[i] = (rpm == 0 && i != 3) ? 0 : lcd_char(rpm % 10);
        rpm = rpm / 10;
    }
}

void ControlPanel :: decomposeValue()
{
    if( this->value != NULL )
    {
        int i;
        for( i=0; i < 4; i++ )
        {
            this->sevenSegmentData[i+4] = this->value[i];
        }
    }
}

KEY_REG ControlPanel :: readKeys(void)
{
    CS_ASSERT;
    spiBus->sendWord(0x40);           // auto-increment
    CS_RELEASE;
    busy_wait_us(CS_RISE_TIME_US);              // give CS line time to register high

    CS_ASSERT;
    spiBus->sendWord(0x42);

    busy_wait_us(DELAY_BEFORE_READING_US); // delay required by TM1638 per datasheet

    uint8_t byte1 = spiBus->receiveWord();
    uint8_t byte2 = spiBus->receiveWord();
    uint8_t byte3 = spiBus->receiveWord();
    uint8_t byte4 = spiBus->receiveWord();

    KEY_REG keyMask;
    keyMask.all =
            (byte1 & 0x11) << 3 |
            (byte2 & 0x11) << 2 |
            (byte3 & 0x11) << 1 |
            (byte4 & 0x11);

    CS_RELEASE;
    busy_wait_us(CS_RISE_TIME_US);              // give CS line time to register high

    return keyMask;
}

KEY_REG ControlPanel :: getKeys()
{
    KEY_REG newKeys;
    static KEY_REG noKeys;

    newKeys = readKeys();
    if( isValidKeyState(newKeys) && isStable(newKeys) && newKeys.all != this->keys.all ) {
        KEY_REG previousKeys = this->keys; // remember the previous stable value
        this->keys = newKeys;

        if( previousKeys.all == 0 ) {     // only act if the previous stable value was no keys pressed
            return newKeys;
        }
    }
    return noKeys;
}

bool ControlPanel :: isValidKeyState(KEY_REG testKeys) {
    // filter out any states with multiple keys pressed (bad communication filter)
    switch(testKeys.all) {
    case 0:
    case 1 << 0:
    case 1 << 2:
    case 1 << 3:
    case 1 << 4:
    case 1 << 5:
    case 1 << 6:
    case 1 << 7:
        return true;
    }

    return false;
}


bool ControlPanel :: isStable(KEY_REG testKeys) {
    // don't trust any read key state until we've seen it multiple times consecutively (noise filter)
    if( testKeys.all != stableKeys.all )
    {
        this->stableKeys = testKeys;
        this->stableCount = 1;
    }
    else
    {
        if( this->stableCount < MIN_CONSECUTIVE_READS )
        {
            this->stableCount++;
        }
    }

    return this->stableCount >= MIN_CONSECUTIVE_READS;
}

void ControlPanel :: setMessage( const uint8_t *message )
{
    this->message = message;
}

void ControlPanel :: setBrightness( uint8_t brightness )
{
    if( brightness > 8 ) brightness = 8;

    this->brightness = brightness;
}

void ControlPanel :: refresh()
{
    decomposeRPM();
    decomposeValue();

    sendData();
}






