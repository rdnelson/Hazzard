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

def LapEvent(carNum):
    global data_ready
    global packet

    packets.append(carNum)
    data_ready.release()

def SendThread():
    global data_ready
    global packet
    
    while True:
        data_ready.acquire()
        num = packets.pop()
        sender.sendAsync("LapFinishedEvent", carNum=str(num))

data_ready = threading.Semaphore(0)
gate_lib = cdll.LoadLibrary("%s/gate.so" % local_dir)

if(gate_lib == None):
    print "Error loading Xbox library."
    exit()
    
gate_lib.init(c_lap_callback(LapEvent))

thread = threading.Thread(None, SendThread)
thread.setDaemon(True)
thread.start()

while True:
    try:
        time.sleep(0.1)
    except KeyboardInterrupt:
        break
