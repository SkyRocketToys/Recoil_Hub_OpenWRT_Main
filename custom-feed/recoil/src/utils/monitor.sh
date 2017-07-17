#!/bin/sh

echo "Starting the RECOIL wlan monitor application..."

#  face   bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
#wlan0:       0       0    0    0    0     0          0         0      360       1    0    0    0     0       0          0
#br-lan:  337850    1627    0    0    0     0          0         0    47976     734    0    0    0     0       0          0


echo
echo "   face   bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed"
echo
while true
do
        cat /proc/net/dev | grep 'wlan0\|br-lan'
        sleep 10
done

