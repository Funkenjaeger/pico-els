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

#ifndef __TABLES_H
#define __TABLES_H

#include <cstdint>
#include "Configuration.h"
#include "ControlPanel.h"

typedef struct FEED_THREAD
{
    uint16_t display[4];
    union LED_REG leds;
    uint64_t numerator;
    uint64_t denominator;
} FEED_THREAD;



class FeedTable
{
private:
    const FEED_THREAD *table;
    uint16_t selectedRow;
    uint16_t numRows;

public:
    FeedTable(const FEED_THREAD *table, uint16_t numRows, uint16_t defaultSelection);

    const FEED_THREAD *current(void);
    const FEED_THREAD *next(void);
    const FEED_THREAD *previous(void);
};


class FeedTableFactory
{
private:
    FeedTable inchThreads;
    FeedTable inchFeeds;
    FeedTable metricThreads;
    FeedTable metricFeeds;

public:
    FeedTableFactory(void);

    FeedTable *getFeedTable(bool metric, bool thread);
};


#endif // __TABLES_H
