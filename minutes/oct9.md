Minutes for Oct 9 2013
======================

Breaking down components and operation

Component list
--------------

* Cars (No Changes)
* RC Controller -> Pi interface
* Xbox Controller -> Pi Interface
* Status display
* Finish gate

Details of Components
=====================

Cars
----

Any changes?

Support pwm input?

Add IR LEDs blinking in a pattern (555?) to identify each car?
Pi doesn't go on car (high risk of breakage)

Lets minimize changes to cars.

RC Controller -> Pi Interface
-----------------------------

Don't know what sort of input the controller expects.

Use transistors if we need a switch, use ADC if it needs a variable voltage.


Xbox Controller -> Pi Interface
-----------------------------

XBox wireless controller supported as joystick on linux with appropriate driver.

Just need to use joystick input device, looks simple


Status Display
--------------

The display should show the current turning status of the cars, the speed they're moving, any modifiers currently in place.

Race previous times?

Place?


Finish Gate
-----------

IR receiver on gate to detect car finish.

How to distinguish which car finished first?
-> Collisions between IR signals in close finish
-> Manchester encoding? Corrupt signal means tie?

Connected to Display for ease of system design
