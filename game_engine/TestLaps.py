#!/usr/bin/env python

import unittest
import laps

class MockPlayer:
    def __init__(self, laps):
        self.laps = laps

class LapsTestCases(unittest.TestCase):
    def setUp(self):

        # Player 1 is in 2nd
        # Player 2 is in 4th
        # Player 3 is in 3rd
        # player 4 is in 1st
        self.racingPlayers = [MockPlayer([1,2,3,4]),
                         MockPlayer([2,3,4]),
                         MockPlayer([2,3,4,5]),
                         MockPlayer([1,2,4,1])]

        self.startingPlayers = [MockPlayer([]),
                           MockPlayer([]),
                           MockPlayer([]),
                           MockPlayer([])]

    def tearDown(self):
        self.racingPlayers = None
        self.startingPlayers = None

    def testCmpLapEmpty(self):
        """Check that zero laps complete evaluate to be equal (0)"""
        assert laps.cmpLap([], []) == 0, "Zero laps complete are not equal"

    def testCmpLapDifferent(self):
        """Check that different number of laps is prioritized over time"""
        assert laps.cmpLap([1], []) == -1, "[1] is not less-than []"
        assert laps.cmpLap([], [1]) == 1, "[] is not greater than [1]"
        assert laps.cmpLap([1,1,1], [1,2]) == -1, "[1,1,1] is not less than [1,2]"
        assert laps.cmpLap([2,1], [1,1,1]) == 1, "[2,1] is not greater than [1,1,1]"
        assert laps.cmpLap([10,10], [1]) == -1, "[10,10] is not less than [1]"
        assert laps.cmpLap([10], [6,6]) == 1, "[10] is not greater than [6,6]"

    def testCmpLapTimes(self):
        """Check that for same lap count, time determines equality"""
        assert laps.cmpLap([1],[1]) == 0, "[1] does not equal [1]"
        assert laps.cmpLap([1],[2]) == -1, "[1] is not less than [2]"
        assert laps.cmpLap([2],[1]) == 1, "[2] is not greater than [1]"
        assert laps.cmpLap([2,3],[1,4]) == 0, "[2,3] does not equal [1,4]"
        assert laps.cmpLap([1,2],[3,3]) == -1, "[1,2] is not less than [3,3]"
        assert laps.cmpLap([3,3],[1,2]) == 1, "[3,3] is not greater than [1,2]"

    def testGetPositionsStart(self):
        """Check that getPositions functions when no laps are finished"""
        assert laps.getPositions(self.startingPlayers) == [0,1,2,3], "getPositions not performing stable sort"

    def testGetPositions(self):
        """Check that getPositions functions during a race"""
        assert laps.getPositions(self.racingPlayers) == [3,0,2,1], "Players sorted into incorrect positions"

if __name__ == "__main__":
    unittest.main()
