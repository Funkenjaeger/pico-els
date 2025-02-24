# Lathe Electronic Leadscrew Controller
**This project is based on the excellent, original [Clough42 ELS](https://github.com/clough42/electronic-leadscrew)**

This is the firmware for a lathe electronic leadscrew controller.  The goal is to replace the change
gears or gearbox on a metalworking lathe with a stepper motor controlled electronically based on an encoder on the
lathe spindle.  The electronic controller allows setting different virtual gear ratios for feeds and threading.

## What's different in this fork?
Relative to the Clough42 ELS, the general architecture is similar and much of the code is has been leveraged.  This implementation is directly compatible with the Clough42 TM1638-based control panel.

### Motivations
This design seeks to improve upon the original ELS in a couple of primary ways:
 * Use a small form-factor controller that can be co-located with the control panel to reduce signal integrity challenges
 * Use an accessible, popular ecosystem to facilitate further development (let's face it, the TI CCS toolchain is not great)
 
### Architecture
This implementation uses the Raspberry Pi Pico 2 (RP2350 silicon).  This chip has dual ARM cores and PIO peripheral state machines, well suited to a mix of realtime and non-realtime interfaces.

## License and Disclaimer

This software is distributed under the terms of the MIT license.  Read the entire license statement [here](https://github.com/Funkenjaeger/pico-els/blob/master/LICENSE).
Portions of this software were leveraged from other sources under their respective license terms, as indicated in the headers of individual files.  Copies of the license terms are also included in the root of this repo, with the naming convention `LICENSE-*`.

**Machine tools can be dangerous.**  This is especially true of machine tools that can move by themselves
under the control of electronics and software.  You are responsible for your own safety.  Always assume
that something could fail and cause the machine to move unexpectedly.  Keep clear of pinch points and 
be ready to kill the power in an emergency.
