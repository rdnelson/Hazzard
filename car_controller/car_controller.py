#!/usr/bin/env python

from ctypes import cdll, Structure, c_int, c_char_p, c_ubyte
import os
import sys

# Get the script's directory
local_dir = os.path.dirname(os.path.realpath(__file__))

sys.path.append("%s/../PiNet" % local_dir)
import PiNet

# Initialize PiNet
rcv = PiNet.Receiver(9002)
nrf_lib = cdll.LoadLibrary("%s/../lib/bcm2835/bcm2835.so" % local_dir)

def CarEvent(player, speed, turn):
    cmd = "H"
    player = int(player)
    speed = int(speed)
    turn = int(turn)
    print "Received CarEvent (Player: %d, Speed: %d, Turn: %d)" % (player, speed, turn)
    if(player == 1):
        print "Prepping packet"
        cmd += chr(speed & 0xFF)
        cmd += chr((speed & 0xFF00) >> 8)
        cmd += chr(turn & 0xFF)
        print "Packet prepped, wrapping"

        ccmd = c_char_p(cmd)

        print "Sending packet to car"
        nrf_lib.nrf_send(ccmd, 4)
        while(nrf_lib.nfr_is_busy()):
            pass;

rcv.addCallback("CarEvent", CarEvent)

CHANNEL=12
PAYLOAD_SIZE=4
MODE_TX=1
VEL = 128
TURN = 1

Packet = c_ubyte * 5

tx_addr = Packet(0xDE, 0xAD, 0xBE, 0xEF, 0xD7)
rx_addr = Packet((0xE7), (0xE7), (0xC3), (0xE2), (0xE7))

print "Initializing nrf24l01 connection..."
nrf_lib.nrf_init()

print "Configuring channel %d and payload size %d" % (CHANNEL, PAYLOAD_SIZE)
nrf_lib.nrf_config(CHANNEL, PAYLOAD_SIZE)

print "Enabling transmission mode"
nrf_lib.nrf_power_radio(MODE_TX)

print "Setting addresses"
nrf_lib.nrf_set_address(1, tx_addr)
nrf_lib.nrf_set_address(0, rx_addr)

while(True):
    pass
