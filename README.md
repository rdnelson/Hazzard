Project Hazzard
===============

Hao Cheng

Robert Nelson

Darren Stahl

Hasith Vidanamadura

This is a term project for SYSC3010 (F13). The project is to have
1 Raspberry Pis control 2 RC cars, 1 more Pi take input from Xbox controllers,
a third Pi monitor a finish gate and a final Pi display the status of the system.

Network Requirements
--------------------

In order for the various components to be able to communicate properly, there
needs to be a route for the 224.0.0.1 multicast group. Usually this route can exist
in two ways. There can either be a default route, or there can explicitly be a
multicast route. If components throw socket errors, this is likely the culprit.
To add a route for this multicast group, run the command:

> sudo route add 224.0.0.1 dev eth0

replacing eth0 with the network interface that the communication should take place on.


Starting the System
-------------------

The system has several components, in order for the GUI to function properly
it must be started before the game engine. In addition, the RC cars must be started
before the car controller. All other components can be started in any order and the
system will function properly.

Starting the GUI
----------------

To start the GUI, the java files must be compiled using the commands:

> cd GUI/src
> env CLASSPATH=com/hazzard/gui/dom4j-1.6.1.jar:com/hazzard/gui/jaxen-1.1.6.jar:. javac com/hazzard/gui/\*.java

Then to run the GUI:

> env CLASSPATH=com/hazzard/gui/dom4j-1.6.1.jar:com/hazzard/gui/jaxen-1.1.6.jar:. java com.hazzard.gui.Start

Starting the Car Controller
---------------------------

To run the car controller, the nrf library must be built.

Run:

> make -C lib/bcm2835/

To build the library. Then, the car controller can be started by running:

> cd car\_controller
> ./car\_controller.py

One thing to note is that the RC cars must be on before the car controller is started.

Starting the Game Engine
------------------------

To run the game engine, simply run:

> ./game\_engine.py

Starting the Input Controller
-----------------------------

To run the input controller, the xbox library must be built.

Run:

> make -C xbox

and then the input controller can be started by executing

> cd xbox
> ./input\_controller.py
