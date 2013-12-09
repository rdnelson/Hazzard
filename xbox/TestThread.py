#!/usr/bin/env python

import ctypes as ct
import time

# Simple class for struct emulation
class PlayerEvt(ct.Structure):
    _fields_ = [("player",ct.c_int), ("event", ct.c_int), ("data", ct.c_int)]

# callback type
c_read_callback = ct.CFUNCTYPE(ct.c_voidp, PlayerEvt)

# Simple class for struct emulation
class info(ct.Structure):
    _fields_ = [("fd", ct.c_int), ("cb", c_read_callback), ("player", ct.c_uint)]

#callback function
def cb(info):
    print "Player: %d Event: %d Data: %d" % (info.player, info.event, info.data)
    return 0

xbox = ct.cdll.LoadLibrary("./xbox.so")

xbox.open_controllers()

xbox.add_callback(0, c_read_callback(cb))
xbox.add_callback(1, c_read_callback(cb))

while True:
    time.sleep(1)
