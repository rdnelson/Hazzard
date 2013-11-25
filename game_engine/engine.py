#!/usr/bin/env python

import os
import sys

local_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/../PiNet" % local_dir)

import PiNet

DEBUG=1
LR = 0
ACCEL = 5
BRAKE = 2
MAX_PLAYERS = 4

sender = PiNet.Sender()
receiver = PiNet.Receiver()

forward_speed = []
rev_speed = []
turn = []

for i in range(MAX_PLAYERS):
    forward_speed.append(0)
    rev_speed.append(0)
    turn.append(0)

def ControllerEvent(player, event, data):
    print "Received controller event"
    player = int(player)
    event = int(event)
    data = int(data)
    send = False
    if (event == ACCEL):
        forward_speed[player] = data
        send = True
    if (event == BRAKE):
        rev_speed[player] = data
        send = True
    if (event == LR):
        send = True
        if data == -32767:
            turn[player] = -1
        if data == 0:
            turn[player] = 0
        if data == 32767:
            turn[player] = 1

    if send:
        speed = forward_speed[player] - rev_speed[player]
        if (DEBUG==1):
            print "Player: %d, Speed=%d, Turn=%d" % (player, speed, turn[player])
        sender.sendAsync("CarEvent", player=str(int(player) + 1), speed=str(speed), turn=str(turn[player]))
        print "Packet Sent"

print "Adding callback"
receiver.addCallback("ControllerEvent", ControllerEvent)

receiver.hibernate()
