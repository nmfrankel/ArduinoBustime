# ArduinoBustime
Have a screen showing the next couple city busses you frequently ride

## What do I need?
* [ESP-8266 micro-controller]() - flashing guide is for with a dev breakout board (link coming soon)
* [20x4 I2C screen]() - If you have 16x2 screen laying around then feel free to just modify the code (Link coming soon)
* [jumper wire]() or wire, solder and solder iron
* [MTA Bustime API key](http://bustime.mta.info/wiki/Developers/Index)

## How do I set this up?
The following will eventually be a guide how to flash an Arduino compaitble ESP-8266 board (from here on, it will be referred to as __The board__) with this software.

## What does this do and how does it work?
This script allows the board to connect to a predetermined WiFi network. Once connected it will fetch and parse bus location data and echo that info onto the screen of when the next couple busses will be coming (in minutes).

## What can I learn from this project
* Flashing and interact with a micro-controller
* Breakout tasks into their own functions
* The C++ programming language syntax
