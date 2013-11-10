import xml.etree.ElementTree as ET 
import socket
import struct
import sys
import thread

class SongData:
	songName = ''
	volume = 0
	class Time:
		frame = 0
		percent = 0
		second = 0
	
	time = Time()
	def __str__(self):
		songDataStr =  ('Song Data: ' +
				'\n	songName = ' + self.songName +
				'\n	volume 	 = ' + str(self.volume) +
				'\n	Time data:' +
				'\n		frame 	= ' + str(self.time.frame) +
				'\n		percent = ' + str(self.time.percent) +
				'\n		second 	= ' + str(self.time.second))
		return songDataStr
		
debug = False

commandCallbacks = dict()

songData = SongData()

MCAST_IP = '224.0.0.1'
LOCAL_IP = socket.gethostbyname(socket.gethostname())

SERVER_ADDRESS = ()
MCAST_GROUP_SEND = ()

sock_receive = socket.socket()
sock_send = socket.socket()
	
def init(rcvPort=9001, sndPort=9001):
	setPort(rcvPort, sndPort)
	global sock_receive
	global sock_send

	if debug: print 'Creating socket for receiving'
	
	# Create the socket for receiving
	sock_receive = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
	sock_receive.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	sock_receive.bind(SERVER_ADDRESS)
	
	if debug: print 'Socket created'

	if debug: print 'Joining multicast group'
	
	# Join the multicast group
	sock_receive.setsockopt(socket.IPPROTO_IP,
			socket.IP_ADD_MEMBERSHIP,
                        socket.inet_aton(MCAST_IP) + socket.inet_aton(LOCAL_IP))

	if debug: print 'Creating socket for sending'					

	# Create a socket for sending
	sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
	sock_send.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

	if debug: 
		print 'Socket created'
		print 'Init finished'
		print ''

	thread.start_new_thread(receiver, ())
	
def setPort(rcvPort, sndPort):
	global MCAST_PORT
	global SERVER_ADDRESS
	global MCAST_GROUP
	if debug: print 'Setting receive port to ', rcvPort
	SERVER_ADDRESS = ('', rcvPort)
	MCAST_GROUP = (MCAST_IP, sndPort)
	
	
def receiver():
	while True:
		try:
			data, address = sock_receive.recvfrom(1024)
			if debug: print data
			parse(data)
		except:
			pass

def addCommandCallback(command, callbackFunction):
	global commandCallbacks
	commandCallbacks.update({command:callbackFunction})
	if debug: print 'Added command, ' + command
	
def removeCommandCallback(command):
	global commandCallbacks
	if debug: 'Removing command: ' + command
	return commandCallbacks.pop(command,False)
	
def clearCommandCallbacks():
	global commandCallbacks
	commandCallbacks.clear()
	if debug: 'Clearing commands'
	
def getData(whatData):
	if debug: print 'Getting ' + whatData + ' data'
	if whatData == 'song':
		return songData
	
def sendCommand(command):
	if debug: print 'Sending command ' + command
	root = ET.Element('commands')
	ET.SubElement(root, 'command').text = command
	sock_send.sendto(ET.tostring(root), MCAST_GROUP)
	if debug: print 'Command sent'
	
def sendSongData(data):
	if debug: print 'Sending song data: ' + data.__str__()
	root = ET.Element('data')
	songData = ET.SubElement(root, 'songData')
	ET.SubElement(songData, 'songName').text = data.songName
	ET.SubElement(songData, 'volume').text = str(data.volume)
	time = ET.SubElement(songData, 'time')
	ET.SubElement(time, 'frame').text = str(data.Time.frame)
	ET.SubElement(time, 'percent').text = str(data.Time.percent)
	ET.SubElement(time, 'second').text = str(data.Time.second)
	sock_send.sendto(ET.tostring(root), MCAST_GROUP)
	if debug: print 'Data sent: ' + ET.tostring(root)

def parse(text):
	if debug: print 'Parsing'
	global songData
	root = ET.XML(text)
	if root.tag == 'data':
		if debug: print 'Parsing data'
		for node in list(root):
			if debug: print 'Parsing data type: ' + node.tag
			if node.tag == 'songData':
				songData = SongData()
				songData.songName 		= root.find('./songData/songName').text
				if songData.songName is None: songData.songName = ''
				songData.volume 		= int(root.find('./songData/volume').text)
				songData.Time.frame 	= int(root.find('./songData/time/frame').text)
				songData.Time.percent 	= int(root.find('./songData/time/percent').text)
				songData.Time.second 	= int(root.find('./songData/time/second').text)
				if debug: 
					print 'Finished parsing songData'
					print songData
			### Add any more processing here
	
	elif root.tag == 'commands':
		if debug: print 'Parsing commands'
		commands = root.findall('./command')
		for commandElem in commands:
			if debug: print 'Parsing command: ' + commandElem.text
			command = commandElem.text
			if commandCallbacks.has_key(command):
				if debug: print 'Calling callback for command: ' + commandElem.text
				commandCallbacks.get(command)()
			elif debug: print 'Command not found'
