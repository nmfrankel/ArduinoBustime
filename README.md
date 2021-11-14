# ArduinoBustime
Have a microcontroller with a screen attached show you the distance for the next few city busses that you frequently ride. Obvoiusly, by chaninging the API and some details you can have the board ahow anything.

## What do I need?
* [ESP-8266 micro-controller](https://github.com/nmfrankel/ArduinoBustime#what-do-i-need) - flashing guide is for with a dev breakout board (link coming soon)
* [20x4 I2C screen](https://github.com/nmfrankel/ArduinoBustime#what-do-i-need) - If you have 16x2 screen laying around then feel free to just modify the code (Link coming soon)
* [jumper wire](https://github.com/nmfrankel/ArduinoBustime#what-do-i-need) or wire, solder and solder iron
* [MTA Bustime API key](http://bustime.mta.info/wiki/Developers/Index)

## How do I set this up?
The following will eventually be a guide how to flash an Arduino compaitble ESP-8266 board (from here on, it will be referred to as __The board__) with this software.

## How does this work?
This script allows the board to connect to a predetermined WiFi network. Once connected it will fetch and parse bus location data and echo that info onto the screen of when the next couple busses will be coming (in minutes).

## What can I learn from this project?
* Flashing and interacting with microcontrollers
* Seperating tasks into their own functions
* A little of the C++ programming language syntax
