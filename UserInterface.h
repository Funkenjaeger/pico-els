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


#ifndef __USERINTERFACE_H
#define __USERINTERFACE_H

#include "ControlPanel.h"
#include "Core.h"
#include "Tables.h"
#include "CoreProxy.h"
#include "Gearbox.h"

class UserInterface
{
private:
    ControlPanel *controlPanel;
    Core *core;
    FeedTableFactory *feedTableFactory;
    Gearbox *gearbox;

    bool metric;
    bool thread;
    bool reverse;

    bool useGearbox;

    FeedTable *feedTable;

    KEY_REG keys;

    GearboxState gearboxState;

    const MESSAGE *message;
    uint16_t messageTime;

    const FEED_THREAD *loadFeedTable();
    LED_REG calculateLEDs();
    void setMessage(const MESSAGE *message);
    void overrideMessage( void );
    void clearMessage( void );

public:
    UserInterface(ControlPanel *controlPanel, Core *core, FeedTableFactory *feedTableFactory, Gearbox *gearbox);

    void loop( void );

    void panicStepBacklog( void );
};

#endif // __USERINTERFACE_H
