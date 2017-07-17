#!/bin/bash

# This is the shared directory that the output is sent to
OUT_DIR="/media/sf_recoil_shared/"

# Work out the parameters of the firmware
IN_NAME="bin/ar71xx/openwrt-ar71xx-generic-som9331-squashfs-sysupgrade.bin"
IN_BASE="firmware.bin"
IN_SIZE=$(stat -c%s "$IN_NAME")
BLOCK_SIZE=4072
IN_BLOCKS=$(( ($IN_SIZE + $BLOCK_SIZE - 1) / $BLOCK_SIZE))
IN_MD5=$(md5sum $IN_NAME | cut -c 1-32)
OUT_NAME="${OUT_DIR}fw_config.txt"
OUT_BIN="${OUT_DIR}firmware.bin"
cp $IN_NAME $OUT_BIN

# Create the config.txt file for the firmware
echo "<ota>" > $OUT_NAME
echo "<type><firmware>" >> $OUT_NAME
echo "<name><$IN_BASE>" >> $OUT_NAME
echo "<size><$IN_SIZE>" >> $OUT_NAME
echo "<path></tmp>" >> $OUT_NAME
echo "<md5sum><$IN_MD5>" >> $OUT_NAME
echo "<compressed><false>" >> $OUT_NAME
echo "<uncomp_algorithm><lzma>" >> $OUT_NAME
echo "<uncomp_size><$IN_SIZE>" >> $OUT_NAME
echo "<uncomp_md5sum><$IN_MD5>" >> $OUT_NAME
echo "<uncomp_path></tmp>" >> $OUT_NAME
echo "<uncomp_name><unfirmware.bin>" >> $OUT_NAME
echo "<block_count><$IN_BLOCKS>" >> $OUT_NAME
echo "<block_size><$BLOCK_SIZE>" >> $OUT_NAME
echo "</ota>" >> $OUT_NAME


# Work out the parameters of the package
IN_NAME=$(ls bin/ar71xx/packages/custom/recoil_*.ipk)
IN_BASE=$(ls bin/ar71xx/packages/custom/recoil_*.ipk | xargs -n 1 basename)
IN_SIZE=$(stat -c%s "$IN_NAME")
BLOCK_SIZE=4072
IN_BLOCKS=$(( ($IN_SIZE + $BLOCK_SIZE - 1) / $BLOCK_SIZE))
IN_MD5=$(md5sum $IN_NAME | cut -c 1-32)
OUT_NAME="${OUT_DIR}config.txt"
OUT_BIN="${OUT_DIR}firmware.bytes"
cp $IN_NAME $OUT_BIN

# Create the config.txt file for the package
echo "<ota>" > $OUT_NAME
echo "<type><package>" >> $OUT_NAME
echo "<name><$IN_BASE>" >> $OUT_NAME
echo "<size><$IN_SIZE>" >> $OUT_NAME
echo "<path></tmp>" >> $OUT_NAME
echo "<md5sum><$IN_MD5>" >> $OUT_NAME
echo "<compressed><false>" >> $OUT_NAME
echo "<uncomp_algorithm><lzma>" >> $OUT_NAME
echo "<uncomp_size><$IN_SIZE>" >> $OUT_NAME
echo "<uncomp_md5sum><$IN_MD5>" >> $OUT_NAME
echo "<uncomp_path></tmp>" >> $OUT_NAME
echo "<uncomp_name><unfirmware.bin>" >> $OUT_NAME
echo "<block_count><$IN_BLOCKS>" >> $OUT_NAME
echo "<block_size><$BLOCK_SIZE>" >> $OUT_NAME
echo "</ota>" >> $OUT_NAME

