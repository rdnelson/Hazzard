# get position of players array
def getPositions(players):
    player_times = {}

    for player in range(len(players)):
        if players[player] != None:
            player_times[player] = players[player].laps
        else:
            player_times[player] = []

    return _getPositions(player_times)

# get positions of player map
def _getPositions(player_times):
    times = player_times.items()
    times.sort(cmpPlayerLap)
    return map(lambda item: item[0], times) # return the key (player #) in sorted order

def cmpPlayerLap(x, y):
    return cmpLap(x[1], y[1])

def cmpLap(x, y):
    # first finished more laps
    if len(x) > len(y):
        return -1
    # second finished more laps
    if len(x) < len(y):
        return 1

    # first is slower across all laps
    if sum(x) > sum(y):
        return 1

    # first is faster across all laps
    if sum(x) < sum(y):
        return -1

    # Same laps, same time.
    return 0
