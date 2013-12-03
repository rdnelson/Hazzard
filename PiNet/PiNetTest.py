import PiNet
import unittest
import socket
import xml.etree.ElementTree
import time

class TestReceiver(unittest.TestCase):
	def callback(self, testArg=None, testArg2=None):
		self.testText = "Simple callback"
		if testArg is not None: self.testText = testArg
		if testArg2 is not None: self.testText += testArg2
		
	def setUp(self):
		self.testText = ""
		self.receiver = PiNet.Receiver()
		self.sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		self.sock_send.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
		
	def test_addCallback_simple(self):
		self.receiver.addCallback("Test", self.callback)
		self.assertTrue(self.receiver.callbacks.has_key("Test"))
		self.assertFalse(self.receiver.callbacks.has_key("This key does not exist!"))
		
	def test_removeCallback_simple(self):
		self.receiver.addCallback("Test", self.callback)
		self.assertTrue(self.receiver.callbacks.has_key("Test"))
		self.assertTrue(self.receiver.removeCallback("Test"))
		self.assertFalse(self.receiver.callbacks.has_key("Test"))
	def test_removeCallback_doesNotExist(self):
		self.assertFalse(self.receiver.removeCallback("This key does not exist!"))
		
	def test_clearCallbacks_simple(self):
		self.receiver.addCallback("Test1", self.callback)
		self.assertTrue(self.receiver.callbacks.has_key("Test1"))
		self.receiver.addCallback("Test2", self.callback)
		self.assertTrue(self.receiver.callbacks.has_key("Test2"))
		
		self.receiver.clearCallbacks()
		
		self.assertFalse(self.receiver.removeCallback("Test1"))
		self.assertFalse(self.receiver.removeCallback("Test2"))

	def test_parse_syncdata_simple(self):
		testData = "test!"
		self.receiver.parse("<syncData><test><data>" + testData + "</data></test></syncData>")
		self.assertEquals(self.receiver.getData("test").data, testData)
		
	def test_parse_syncdata_multiple(self):
		testData1 = "test!"
		testData2 = "testing2!"
		self.receiver.parse("<syncData><test><data1>" + testData1 + "</data1><data2>" + testData2 + "</data2></test></syncData>")
		self.assertEquals(self.receiver.getData("test").data1, testData1)
		self.assertEquals(self.receiver.getData("test").data2, testData2)
		
	def test_parse_syncdata_recursive(self):
		testData = "test!"
		self.receiver.parse("<syncData><test><recurse1><recurse2>" + testData + "</recurse2></recurse1></test></syncData>")
		self.assertEquals(self.receiver.getData("test").recurse1.recurse2, testData)
		
	def test_parse_syncdata_invalid(self):
		self.assertRaises(xml.etree.ElementTree.ParseError, self.receiver.parse, ("INVALID XML!!"))
		
	def test_parse_asyncdata_simple(self):
		self.receiver.addCallback("test", self.callback)
		self.receiver.parse("<asyncData><test></test></asyncData>")
		
		self.assertEquals(self.testText, "Simple callback")
		
	def test_parse_asyncdata_1kwarg(self):
		testArg = "testArg data"
		self.receiver.addCallback("test", self.callback)
		self.receiver.parse("<asyncData><test testArg='" + testArg + "'></test></asyncData>")
		
		self.assertEquals(self.testText, testArg)
		
	def test_parse_asyncdata_2kwargs(self):
		testArg = "testArg1 data"
		testArg2 = "testArg2 data"
		self.receiver.addCallback("test", self.callback)
		self.receiver.parse("<asyncData><test testArg='" + testArg + "' testArg2='" + testArg2 + "'></test></asyncData>")
		
		self.assertEquals(self.testText, testArg + testArg2)
		
	def test_parse_asyncdata_int(self):
		testArg = 1
		self.receiver.addCallback("test", self.callback)
		self.assertRaises(xml.etree.ElementTree.ParseError, self.receiver.parse, ("<asyncData><test testArg=" + str(testArg) + "></test></asyncData>"))
	
	def test_init_simple(self):
		testObj = PiNet.Receiver()
		self.assertEquals(testObj.SERVER_ADDRESS, ('', 9001))
		
	def test_init_setport(self):
		testObj = PiNet.Receiver(9002)
		self.assertEquals(testObj.SERVER_ADDRESS, ('', 9002))
		
	def test_receive(self):
		self.receiver.addCallback("test", self.callback)
		message = 'MessageReceived'
		self.sock_send.sendto("<asyncData><test testArg='" + message + "'></test></asyncData>", ('224.0.0.1', 9001))
		time.sleep(0.5)
		self.assertEquals(self.testText, 'MessageReceived')
		
	def test_getdata(self):
		self.sock_send.sendto("<syncData><test><testData Type='Integer'>4</testData></test></syncData>", ('224.0.0.1', 9001))
		time.sleep(0.5)
		self.assertEquals(self.receiver.getData('test').testData, 4)
		
if __name__ == '__main__':
    unittest.main()