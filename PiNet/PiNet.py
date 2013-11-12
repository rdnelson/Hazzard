import xml.etree.ElementTree as ET 
import socket
import struct
import sys
import thread

class Data:
	def __init__(self, **kwds):
		self.__dict__.update(kwds)
	def __repr__(self):
		return str(vars(self))
		
debug = False

commandCallbacks = dict()

data = Data()

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
	return getattr(data, whatData)
	
def sendCommand(command, **kwargs):
	if debug: print 'Sending command ' + command
	root = ET.Element('commands')
	commandElem = ET.SubElement(root, 'command')
	commandElem.text = command
	for key, value in kwargs.iteritems():
		commandElem.set(key, value)
	sock_send.sendto(ET.tostring(root), MCAST_GROUP)
	if debug: print 'Command sent'
	
def sendData(data, name):
	if debug: print 'Sending data: ' + name
	root = ET.Element('data')
	dataNode = ET.SubElement(root, name)
	addNodes(dataNode, vars(data))
	sock_send.sendto(ET.tostring(root), MCAST_GROUP)
	if debug: print 'Data sent: ' + ET.tostring(root)

def parse(text):
	if debug: print 'Parsing'
	global data
	root = ET.XML(text)
	if root.tag == 'data':
		if debug: print 'Parsing data'
		for node in list(root):
			if debug: print 'Parsing data type: ' + node.tag
			nodeClass = Data()
			for dataNode in list(node):
				addItem(nodeClass, dataNode)
			setattr(data, node.tag, nodeClass)
	
	elif root.tag == 'commands':
		if debug: print 'Parsing commands'
		commands = root.findall('./command')
		for commandElem in commands:
			if debug: print 'Parsing command: ' + commandElem.text
			command = commandElem.text
			if commandCallbacks.has_key(command):
				if debug: print 'Calling callback for command: ' + commandElem.text
				print commandElem.items()
				commandCallbacks.get(command)(**dict(commandElem.items()))
			elif debug: print 'Command not found'

def addItem(root, node):
	if list(node) == []:
		if node.get('Type') == 'Integer':
			setattr(root, node.tag, int(node.text))
		else:
			setattr(root, node.tag, node.text)
	else:
		newItem = Data()
		for subItem in list(node):
			addItem(newItem, subItem)
		setattr(root, node.tag, newItem)
	
def addNodes(root, items):
	for name, value in items.iteritems():
		newNode = ET.SubElement(root, name)
		if type(value) == dict:
			print value
			addNodes(newNode, value)
		elif isinstance(value, Data):
			addNodes(newNode, vars(value))
		else:
			if type(value) == int:
				newNode.set('Type', 'Integer')
			newNode.text = str(value)