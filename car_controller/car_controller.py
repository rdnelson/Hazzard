#!/usr/bin/env python

from ctypes import cdll, Structure, c_int, c_char_p, c_ubyte
import os
import sys

# Get the script's directory
local_dir = os.path.dirname(os.path.realpath(__file__))

sys.path.append("%s/../PiNet" % local_dir)
import PiNet

# Initialize PiNet
rcv = PiNet.Receiver()
nrf_lib = cdll.LoadLibrary("%s/../lib/bcm2835/bcm2835.so" % local_dir)

player_car = {}

CAR_IDS = [0x0A, 0x0B]

def CarEvent(player, speed, turn):
    cmd = "H"
    player = int(player)
    if player_car.get(player) == None:
        return
    car = player_car[player]
    speed = int(speed)
    turn = int(turn)
    print "Received CarEvent (Player: %d, Car: %d, Speed: %d, Turn: %d)" % (player, car, speed, turn)
    if(car < len(CAR_IDS)):
        print "Prepping packet"
        cmd += chr(speed & 0xFF)
        cmd += chr((speed & 0xFF00) >> 8)
        cmd += chr(turn & 0xFF)
        print "Packet prepped, wrapping"

        ccmd = c_char_p(cmd)

        print "Sending packet to car"

        nrf_lib.nrf_set_address_suffix(c_ubyte(CAR_IDS[car]))
        nrf_lib.nrf_send(ccmd, 4)
        while(nrf_lib.nfr_is_busy()):
            pass

def RegisterCar(player, car):
    if car >= len(CAR_IDS):
        return

    player_car[player] = car
    cmd = "I"
    cmd += chr(player + 1)

    ccmd = c_char_p(cmd)

    nrf_lib.nrf_set_address_suffix(c_ubyte(CAR_IDS[car]))
    nrf_lib.nrf_send(ccmd, 2);
    while(nfr_lib.nrf_is_busy()):
        pass

rcv.addCallback("CarEvent", CarEvent)
rcv.addCallback("RegisterCar", RegisterCar)

CHANNEL=12
PAYLOAD_SIZE=4
MODE_TX=1
VEL = 128
TURN = 1

RegisterCar(0, 0)
RegisterCar(1, 1)

Packet = c_ubyte * 5

tx_addr = Packet(0x0A, 0xEF, 0xAD, 0xAD, 0xDE)
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
