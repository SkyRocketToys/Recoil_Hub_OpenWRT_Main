Command to compile the recoil network server :-
--------------------------------------------

gcc -DBUILD_USER=\"$(whoami)\" recoil.c ping.c utils.c sync.c discovery.c time.c event.c testapp.c -lpthread -o zserver && ./zserver

Command to run the recoil network server :-
-----------------------------------------

rajg@rajg-LinuxVM:~/Desktop/WinShare/recoil$ ./zserver 
M:main:130> Welcome to RECOIL v1.0-1 Build[Feb  1 2017 17:03:49 rajg@hotgen.com]
M:NWDISCOVERY_Start:637> Network Discovery started successfully on IP(10.10.10.32) port(50000) interface(enp0s8)...
M:NWTIME_Start:326> Network Time started successfully on port(50003)...
M:NWEVENT_Start:389> Network Event started successfully on port(50004)...
M:NWSYNC_Start:657> Network Sync started successfully on port(50001)...
M:NWPING_Start:327> Network Ping started successfully on port(50002)...
M:TESTAPP_Start:132> Event Test App started successfully...

Header parser utility 
---------------------
rajg@rajg-LinuxVM:~/Desktop/WinShare/recoil$ ./header 00804101

input header (32bits): 0x804101

 Product ID      : hex(  1) dec(  1)
 Product Version : hex(  1) dec(  1)
 Protocol ID     : hex(  1) dec(  1)
 Payload length  : hex(  8) dec(  8)

 Product Name    : RECOIL
 Protocol Name   : NetworkTime