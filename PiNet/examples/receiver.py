import os
import sys

#First, import the PiNet module
#Change directory to PiNet's location

local_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/.." % local_dir)

#Import PiNet
from PiNet import *

#Create a receiver, specify a port if you wish
pn = Receiver() #Blank defaults to 9001

#Start debug mode to see packets received
pn.debug = True

#Create a callback with any args you want
def CallbackFoo(name, age):
	print 'His name is ' + name + ', he is ' + age + ' years old.'

print 'Now listening'

#Add the callback to a tag
pn.addCallback('Test', CallbackFoo)
#You can add multiple of the same callback if you wish
pn.addCallback('Test2', CallbackFoo)

#Enter hibernate mode if you do not wish to do anything but wait for callbacks
while True:
	pn.hibernate()
