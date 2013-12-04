import xml.etree.ElementTree as ET
import socket
import threading
import thread

class Data:
	def __init__(self, **kwds):
		self.__dict__.update(kwds)
	def __repr__(self):
		return str(vars(self))

LOCAL_IP = socket.gethostbyname(socket.gethostname())
MCAST_IP = '224.0.0.1'

class Sender:
	sock_send = socket.socket()
	defaultPort = 0

	debug = False

	def __init__(self, port=9001):
		self.defaultPort = port

		if self.debug: print 'Creating socket for sending'

		# Create a socket for sending
		self.sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		self.sock_send.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

		if self.debug:
			print 'Socket created'
			print 'Init finished'
			print ''

	def setDefaultPort(self, port):
		if self.debug: print 'Setting default port to ', port
		self.defaultPort = port

	def sendAsync(self, tag, port=None, **kwargs):
		if port is None: port = self.defaultPort
		if self.debug: print 'Sending asyncData ' + tag + ' to ', port
		root = ET.Element('asyncData')
		node = ET.SubElement(root, tag)
		for key, value in kwargs.iteritems():
			node.set(key, value)
		self.sock_send.sendto(ET.tostring(root), (MCAST_IP, port))
		if self.debug: print 'AsyncData sent'

	def sendUpdate(self, data, name, port=None):
		if port is None: port = self.defaultPort
		if self.debug: print 'Sending syncData: ' + name + ' to ', port
		root = ET.Element('syncData')
		dataNode = ET.SubElement(root, name)
		self.__addNodes(dataNode, vars(data))
		self.sock_send.sendto(ET.tostring(root), (MCAST_IP, port))
		if self.debug: print 'Data sent: ' + ET.tostring(root)

	def __addNodes(self, root, items):
		for name, value in items.iteritems():
			newNode = ET.SubElement(root, name)
			if type(value) == dict:
				print value
				self.__addNodes(newNode, value)
			elif isinstance(value, Data):
				self.__addNodes(newNode, vars(value))
			else:
				if type(value) == int:
					newNode.set('Type', 'Integer')
				newNode.text = str(value)

class Receiver:
	callbacks = dict()
	__data = Data()
	sock_receive = socket.socket()

	SERVER_ADDRESS = ()

	debug = False

	def __init__(self, port=9001):
		self.__setPort(port)

		if self.debug: print 'Creating socket for receiving'

		# Create the socket for receiving
		self.sock_receive.settimeout(None)
		self.sock_receive = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		self.sock_receive.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.sock_receive.bind(self.SERVER_ADDRESS)

		if self.debug: print 'Socket created, bound to ', port

		if self.debug: print 'Joining multicast group'

		# Join the multicast group
		self.sock_receive.setsockopt(socket.IPPROTO_IP,
									 socket.IP_ADD_MEMBERSHIP,
									 socket.inet_aton(MCAST_IP) + socket.inet_aton(LOCAL_IP))

		if self.debug:
			print 'Init finished'
			print ''

		self.thread = threading.Thread(target=self.receiver, args=())
		self.thread.daemon = True
		self.thread.start()

	def close(self):
		self.sock_receive.close()

	def __setPort(self, port):
		if self.debug: print 'Setting receive port to ', port
		self.SERVER_ADDRESS = ('', port)


	def receiver(self):
		while True:
			try:
				data, address = self.sock_receive.recvfrom(1024)
				if self.debug: print "Got ", data, " from ", address
				self.parse(data)
			except:
				pass

	def addCallback(self, tag, callbackFunction):
		self.callbacks.update({tag:callbackFunction})
		if self.debug: print 'Added callback: ' + tag

	def removeCallback(self, tag):
		if self.debug: 'Removing callback: ' + tag
		return self.callbacks.pop(tag,False)

	def clearCallbacks(self):
		self.callbacks.clear()
		if self.debug: 'Clearing callbacks'

	def getData(self, whatData):
		if self.debug: print 'Getting ' + whatData + ' syncData'
		return getattr(self.__data, whatData)

	def hibernate(self):
		try:
			while True:
				self.thread.join(1)
		except(KeyboardInterrupt):
			print "Waking from Hibernation"

	def parse(self, text):
		if self.debug: print 'Parsing'
		root = ET.XML(text)
		if root.tag == 'syncData':
			if self.debug: print 'Parsing syncData'
			for node in list(root):
				if self.debug: print 'Parsing syncData type: ' + node.tag
				nodeClass = Data()
				for dataNode in list(node):
					self.__addItem(nodeClass, dataNode)
				setattr(self.__data, node.tag, nodeClass)

		elif root.tag == 'asyncData':
			if self.debug: print 'Parsing asyncData'
			for node in list(root):
				tag = node.tag
				if self.debug: print 'Parsing asyncData: ' + tag
				if self.callbacks.has_key(tag):
					if self.debug: print 'Calling callback for packet: ' + node.tag
					if self.debug: print node.items()
					self.callbacks.get(tag)(**dict(node.items()))
				elif self.debug: print 'Callback not found for ' + node.tag

	def __addItem(self, root, node):
		if list(node) == []:
			if node.get('Type') == 'Integer':
				setattr(root, node.tag, int(node.text))
			else:
				setattr(root, node.tag, node.text)
		else:
			newItem = Data()
			for subItem in list(node):
				self.__addItem(newItem, subItem)
			setattr(root, node.tag, newItem)
