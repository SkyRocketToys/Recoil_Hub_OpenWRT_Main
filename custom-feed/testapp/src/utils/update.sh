#!/bin/sh

file=recoil.ipk

ps | grep recoil | grep -v grep
echo "is there a recoil app running in the system (y/Y/n/N)?"
read resp

if [[ "$resp" = "Y" ]] || [[ "$resp" = "y" ]]; then
  echo "please kill the process and rerun this script."
  echo "command :- kill -9 process_pid (ie number_before_root_in_ps_result)"
  exit 1
fi

echo "removing current version of recoil"
opkg remove recoil
rm -f $file

echo "download new version of recoil"
wget http://192.168.1.33/$file

echo "installing new version of recoil"
opkg install $file

echo "starting the new version of recoil"
recoil &

echo "recoil is running as a background process"
ps | grep recoil | grep -v grep

