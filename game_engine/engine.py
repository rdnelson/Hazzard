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
        self.Laps = laps
        self.JoinedPlayers = players
        self.Time = 0
        self.State = state

class PlayerInfo(PiNet.Data):
    def __init__(self, num):
        self.Number = num
        self.CurrentLap = 0
        self.RelativeTime = 0
        self.Speed = 0
        self.Turn = 0
        self.Position = 0
        self.Finished = 0

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
race_info = RaceInfo(DEFAULT_LAPS, 0, JOINING)
race_info.JoinedPlayers = 0
num_finished = 0
start_time = datetime.now()
players = []
player_infos = []
last_id = -1

# Populate the players array with nonexsistant players
for i in range(Players.MAX_PLAYERS):
    players.append(None)
    player_infos.append(None)

#Event handlers
def TimeEvent():
    global race_info

    # update effect status
    #for player in players:
        #Players.handleEffects(player)

    diff = (datetime.now() - start_time)
    race_info.Time = diff.seconds * 1000 + diff.microseconds / 1000
    sender.sendUpdate(race_info, "RaceInfo")

    if not shutdown:
        timer = Timer(0.1, TimeEvent)
        timer.start()

def ControllerEvent(player, event, data, packet_id):
    global race_info
    global players
    global last_id
    global player_infos
    packet_id = int(packet_id)

    try:
        if packet_id < last_id:
            print "Discarding packet %d, last_received %d" % (packet_id, last_id)
            return
    except Exception as e:
        print e

    last_id = packet_id

    if (DEBUG and race_info.State != LAP_SELECT):
        try:
            print "Received controller event (Player %s State %d)" % (player, race_info.State)
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

    if race_info.State == JOINING:
        if DEBUG:
            print "Joining event=%d data=%d" % (event, data)

        # If player hasn't joined and pressed A, join them
        if event == BUT_A and data == 1 and players[player] == None:
            print "Player %d is now ready" % (player + 1)
            players[player] = Players.PlayerState()
            player_infos[player] = PlayerInfo(race_info.JoinedPlayers + 1)
            race_info.JoinedPlayers += 1
            sender.sendUpdate(race_info, "RaceInfo")

        # If joined player presses start, begin lap count selection
        if event == START and data == 1 and players[player] != None:
            print "Starting with current players"
            race_info.State = LAP_SELECT
            sender.sendUpdate(race_info, "RaceInfo")

        # Ignoring all other input in this state
        elif DEBUG:
            print "Ignoring input until race begins"
        return

    elif race_info.State == LAP_SELECT:
        # If player pressed up, increase lap count
        if event == UD and data == -32767 or event == DUP and data == 1:
            race_info.Laps += 1

        # If player pressed down, decrease lap count
        elif event == UD and data == 32767 or event == DDOWN and data == 1:
            race_info.Laps -= 1

            # Ensure at least one lap
            if race_info.Laps < 1:
                race_info.Laps = 1

        # If player pressed start, begin the countdown
        elif event == START and data == 1:
            race_info.State = COUNTDOWN
            sender.sendUpdate(race_info, "RaceInfo")
            print
            return

        # Erase the current line, and return to the beginning
        print "%s[2K\r" % chr(27),

        # Prompt for number of laps
        print "Please select number of Laps: %d" % race_info.Laps,
        # Force buffer to flush (needed because no newline terminating print)
        sys.stdout.flush()

        sender.sendUpdate(race_info, "RaceInfo")

    elif race_info.State == COUNTDOWN:
        return
    elif race_info.State == FINISHED:
        if event == START and data == 1:
            # reset all the players stats
            for pl in players:
                if pl != None:
                    pl.init()
            race_info.State = JOINING
            race_info.Time = 0
            sender.sendUpdate(race_info, "RaceInfo")
            return

    if (event == ACCEL):
        players[player].forward = data
        send = True
    if (event == BRAKE):
        players[player].reverse = data
        send = True
    if (event == LR):
        send = True
        # Parse xbox controller data
        if data == -32767:
            players[player].turn = -1
        if data == 0:
            players[player].turn = 0
        if data == 32767:
            players[player].turn = 1
        print "Player turn", players[player].turn, player, player_infos
        player_infos[player].Turn == players[player].turn
        print player_infos[player].Turn
        print "updated"
    if event == BUT_A and data == 1 and players[player].powerup != None:
        # Player fired a powerup
        triggerPowerup(player)

    print "F: %d R: %d" % (players[player].forward, players[player].reverse)
    if send:
        print "Beginning of controller event"
        speed2 = players[player].forward - players[player].reverse

        #Deal with player effects
        #Players.handleEffects(player)
        players[player].speed_percent = Players.DEFAULT_SPEED_PERCENT

        speed2 = int(speed2 * players[player].speed_percent)

        #enforce speed boundaries
        if (speed2 > Players.MAX_SPEED):
            speed2 = Players.MAX_SPEED

        if (speed2 < Players.MIN_SPEED):
            speed2 = Players.MIN_SPEED

        player_infos[player].Speed = speed * 100.0 / (MAX_SPEED * DEFAULT_SPEED_PERCENT)
        sender.sendUpdate(player_infos[player], "PlayerInfo" + str(player_infos[player].Number))

        try:
            if (DEBUG):
                print "Player: %d, Speed=%d, Turn=%d" % (player, speed2, players[player].turn)
            sender.sendAsync("CarEvent", player=str(int(player_infos[player].Number)), speed=str(speed2), turn=str(players[player].turn))
            if (DEBUG):
                print "Packet Sent"
        except Exception as e:
            print e

