#-----------------------------------------------------------------------------
# update-custom-feed.sh
#
# Description:
# Updates the files used to build the GAME SERVER on a PC with UBUNTU 16.04
# from the files used to build the GAME SERVER into the FIRMWARE for the
# WIRELESS ROUTER or BASE STATION for RECOIL, or vice versa
#
# Created: 06/03/17
# Authors: Rodney Quaye
#
# Copyright (c) 2017 Hotgen Ltd. All rights reserved.
#-----------------------------------------------------------------------------

SOURCE_GAME_SERVER=${1:-~/RecoilOpenWRT/custom-feed/recoil/src}
TARGET_GAME_SERVER=${2:-/media/Documents/Programming/C#/Unity_Projects/hotgen/Recoil/BaseStation/Ubuntu1604GameServer/Ubuntu1604GameServer}

echo "SOURCE_GAME_SERVER = "$SOURCE_GAME_SERVER
echo "TARGET_GAME_SERVER = "$TARGET_GAME_SERVER
SOURCE_LEN=`echo $SOURCE_GAME_SERVER | wc -c`
echo ${SOURCE_LEN}


echo
echo "About to update TARGETS from SOURCE..."

sleep 5

find ${SOURCE_GAME_SERVER} -type f | grep -v ".txt" | grep -v ".o$" | grep -v x64 | grep -v vcxproj | awk '{ print "/bin/cp "$0" '${TARGET_GAME_SERVER}'"substr($0, "'${SOURCE_LEN}'") }' > /tmp/set_of_update_cmds_${USER}.bash

source /tmp/set_of_update_cmds_${USER}.bash
