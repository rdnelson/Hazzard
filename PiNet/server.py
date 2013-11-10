import PiNet

PiNet.init()

songData = PiNet.SongData()

def callMeMaybe():
	print 'This is crazy!'
	print songData.songName
	print songData.volume
	print songData.Time.frame
	print songData.Time.percent
	print songData.Time.second
PiNet.addCommandCallback('Crazy', callMeMaybe)

print "Ready for data"
while True:
	pass
