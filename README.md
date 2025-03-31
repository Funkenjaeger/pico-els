# Lathe Electronic Leadscrew Controller
**This project is based on the excellent, original [Clough42 ELS](https://github.com/clough42/electronic-leadscrew)**

I suggest checking out Clough42's original design and [video series](https://www.youtube.com/@Clough42/search?query=electronic%20leadscrew) to get a feel for what the ELS is and what it generally takes to integrate it onto your lathe.

## What is it?
This is the firmware for a lathe electronic leadscrew controller.  The goal is to replace the change gears or gearbox on a metalworking lathe with a stepper motor controlled electronically based on an encoder on the lathe spindle.  The electronic controller allows setting different virtual gear ratios for feeding and threading.

## What's different in this fork?
This fork has migrated the controller from the original TI C2000 dev board to a Raspberry Pi Pico 2.  Motivations for doing so are enumerated below.  Apart from the processor change it's functionally equivalent (when not taking advantage of any added hardware features) to the original ELS, maintains the same basic hardware architecture and same control panel.  

I haven't (yet) published a hardware design for the pico-based control board as it's still a work in progress, but I have successfully stuffed the controller into the same bud box as the control panel.  More to come.
<img width="927" alt="image" src="https://github.com/user-attachments/assets/a3d3379c-4c5f-4584-8bc5-2dc45e91bb70" />

### Motivations
This design seeks to build upon the original ELS in a number of ways:
 * Make a controller board small enough to be co-located with the control panel.  The TI board is fairly large, so in many cases ends up mounted behind or above the lathe, some distance away from the control panel.  The control panel interface is open-drain SPI, and is known to be susceptible to signal integrity and/or interference issues with long cable runs.
 * Use a modern, accessible, popular ecosystem to facilitate further code development.  The TI part is targeted at commercial users - and the documentation, SDK and toolchain reflect that.  Accordingly, there isn't really an online user community to speak of.  The Raspberry Pi ecosystem and community are, by contrast, far more hobbyist-friendly.
 * Provision for capability expansion.  Running critical realtime motion-control code along with everything else in a single core would increasingly become a limitation, if not a liability, when adding more complex functions. 
 
### Architecture
This implementation uses the Raspberry Pi Pico 2 (RP2350 silicon).  True realtime functions (encoder interface, step pulse generation) are handled in PIO state machines, and the rest of the timing-sensitive core motion control code is dedicated to one of the two ARM cores.  The other ARM core hosts all other, non-realtime functions such as user interfaces.

### Added capabilities
#### Implemented
* Auxiliary interface to (optional) gearbox sensor board.  Basically, make the ELS 'aware' of mechanical gearbox settings - forward/reverse, feed/thread, and multiple gear ratios.  At minimum, this improves idiot resistance, but also enables graceful handling of lathes with separate feed (slotted) vs. thread (leadscrew) driveshafts.  On lathes which have separate driveshafts, typically certain features (e.g. power crossfeed, threading dial indicator) are only available via one shaft or the other, so it is desirable to have access to both.
#### Roadmap
* Electronic stop, for turning/threading up to a shoulder (almost certainly dependent on a more elaborate UI).  At minimum this could use a physical limit switch
* Auxiliary touchscreen LCD control panel to provide a UI for more complex functions (such as electronic stop)

## Usage
This code uses the official Raspberry Pi Pico SDK.  The officially recommended approach is to install and use the official Pico SDK extension for VS Code, instructions for which are detailed in the [Getting started with Raspberry Pi Pico-series guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).  With the extension properly installed, when you open this project in VS Code, you should be prompted to import it as a Pico SDK project, which will set up the SDK and build toolchain, create a build subdirectory and automatically configure CMake, etc. 

If you run into failures installing the extension on Windows as I did, check out [pico-vscode issue 141](https://github.com/raspberrypi/pico-vscode/issues/141).

## License and Disclaimer
This software is distributed under the terms of the MIT license.  Read the entire license statement [here](https://github.com/Funkenjaeger/pico-els/blob/develop/LICENSE).
Portions of this software were leveraged from other sources under their respective license terms, as indicated in the headers of individual files.  Copies of the license terms are also included in the root of this repo, with the naming convention `LICENSE-*`.

**Machine tools can be dangerous.**  This is especially true of machine tools that can move by themselves under the control of electronics and software.  You are responsible for your own safety.  Always assume that something could fail and cause the machine to move unexpectedly.  Keep clear of pinch points and be ready to kill the power in an emergency.
