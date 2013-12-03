#!/usr/bin/env python

import os
import sys
from threading import Timer
from lxml import etree as xml
from datetime import datetime, timedelta
from time import sleep

local_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/../PiNet" % local_dir)

import PiNet
import Powerups
import laps
import Players

class RaceInfo(PiNet.Data):
    def __init__(self, laps, players, state):
        self.num_laps = laps
        self.num_players = players
        self.state = state

#Define constants
DEBUG=True
DEFAULT_LAPS = 5

#Event codes for xbox events
LR = 0
UD = 1
DUP = 706
DDOWN = 707
ACCEL = 5
BRAKE = 2
BUT_A = 304
START = 315

#race states
JOINING = 0
LAP_SELECT = 1
COUNTDOWN = 2
RACING = 3
FINISHED = 4

#initialize variables
#have to be global to be useable inside the callbacks
shutdown = False
race_state = JOINING
num_laps = DEFAULT_LAPS
num_players = 0
num_finished = 0
start_time = datetime.now()
players = []

# Populate the players array with nonexsistant players
for i in range(Players.MAX_PLAYERS):
    players.append(None)

#Event handlers
def TimeEvent():

    # update effect status
    for player in players:
        Players.handleEffects(player)

    if not shutdown:
        timer = Timer(0.1, TimeEvent)
        timer.start()

def ControllerEvent(player, event, data):
    global num_players
    global race_state
    global num_laps
    global players

    if (DEBUG and race_state != LAP_SELECT):
        try:
            print "Received controller event (Player %s State %d)" % (player, race_state)
        except Exception as e:
            print e

    player = int(player)

    #Bail if there's an invalid player
    if (player < 0 or player > Players.MAX_PLAYERS):
        if DEBUG:
            print "Invalid player, should be >= 0 and < %d" % Players.MAX_PLAYERS
        return

    event = int(event)
    data = int(data)

    #don't send a packet unless necessary
    send = False

    if race_state == JOINING:
        if DEBUG:
            print "Joining event=%d data=%d" % (event, data)
        if event == BUT_A and data == 1 and players[player] == None:
            print "Player %d is now ready" % (player + 1)
            players[player] = Players.PlayerState()
            num_players += 1
            sender.sendUpdate(RaceInfo(num_laps, num_players,race_state), "RaceInfo")
        if event == START and data == 1 and players[player] != None:
            print "Starting with current players"
            race_state = LAP_SELECT
        elif DEBUG:
            print "Ignoring input until race begins"
        return
    elif race_state == LAP_SELECT:
        if event == UD and data == -32767 or event == DUP and data == 1:
            num_laps += 1
        elif event == UD and data == 32767 or event == DDOWN and data == 1:
            num_laps -= 1
            if num_laps < 1:
                num_laps = 1
        elif event == START and data == 1:
            race_state = COUNTDOWN
            print
            return
        print "%s[2K\r" % chr(27),
        print "Please select number of Laps: %d" % num_laps,
        sys.stdout.flush()
        try:
            sender.sendUpdate(RaceInfo(num_laps, num_players, race_state), "RaceInfo")
        except Exception as e:
            print e
        return

    elif race_state == COUNTDOWN:
        return
    elif race_state == FINISHED:
        if event == START and data == 1:
            # reset all the players stats
            for pl in players:
                if pl != None:
                    pl.init()
            race_state = JOINING
            return

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
        Players.handleEffects(player)

        speed = int(speed * players[player].speed_percent)

        #enforce speed boundaries
        if (speed > Players.MAX_SPEED):
            speed = Players.MAX_SPEED

        if (speed < Players.MIN_SPEED):
            speed = Players.MIN_SPEED

        if (DEBUG):
            print "Player: %d, Speed=%d, Turn=%d" % (player, speed, turn[player])
        sender.sendAsync("CarEvent", player=str(int(player) + 1), speed=str(speed), turn=str(turn[player]))
        if (DEBUG):
            print "Packet Sent"

def PingEvent():
    sender.sendAsync("Pong")
    sender.sendUpdate(RaceInfo(num_laps, num_players, race_state), "RaceInfo")

def GateEvent(player):
    global players
    global num_finished
    global race_state

    player = int(player)
    if player < 0 or player >= Players.MAX_PLAYERS:
        return

    if players[player] == None:
        return

    timediff = datetime.now() - start_time
    secs = timediff.seconds + timediff.microseconds / 1000000.0

    if len(players[player].laps) != 0:
        secs -= sum(players[player].laps)

    if DEBUG:
        try:
            print "Player %d finished lap %d in %f seconds" % (player + 1, len(players[player].laps) + 1, secs)
        except Exception as e:
            print e

    players[player].laps.append(round(secs, 2))

    if len(players[player].laps) == num_laps:
        sender.sendAsync("PlayerFinish", player=str(player), time=str(sum(players[player].laps)))
        num_finished += 1
        if DEBUG:
            print "# finished: %d, # total: %d" %(num_finished, num_players)
        if num_finished == num_players:
            sender.sendAsync("RaceFinish")
            race_state = FINISHED
            print "The race is over!"
    elif len(players[player].laps) < num_laps:
        sender.sendAsync("LapComplete", player=str(player), time=str(secs))


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

#Only execute code if engine is running
if __name__ == "__main__":
    print "Setting up PiNet connection"
    receiver.addCallback("ControllerEvent", ControllerEvent)
    receiver.addCallback("CarPassed", GateEvent)
    receiver.addCallback("Ping", PingEvent)

    try:
        while True:
            print "Waiting for players to confirm (Press A)"
            while race_state == JOINING:
                sleep(0.25)
                ready = True
                for player in players:
                    ready &= player != None
                if ready:
                    race_state = LAP_SELECT

            print "Please select number of Laps: %d" % num_laps,
            sys.stdout.flush()
            while race_state == LAP_SELECT:
                sleep(0.25)

            print "All players are ready. Starting race in:"
            for i in range(3,0,-1):
                print i
                sender.sendAsync("Countdown", count=str(i))
                sleep(1)

            print "Go!"

            sender.sendAsync("GameStart")

            start_time = datetime.now()
            race_state = RACING;

            timer = Timer(0.1, TimeEvent)
            timer.start()
            print "Beginning event timer"

            #wait for race to finish
            while race_state == RACING:
                sleep(0.5)

            print "Waiting to restart race. (Press Start)"

            while race_state == FINISHED:
                sleep(0.25)
    except KeyboardInterrupt:
        pass

        #receiver.hibernate()
    shutdown = True
    print "Shutting down game engine."
