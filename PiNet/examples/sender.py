# This is an EXAMPLE of the use of PiNet
# Use this as a template for how to integrate PiNet into your project

import os
import sys

#First, import the PiNet module
#Change directory to PiNet's location

local_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/.." % local_dir)

#Import PiNet
from PiNet import *

#Create a user class, deriving from the Data class
class SongData(Data):
	volume = 8 #DOES NOT WORK, only variables are sent, not attributes
	def __init__(self):
		self.songName = "Blank!"
		
		self.time = Data(seconds = 10, frames = 2364, percent = 27)

#Create a sender, and give it a port
pn = Sender(9001)

#Start debugging mode to see all packets sent
pn.debug = True

#Send an Async packet, with two arguments
pn.sendAsync('Test', name='Darren', age="20")

#Or send the contents of a dict
testDict = {'name':'Darren', 'age':"20"}
pn.sendAsync('Test2', **testDict)

#Send an update with data
songData = SongData()

#Add anything you want to the data, it will be there
songData.randomItemOfData = "something..."

#Send the sync update
pn.sendUpdate(songData, 'songData')
