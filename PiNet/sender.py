import PiNet

PiNet.debug = True

PiNet.init(sndPort=9001, rcvPort=9002)

songData = PiNet.SongData()

def callMeMaybe():
	print 'This is crazy!'
	print songData.songName
	print songData.volume
	print songData.Time.frame
	print songData.Time.percent
	print songData.Time.second
PiNet.addCommandCallback('Crazy', callMeMaybe)

songData.songName = 'Testing!'
songData.volume = 47
songData.Time.frame = 2324
songData.Time.percent = 84
songData.Time.second = 126

PiNet.sendSongData(songData)

PiNet.sendCommand('Crazy')
