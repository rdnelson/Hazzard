#!/usr/bin/env python
from ctypes import cdll, Structure, c_int
import os
import sys
from math import copysign

# Get the script's directory
local_dir = os.path.dirname(os.path.realpath(__file__))

sys.path.append("%s/../PiNet" % local_dir)
import PiNet

# Initialize PiNet
sender = PiNet.Sender()


#constants
STICK_DEADZONE = 15000
STICKS = [0,1,3,4]
THROTTLE = [2, 5]
BUTTONS = [304,305,307,308,310,311,314,315,316,317,318,704,705,706,707]
MAX_PLAYERS = 4
STICK_MAX = 32767
DEBUG = 0

# Define the event class for the xbox library
class PlayerEvent(Structure):
    _fields_ = [("player", c_int), ("event", c_int), ("data", c_int)]

def valid_event(evt):
    # Filter out invalid players
    if(evt.player == -1 or evt.player > MAX_PLAYERS):
        return False

    # Filter out stick dead zones (Bandwidth throttling)
    if(STICKS.count(evt.event) == 1):
        if(sticks_dead[evt.event] == 1 and abs(evt.data) > STICK_DEADZONE):
            sticks_dead[evt.event] = 0
            evt.data = int(copysign(STICK_MAX, evt.data))
            return True
        elif(sticks_dead[evt.event] == 0 and abs(evt.data) < STICK_DEADZONE):
            sticks_dead[evt.event] = 1
            evt.data = 0
            return True
        else:
            return False
    return True

def error_event(evt):
    if(evt.player == -1 and evt.data != -1):
        return True
    return False

# Create stick deadzone status dictionary
sticks_dead = {}
for stick in STICKS:
    sticks_dead[stick] = 1


# Load the library, and bail if the loading fails
xbox_lib = cdll.LoadLibrary("%s/xbox.so" % local_dir)

if(xbox_lib == None):
    print "Error loading Xbox library."
    exit()

print "Number of controllers:", xbox_lib.open_controllers()

# Loop forever reading packets, validating them, and forwarding them to pinet
while True:
    get_event = xbox_lib.get_event
    get_event.restype = PlayerEvent
    e = xbox_lib.get_event()
    if (valid_event(e)):
        sender.sendAsync("ControllerEvent", player=str(e.player), event=str(e.event), data=str(e.data))
        if (DEBUG):
            print e.player, ",", e.event, ",", e.data
    elif(err_event(e)):
        print "Fatal error, quitting."
