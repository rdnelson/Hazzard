import PiNet
import unittest
import socket
import xml.etree.ElementTree

class TestReceiver(unittest.TestCase):
	def callback(self, testArg1=None, testArg2=None):
		if testArg1 is None: self.testText = "Simple callback"
		elif testArg2 is None: self.testText = testArg1
		else: self.testText = testArg1 + testArg2
		
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
		
		self.assertFalse(self.receiver.callbacks.has_key("Test1"))
		self.assertFalse(self.receiver.callbacks.has_key("Test2"))

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
		testArg1 = "testArg data"
		self.receiver.addCallback("test", self.callback)
		self.receiver.parse("<asyncData><test testArg1='" + testArg1 + "'></test></asyncData>")
		
		self.assertEquals(self.testText, testArg1)
		
	def test_parse_asyncdata_2kwargs(self):
		testArg1 = "testArg1 data"
		testArg2 = "testArg2 data"
		self.receiver.addCallback("test", self.callback)
		self.receiver.parse("<asyncData><test testArg1='" + testArg1 + "' testArg2='" + testArg2 + "'></test></asyncData>")
		
		self.assertEquals(self.testText, testArg1 + testArg2)
		
	def test_parse_asyncdata_int(self):
		testArg1 = 1
		self.receiver.addCallback("test", self.callback)
		self.assertRaises(xml.etree.ElementTree.ParseError, self.receiver.parse, ("<asyncData><test testArg1=" + str(testArg1) + "></test></asyncData>"))

if __name__ == '__main__':
    unittest.main()