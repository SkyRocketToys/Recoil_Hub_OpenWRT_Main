#!/bin/bash

# Call this only if the version of OpenWRT is confirmed to be a good version.

# First ensure the package is up-to-date
make -j1 V=s

# Now copy the output package from the generated file to the target backup file
IN_NAME=$(ls bin/ar71xx/packages/custom/recoil_*.ipk)
IN_BASE=$(ls bin/ar71xx/packages/custom/recoil_*.ipk | xargs -n 1 basename)
if [ -f $IN_NAME ]
    then
        echo OK
    else
        echo File does not exist!
fi
IN_SIZE=$(stat -c%s "$IN_NAME")
IN_MD5=$(md5sum $IN_NAME | cut -c 1-32)
OUT_DIR=target/linux/ar71xx/base-files/upgrade/
OUT_NAME="${OUT_DIR}recoil.ipk"
OUT_MD5="${OUT_DIR}recoil.md5"
cp $IN_NAME $OUT_NAME
echo $IN_MD5 > $OUT_MD5

# Now rebuild the full firmware again
make -j1 V=s

