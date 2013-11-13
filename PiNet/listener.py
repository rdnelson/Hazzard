import PiNet

print 'Listener started'

PiNet.debug = True
PiNet.init()

print 'Now listening'

def CallbackFoo(name, age):
	print 'His name was ' + name + ', he was only ' + age + ' years old...'

PiNet.addCommandCallback('Test', CallbackFoo)

while True:
	pass
