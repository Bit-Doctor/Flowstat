##Flowstat
This deamon will analyze all packet redirected to ULOG rule in iptables
and make statistics on internet flow

#Usage
You need to be root to lunch flowstat :

    ./flowstat

or

    sudo ./flowstat

Connection telnet

    telnet localhost 5454

#option

* **-d** - activate dns resolution of ip
* **-l** - advanced output                   
* **-L** - set a limit of different ip listed (5000 by default)
* **-H** - set the size of the closed connection's history (50 by default)

#Comande
* **ip** - show ip connected 
* **connection param** - where param is an ip, show all active/non flushed connection from/to param
* **stat param** - where param is an ip, show stat of param
* **exit** - close the connection
* **kill** - kill the deamon

#Output
**Basic**

    (dns)|protocole|port/type|first packet|last packet

**Long**

    (dns)|protocole|port/type|first packet|last packet|duration|input packet|input data|input obs|input obp|output packet|output data|output obs|output obp