#-----------------------------------------------------------------------------
# update-custom-feed-RecoilOpenWRT.sh
#
# Description:
# Updates the files used to build the GAME SERVER into the FIRMWARE for the
# WIRELESS ROUTER or BASE STATION for RECOIL, from the files used to build
# the GAME SERVER on a PC with UBUNTU 16.04.
#
# Created: 03/03/17
# Authors: Rodney Quaye
#
# Copyright (c) 2017 Hotgen Ltd. All rights reserved.
#-----------------------------------------------------------------------------

SOURCE_GAME_SERVER=${1:-/media/Documents/Programming/C#/Unity_Projects/hotgen/Recoil/BaseStation/Ubuntu1604GameServer/Ubuntu1604GameServer}
TARGET_GAME_SERVER=${2:-~/RecoilOpenWRT/custom-feed/recoil/src}

./update-custom-feed.sh ${SOURCE_GAME_SERVER} ${TARGET_GAME_SERVER}
