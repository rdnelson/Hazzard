import PiNet

PiNet.debug = True

PiNet.init(sndPort=9001, rcvPort=9001)

def CallbackFoo(name, age):
	print 'His name was ' + name + ', he was only ' + age + ' years old...'

PiNet.addCommandCallback('Test', CallbackFoo)

PiNet.sendCommand('Test', name='Darren', age="20")

songData = PiNet.Data(songName = 'Testing', volume = 10, time = PiNet.Data(seconds = 5))

PiNet.sendData(songData, 'songData')
