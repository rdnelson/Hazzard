import PiNet

PiNet.init()

data = '<data><songData><songName>Testing!</songName><volume Type="Integer">47</volume><time><frame>2324</frame><percent>84</percent><second>126</second></time></songData></data>'

PiNet.parse(data)
songData = PiNet.data.songData
print PiNet.data