# Used to determine engine responsiveness
def PingEvent():
    sender.sendAsync("Pong")
    sender.sendUpdate(race_info, "RaceInfo")

# Triggered when player passes through gate
def GateEvent(carNum):
    print "Player", carNum
    player = carNum
    global players
    global num_finished
    global race_info


    # Verify valid player number
    player = int(player)
    if player < 0 or player >= Players.MAX_PLAYERS:
        return

    # Verify player is joined
    if players[player] == None:
        return

    # Calculate total time
    timediff = datetime.now() - start_time
    secs = timediff.seconds + timediff.microseconds / 1000000.0

    # Calculate current lap time
    if len(players[player].laps) != 0:
        secs -= sum(players[player].laps)

    print "Player %d finished lap %d of %d in %f seconds" % (player + 1, len(players[player].laps) + 1, race_info.Laps, secs)

    # Add new lap time
    try:
        players[player].laps.append(round(secs, 3))
    except Exception as e:
        print e

    # If last lap
    if len(players[player].laps) == race_info.Laps:

        # This player finished
        sender.sendAsync("PlayerFinish", player=str(player), time=str(sum(players[player].laps)))
        print "Player %d finished." % player
        num_finished += 1
        player_infos[player].Finished = 1
        if DEBUG:
            print "# finished: %d, # total: %d" %(num_finished, race_info.JoinedPlayers)

        # If all players are finished
        if num_finished == race_info.JoinedPlayers:
            sender.sendAsync("RaceFinish")
            race_info.State = FINISHED
            print "The race is over!"
            num_finished = 0

    # If Player didn't finish
    elif len(players[player].laps) < race_info.Laps:
        sender.sendAsync("LapComplete", player=str(player), time=str(secs))

    # Update positions
    try:
        positions = laps.getPositions(players)

        player_infos[player].RelativeTime = int(round((sum(players[player].laps) - sum(players[positions[0]].laps)) * 1000, 0))
        player_infos[player].CurrentLap += 1

        # Update each player
        for p in range(Players.MAX_PLAYERS):
            if player_infos[p] != None:
                player_infos[p].Position = positions.index(p)
                sender.sendUpdate(player_infos[p], "PlayerInfo" + str(player_infos[player].Number))
                print "Sent player info"
    except Exception as e:
        print e


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
    if DEBUG:
        print "Setting up PiNet connection"

    # Register PiNet callbacks
    receiver.addCallback("ControllerEvent", ControllerEvent)
    receiver.addCallback("CarPassed", GateEvent)
    receiver.addCallback("Ping", PingEvent)

    # Create timer for time based changes (effects wearing off, etc.)
    timer = Timer(0.1, TimeEvent)

    try:
        # Game loop, should run forever (Who wants to quit?)
        while True:
            print "Waiting for players to confirm (Press A)"

            # Wait for all players to be ready
            while race_info.State == JOINING:
                sleep(0.25)
                ready = True
                for player in players:
                    ready &= player != None
                if ready:
                    race_info.State = LAP_SELECT
                    sender.sendUpdate(race_info, "RaceInfo")

            # Get number of laps
            print "Please select number of Laps: %d" % race_info.Laps,

            # Necessary because of subsequent VT100 commands
            sys.stdout.flush()

            # Sleep until ControllerEvent updates race_info.State
            while race_info.State == LAP_SELECT:
                sleep(0.25)

            # Countdown has begun
            print "All players are ready. Starting race in:"
            for i in range(3,0,-1):
                print i
                sender.sendAsync("Countdown", count=str(i))

                # Update current time to negative value for GUI
                race_info.Time = -i*1000
                sender.sendUpdate(race_info, "RaceInfo")

                sleep(1)

            # Game has begun
            print "Go!"
            start_time = datetime.now()
            sender.sendAsync("GameStart")
            race_info.State = RACING;
            sender.sendUpdate(race_info, "RaceInfo")

            # Start event timer
            timer.start()
            if DEBUG:
                print "Beginning event timer"

            #wait for race to finish
            while race_info.State == RACING:
                sleep(0.5)

            print "Waiting to restart race. (Press Start)"

            # Wait for ControllerEvent to update race_info.State
            while race_info.State == FINISHED:
                sleep(0.25)

    except KeyboardInterrupt:
        pass

    # Set flag to kill TimeEvent thread
    shutdown = True
    print "Shutting down game engine."
