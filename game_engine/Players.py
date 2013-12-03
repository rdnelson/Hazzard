#!/usr/bin/env python

import datetime

#Game constants
MAX_PLAYERS = 2
MAX_SPEED = 255
MIN_SPEED = -255
DEFAULT_SPEED_PERCENT = 0.75

class PlayerState:
    def __init__(self):
        self.init()

    def init(self):
        self.forward = 0
        self.reverse = 0
        self.speed_percent = DEFAULT_SPEED_PERCENT
        self.turn = 0
        self.immortal = False
        self.effects = []
        self.powerups = None
        self.turn = 0
        self.laps = []


def handleEffects(player):
    if player == None:
        return

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

    players[player].powerup == None

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
