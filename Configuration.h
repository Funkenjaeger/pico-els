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

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

//================================================================================
//                                  LEADSCREW
//
// Define the type and pitch of leadscrew you have in your lathe.  If you have an
// imperial leadscrew, define LEADSCREW_TPI.  If you have a metric leadscrew,
// define LEADSCREW_HMM (pitch in hundredths of a millimeter).  Do not define
// both.
//================================================================================

// For Imperial leadscrews: pitch in Threads Per Inch (TPI)
#define LEADSCREW_TPI 8

// For metric leadscrews: pitch in hundredths of a millimeter (HMM)
// Example: 200hmm = 2mm
//#define LEADSCREW_HMM 200

//================================================================================
//                                STEPPER/SERVO
//
// Define the number of steps and microsteps for your stepper motor, the pin
// polarity of the driver, and whether to use additional features, like servo
// alarm feedback.
//
// NOTE: If you are using a servo drive with something other than a 1:1 drive
// ratio, you can use the STEPPER_MICROSTEPS to compensate.  For example, if you
// have a servo configured for 1000 steps/revolution and it drives the leadscrew
// through a 3:1 reduction, you can set STEPPER_RESOLUTION to 1000 and
// STEPPER_MICROSTEPS to 3.
//================================================================================

// Steps and microsteps
#define STEPPER_MICROSTEPS 8
#define STEPPER_RESOLUTION 200

// Separate step and microstep settings for feed rates.  Redefine these if your
// lathe has a separate feed drive train with a different ratio.
#define STEPPER_MICROSTEPS_FEED STEPPER_MICROSTEPS
#define STEPPER_RESOLUTION_FEED STEPPER_RESOLUTION

// Step, direction and enable pins are normally active-high
// #define INVERT_STEP_PIN true
// #define INVERT_DIRECTION_PIN true
#define INVERT_ENABLE_PIN true
#define INVERT_ALARM_PIN true

// Enable servo alarm feedback
#define USE_ALARM_PIN

//================================================================================
//                                 ENCODER
//
// Define the type of encoder you are using on the spindle.  The firmware assumes
// the encoder is turning at a 1:1 ratio with the spindle.
//
// NOTE: the firmware is concerned with the quadrature edge count, which is
// four times the number of pulses.  For example, if you have a 1024 P/R
// encoder, you need to enter 4096 here.
//================================================================================

// Encoder resolution (counts per revolution)
#define ENCODER_RESOLUTION 6144 //= 1024 PPR * 4 counts/pulse * 60 T spindle pulley / 40 T encoder pulley

// Uncomment to reverse encoder direction
#define REVERSE_ENCODER

//================================================================================
//                                FEATURES
//
// Additional features that can be enabled for your configuration.
//================================================================================

// Ignore all key presses when the machine is running.  Normally, only the mode
// and direction keys are ignored.
//#define IGNORE_ALL_KEYS_WHEN_RUNNING

//================================================================================
//                              VALIDATION/TRIP
//
// Validation thresholds and automatic trip behavior.
//================================================================================

// Maximum number of buffered steps
// The ELS can only output steps at approximately 100KHz.  If you ask the ELS to
// output steps faster than this, it will get behind and will stop automatically
// when the buffered step count exceeds this value.
#define MAX_BUFFERED_STEPS 100

//================================================================================
//                               CPU / TIMING
//
// Define the CPU clock, interrupt, and refresh timing.  Most users will not need
// to touch these settings.
//================================================================================

// Stepper state machine cycle time, in microseconds
// Two cycles are required per step
#define STEPPER_CYCLE_US 5

// User interface refresh rate, in Hertz
#define UI_REFRESH_RATE_HZ 100

// RPM recalculation rate, in Hz
#define RPM_CALC_RATE_HZ 2

// Tick period for control panel interface in microseconds.  The resulting clock
// period will be twice this value.
#define CONTROL_PANEL_CLK_CYCLE_US 5

//================================================================================
//                               GPIO PIN ASSIGNMENTS
//
// Define the mapping of GPIO pins.
//================================================================================
// Stepper/servo motor driver I/O
#define STEPPER_STEP_PIN        6
#define STEPPER_DIRECTION_PIN   7
#define STEPPER_ENABLE_PIN      8
#define STEPPER_ALARM_PIN       9

// Spindle encoder inputs
#define QUADRATURE_A_PIN 28
#define QUADRATURE_B_PIN 27

// Control panel I/O
#define CONTROL_PANEL_STB_PIN 16
#define CONTROL_PANEL_DO_PIN 19
#define CONTROL_PANEL_CLK_PIN 17
#define CONTROL_PANEL_DI_PIN 18

#endif // __CONFIGURATION_H