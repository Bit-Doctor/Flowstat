Flowstat
========

This deamon will log all packet marked with the ULOG rule in iptables
The log format is a JSON tree stored in a file and loaded at every start of packetstat

Usage
-----

You need to be root to lunch flowstat :

    ./flowstat

or

    sudo ./flowstat

log
---

Packetstat log structure (JSON) :

{
"Log start":string
"Last save":string
"Log start time":int
"Last save time":int
"host ip":string
"flux":
	[
	{
	"ip":string
	"hostname":string
	"first connection":string
	"last connection":string
	"first connection time":int
	"last connection time":int
	"connections":
			[
			{
			"direction":string
			"protocole number":int
			"protocole name":string
			## in fonction of which protocol
			"port":int
			"type":string
			"type number":int
			##
			"first log":string
			"last log":string
			"first log time":int
			"last log time":int
			"number of occurancy":int
			}, ...
			]
	}, ...
	]

}
