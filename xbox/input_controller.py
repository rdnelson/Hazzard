#!/usr/bin/env python
from ctypes import cdll, Structure, c_int, CFUNCTYPE, c_uint
import os
import sys
from math import copysign
import time
import threading

# Get the script's directory
local_dir = os.path.dirname(os.path.realpath(__file__))

sys.path.append("%s/../PiNet" % local_dir)
import PiNet

# Initialize PiNet
sender = PiNet.Sender()

packets = []

#constants
STICK_DEADZONE = 15000
STICKS = [0,1,3,4]
THROTTLE = [2, 5]
BUTTONS = [304,305,307,308,310,311,314,315,316,317,318,704,705,706,707]
MAX_PLAYERS = 4
STICK_MAX = 32767
DEBUG = 1

# Define the event class for the xbox library
class PlayerEvent(Structure):
    _fields_ = [("player", c_int), ("event", c_int), ("data", c_int)]

#define the input callback type
c_read_callback = CFUNCTYPE(None, PlayerEvent)

#define the input callback arg structure
class info(Structure):
    _fields_ = [("fd", c_int), ("cb", c_read_callback), ("player", c_uint)]

def InputEvent(e):
    global packet
    global data_ready
    global STICKS
    global sticks_dead
    global STICKS_DEADZONE
    global STICK_MAX

    if(STICKS.count(e.event) == 1):
        if(sticks_dead[e.player][e.event] == 1 and abs(e.data) > STICK_DEADZONE):
            sticks_dead[e.player][e.event] = 0
            e.data = int(copysign(STICK_MAX, e.data))
        elif(sticks_dead[e.player][e.event] == 0 and abs(e.data) < STICK_DEADZONE):
            sticks_dead[e.player][e.event] = 1
            e.data = 0
        else:
            return

    packets.append((e.player,e.event,e.data))
    data_ready.release()
    #sender.sendAsync("ControllerEvent", player=str(e.player), event=str(e.event), data=str(e.data))
    if DEBUG:
        print e.player, ",", e.event, ",", e.data

def SendThread():
    global data_ready
    global packet

    while True:
        data_ready.acquire()
        packet = packets.pop()
        sender.sendAsync("ControllerEvent", player=str(packet[0]), event=str(packet[1]), data=str(packet[2]))


# Create stick deadzone status dictionary
sticks_dead = []
for i in range(MAX_PLAYERS):
    sticks_dead.append({})
    for stick in STICKS:
        sticks_dead[i][stick] = 1

data_ready = threading.Semaphore(0)

# Load the library, and bail if the loading fails
xbox_lib = cdll.LoadLibrary("%s/xbox.so" % local_dir)

if(xbox_lib == None):
    print "Error loading Xbox library."
    exit()

print "Number of controllers:", xbox_lib.open_controllers()

for i in range(MAX_PLAYERS):
    if DEBUG:
        print "Registering callback for player %d" % i
    xbox_lib.add_callback(i, c_read_callback(InputEvent))

thread = threading.Thread(None, SendThread)
thread.setDaemon(True)
thread.start()

while True:
    try:
        time.sleep(0.1)
    except KeyboardInterrupt:
        break
