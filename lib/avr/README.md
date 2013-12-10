
Introduction
------------
The AVR binaries for the car controller are contained in this directory. 

Prerequisites
-------------
The firmware requires avr-gcc and libc-avr to compile for the AVR ATMEGA328P device.



Compiling and Flashing
---------

> make
> sudo make flash

The makefile assumes that there is a USBtiny programmer connected to the ICSP header of the controllers, and flashes main.hex to the device.

