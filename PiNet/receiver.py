from PiNetJython import *

pn = Receiver() #Blank defaults to 9001
pn.debug = True

def CallbackFoo(name, age):
	print 'His name is ' + name + ', he is ' + age + ' years old.'

print 'Now listening'

pn.addCallback('Test', CallbackFoo)
pn.addCallback('Test2', CallbackFoo)

while True:
	pass
