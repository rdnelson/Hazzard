#!/usr/bin/env python

import os
import sys
from threading import Timer
from lxml import etree as xml
from datetime import datetime, timedelta

local_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/../PiNet" % local_dir)

import PiNet
import Powerups
import laps

shutdown = False

class PlayerState:
    def __init__(self):
        self.forward = 0
        self.reverse = 0
        self.speed_percent = DEFAULT_SPEED_PERCENT
        self.turn = 0
        self.immortal = False
        self.effects = []
        self.powerups = None
        self.turn = 0
        self.laps = []

def TimeEvent():

    # update effect status
    for player in players:
        handleEffects(player)

    # insert powerup awarding here

    if not shutdown:
        timer = Timer(0.1, TimeEvent)
        timer.start()


def handleEffects(player):
    for i in range(len(player.effects) - 1, -1, -1):
        eff = player.effects[i]
        timeElapsed = datetime.now() - eff[1]

        #prune expired effects
        if timeElapsed.seconds + timeElapsed.microseconds / 1000000.0 > eff[0].get_duration():
            player.effects.pop(i)
            continue

        new_speed_percent = DEFAULT_SPEED_PERCENT
        #if this executes, the effect is affecting the player.
        #apply any speed modifiers
        if eff[0].get_maxSpeed() != None:
            if self.immortal or (self.immortal and eff[0].get_maxSpeed() > 0):
                new_speed_percent += eff[0].get_maxSpeed()

        player.speed_percent = new_speed_percent

def triggerPowerup(player):

    # Bail if there's no powerup
    if players[player].powerup == None:
        return

    effects = players[player].powerup.get_effect()
    if effects == None:
        return

    for target in getTargets(players[player].powerup, players.index(player)):
        for eff in effects:
            players[target].effects.append((eff,datetime.now()))

def getTargets(powerup, caller_idx):

    retVal = []

    if powerup == None:
        return retVal

    rel = powerup.get_positionRel()
    absol = powerup.get_positionAbs()
    inv = powerup.get_invertSel()

    # get current positions
    positions = getPositions(players)
    caller_pos = positions.index(caller_idx)

    if absol != None:
        # specific place (ex. 1st place)
        if inv == None:
            if absol < MAX_PLAYERS and absol >= 0:
                if DEBUG:
                    print "Targeting (Absolute): ", [positions[absol]]
                return [positions[absol]]

        # inverted placing (ex. all but 2nd)
        if inv != None:
            for i in range(MAX_PLAYERS):
                if i != absol:
                    retVal.append(positions[i])
            if DEBUG:
                print "Targeting (Inverse Absolute): ", retVal
            return retVal

    if rel != None:

        target_pos = caller_pos - rel

        # relative place (ex. 1 in front)
        if inv == None:
            if target_pos >= 0 and target_pos < MAX_PLAYERS:
                if DEBUG:
                    print "Targeting (Relative): ", [positions[target_pos]]
                return [positions[target_pos]]

        # relative inverted place (ex. all but me)
        if inv != None:
            for i in range(MAX_PLAYERS):
                if i != target_pos:
                    retVal.append(i)
            if DEBUG:
                print "Targeting (Inverse Relative): ", retVal
            return retVal

    return []

#Define constants
DEBUG=True

#Event codes for xbox events
LR = 0
ACCEL = 5
BRAKE = 2
BUT_A = 304

#Game constants
MAX_PLAYERS = 4
MAX_SPEED = 255
MIN_SPEED = -255
DEFAULT_SPEED_PERCENT = 0.75

#Validate the powerups schema
powerups_schema_doc = xml.parse("./powerup.xsd")
powerups_schema = xml.XMLSchema(powerups_schema_doc)
powerups_doc = xml.parse("./powerups.xml")
powerups_schema.assertValid(powerups_doc)

# cleanup schema validation objects
powerups_schema = None
powerups_doc = None
powerups_schema_doc = None

#initialize pinet
sender = PiNet.Sender()
receiver = PiNet.Receiver()

#load the powerup classes
powerups = Powerups.parse("./powerups.xml").get_powerup()

players = []

for i in range(MAX_PLAYERS):
    players.append(PlayerState())

def ControllerEvent(player, event, data):
    if (DEBUG):
        print "Received controller event"

    player = int(player)

    #Bail if there's an invalid player
    if (player < 0 or player > MAX_PLAYERS):
        return

    event = int(event)
    data = int(data)

    #don't send a packet unless necessary
    send = False
    if (event == ACCEL):
        players[player].forward = data
        send = True
    if (event == BRAKE):
        players[player].reverse = data
        send = True
    if (event == LR):
        send = True
        if data == -32767:
            players[player].turn = -1
        if data == 0:
            players[player].turn = 0
        if data == 32767:
            players[player].turn = 1
    if event == BUT_A and data == 1 and players[player].powerup != None:
        triggerPowerup(player)

    if send:
        speed = players[player].forward - players[player].reverse

        #Deal with player effects
        handleEffects(player)

        speed = int(speed * players[player].speed_percent)

        #enforce speed boundaries
        if (speed > MAX_SPEED):
            speed = MAX_SPEED

        if (speed < MIN_SPEED):
            speed = MIN_SPEED

        if (DEBUG):
            print "Player: %d, Speed=%d, Turn=%d" % (player, speed, turn[player])
        sender.sendAsync("CarEvent", player=str(int(player) + 1), speed=str(speed), turn=str(turn[player]))
        if (DEBUG):
            print "Packet Sent"

print "Setting up PiNet connection"
receiver.addCallback("ControllerEvent", ControllerEvent)

timer = Timer(0.1, TimeEvent)
timer.start()
print "Beginning event timer"

receiver.hibernate()
shutdown = True
print "Shutting down game engine."
