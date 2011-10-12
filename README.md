Flowstat
========

This deamon will log all packet marked with the ULOG rule in iptables

Usage
-----

You need to be root to lunch flowstat :

    ./flowstat

or

    sudo ./flowstat

Connection telenet
------------------

telenet localhost 5454

Comande:
	"kill" : kill the deamon
	"ip" : show ip connected  
	"flux param" : where param is an ip, show all active/non flushed connection from/to param
	"stat param" : where param is an ip, show stat of param