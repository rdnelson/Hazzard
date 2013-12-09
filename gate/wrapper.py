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
sender.debug = True
#define the input callback type
c_lap_callback = CFUNCTYPE(None, c_int)

#Callback when we get a lap
def LapEvent(carNum):
    global data_ready
    global packet

    packets.append(carNum)
    #notify send thread
    data_ready.release()

#Send any data when available
def SendThread():
    global data_ready
    global packet
    
    while True:
    	#Wait on data
        data_ready.acquire()
        num = packets.pop()
        sender.sendAsync("LapFinishedEvent", carNum=str(num))

data_ready = threading.Semaphore(0)

#Load the shared library
gate_lib = cdll.LoadLibrary("%s/gate.so" % local_dir)

if(gate_lib == None):
    print "Error loading Xbox library."
    exit()
    
#Call the init function with the callback for LapEvent
gate_lib.init(c_lap_callback(LapEvent))

#Create and start the sender thread
thread = threading.Thread(None, SendThread)
thread.setDaemon(True)
thread.start()

sender.hibernate()
