from PiNet import *

class SongData(Data):
	def __init__(self):
		self.songName = "Blank!"
		self.volume = 8
		
		self.time = Data(seconds = 10, frames = 2364, percent = 27)

pn = Sender(9001)
pn.debug = False

pn.sendAsync('Test', name='Darren', age="20")

testDict = {'name':'Darren', 'age':"20"}
pn.sendAsync('Test2', **testDict)

songData = SongData()
songData.randomItemOfData = "something..."
pn.sendUpdate(songData, 'songData')