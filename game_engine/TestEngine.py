#!/usr/bin/env python

import os
import sys
import time
import unittest

local_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/../PiNet" % local_dir)

import PiNet

#Event codes for xbox events
LR = "0"
UD = "1"
DUP = "706"
DDOWN = "707"
ACCEL = "5"
BRAKE = "2"
BUT_A = "304"
START = "315"

class EngineTestCases(unittest.TestCase):
    def setUp(self):
        self.sender = PiNet.Sender();
        self.receiver = PiNet.Receiver();

    def tearDown(self):
        self.receiver.close()
        self.receiver = None

        self.sender = None

    def PongEvent(self):
        self.engine_up = True

    def StartEvent(self):
        self.game_started = True

    def CountingEvent(self, count):
        self.countdown = int(count)

    def LapEvent(self, player, time):
        self.laps.append((int(player), float(time)))

    def PlayerFinishEvent(self, player, time):
        self.players_finished.append(int(player))

    def RaceFinishEvent(self):
        self.race_finished = True

    def testOnePlayer(self):
        """Black box test of game engine with one player"""
        self.engine_up = False
        self.game_started = False
        self.countdown = -1
        self.laps_complete = [0]
        self.laps = []
        self.players_finished = []
        self.race_finished = False

        self.receiver.addCallback("Pong", self.PongEvent)
        self.receiver.addCallback("GameStart", self.StartEvent)
        self.receiver.addCallback("Countdown", self.CountingEvent)
        self.receiver.addCallback("LapComplete", self.LapEvent)
        self.receiver.addCallback("PlayerFinish", self.PlayerFinishEvent)
        self.receiver.addCallback("RaceFinish", self.RaceFinishEvent)

        self.sender.sendAsync("Ping")
        time.sleep(0.1)
        assert self.engine_up, "Engine is not responding to pings."

        # Have player 1 join
        self.sender.sendAsync("ControllerEvent", player="0", event=BUT_A, data="1")
        # Finish player join
        self.sender.sendAsync("ControllerEvent", player="0", event=START, data="1")
        time.sleep(0.2)
        raceInfo = self.receiver.getData("RaceInfo")
        assert raceInfo.num_laps == 5, "Default lap count differs from 5"
        assert raceInfo.num_players == 1, "Player count is not 1"

        # Select 5 laps
        self.sender.sendAsync("ControllerEvent", player="0", event=START, data="1")

        while self.countdown != 1:
            time.sleep(0.25)

        # delay to allow game start to arrive
        time.sleep(1.1)

        assert self.game_started, "Game not started"

        for lap in range(raceInfo.num_laps - 1):
            # delay for lap times
            time.sleep(2+0.25*lap)
            self.sender.sendAsync("CarPassed", player="0")
            time.sleep(0.1)

            assert len(self.laps) == lap+1, "Lap %d not broadcast as finished" % (lap + 1)

        self.sender.sendAsync("CarPassed", player="0")
        time.sleep(0.1)

        assert self.players_finished == [0], "Player did not finish race"
        assert self.race_finished, "Race did not finish"

        self.sender.sendAsync("Ping")
        time.sleep(0.1)
        raceInfo = self.receiver.getData("RaceInfo")
        assert raceInfo.state == 4, "Engine not in finished state"

        #send start from different player
        self.sender.sendAsync("ControllerEvent", player="1", event=START, data="1")

        #check if the engine is now in joining state
        self.sender.sendAsync("Ping")
        time.sleep(0.1)
        raceInfo = self.receiver.getData("RaceInfo")
        assert raceInfo.state == 0, "Engine not in JOINING state"


if __name__ == "__main__":
    unittest.main()

