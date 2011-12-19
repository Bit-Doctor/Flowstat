##Flowstat
This deamon will analyze all packet redirected to NFLOG rule in iptables
and make statistics on internet flow

#Usage
You need to be root to start flowstat :

    ./flowstat

or

    sudo ./flowstat

Connection telnet

    telnet localhost 5454

#option

* **-c** - change the listening channel of the netlink socket (channel 0 by default)
* **-l** - set a limit of different ip listed (no limit by default)
* **-H** - set the size of the closed connection's history (50 by default)

#Comande
* **ip** - show the listed ip
* **connection param** - where param is a peer of ips, show all active/non flushed connection from/to param
* **stat param** - where param is a peer of ips, show stat of param
* **flux param** - where param is a peer of ips, show history of param
* **flush** - flush the current data
* **exit** - close the connection
* **kill** - kill the deamon

#connection output
port|input packet|input data|output packet|output data|first packet|last packet