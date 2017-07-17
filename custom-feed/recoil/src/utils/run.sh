#!/bin/sh

#server application file with full path, md5sum file and expected file permission
SERVER_APP_FILE=/usr/bin/recoil
SERVER_MD5_FILE=/usr/bin/recoil.md5
SERVER_APP_PERM="-rwxr-xr-x"

# backup installer to restore server application and md5sum file
BACKUP_PKG_FILE=/upgrade/recoil.ipk
BACKUP_MD5_FILE=/upgrade/recoil.md5
OPKG_COMMAND="/bin/opkg install --force-downgrade"
BACKUP=0

# Extra settings
GAMESERVER=0
APP_CRASH_CHECK=0

# ----------------------------------------------------------------------------------------

if [ -s /etc/uci-defaults/setup.sh ]; then
    echo "Setup file exist.. Recoil server will be launched by it... Exiting..."
    exit 1
fi

# ----------------------------------------------------------------------------------------

CMD="$(ps | grep recoil | grep -v grep | sed 's/.*\(root\).*/\1/')"
echo "Recoil Game Server launcher...result= ${CMD}"
if [ "${CMD}" == "" ]; then
    echo "No instance of Recoil Game Server is running launch one now..."

    if [ -x ${SERVER_APP_FILE} ]; then
        md5_exp=$(cat ${SERVER_MD5_FILE})
        md5_act=`md5sum ${SERVER_APP_FILE} | cut -f 1 -d " "`
        echo "md5_exp=${md5_exp}"
        echo "md5_act=${md5_act}"
        
        if [ "${md5_exp}" == "${md5_act}" ]; then
            echo "md5 matches the expect .. server is valid"    
            perm_exp=${SERVER_APP_PERM}
            perm_act=`ls -l ${SERVER_APP_FILE} | cut -f 1 -d " "`
            if [ "${perm_exp}" == "${perm_act}" ]; then
                echo "file permission matches the expect .. server is valid..launching server now !!"
                ash -c "sleep 25; ${SERVER_APP_FILE} >& /tmp/recoil.log" &
            else
                echo "file permission does not match the expected.. server is invalid"
                BACKUP=1
            fi
        else
            echo "md5 does not match the expected.. server is invalid"
            BACKUP=1
        fi
    else
        echo "File does not exist"
        BACKUP=1
    fi

    if [ "${BACKUP}" -eq 1 ]; then
        echo "Sanity check backup installer"
        md5_exp=$(cat ${BACKUP_MD5_FILE})
        md5_act=`md5sum ${BACKUP_PKG_FILE} | cut -f 1 -d " "`
        if [ "${md5_exp}" == "${md5_act}" ]; then
            echo "md5 matches the expect .. backup installer is valid"
            echo
            echo "Recoil server is corrupted... restoring from backup installer"
            ${OPKG_COMMAND} ${BACKUP_PKG_FILE}
            md5_new=`md5sum ${SERVER_APP_FILE} | cut -f 1 -d " "`
            echo ${md5_new} > ${SERVER_MD5_FILE}

            # try launching now
            echo "Recoil Application is restored.. try launching now !!!"
            if [ -x ${SERVER_APP_FILE} ]; then
                ash -c "sleep 25; ${SERVER_APP_FILE} >& /tmp/recoil.log" &
            else
                echo "Recoil Game Server reinstall failed, rebooting"
                reboot
            fi     
        else
            echo "Backup installer is corrupt too... rebooting base station..."
        fi
    else
        echo "Backup installation not required"
    fi    
else
    echo "An instance of Recoil Game Server is already running..."
fi

if [ "${GAMESERVER}" -eq 1 ]; then
    CMD="$(ps | grep testapp | grep -v grep | sed 's/.*\(root\).*/\1/')"
    echo "Recoil Networking Test Game server launcher...result= ${CMD}"

    if [ "${CMD}" == "" ]; then
        echo "No instance of Recoil Networking Test Game Server is running launch one now..."
        ash -c "sleep 25; /usr/bin/testapp >& /tmp/testapp.log" &
    else
        echo "An instance of Recoil Networking Test Game Server is already running..."
    fi
fi

if [ "${APP_CRASH_CHECK}" -eq 1 ]; then
    # wait a few seconds and check again that the recoil server is still up.
    sleep 30
    echo "Checking Recoil Game Server has not crashed..."
    CMD="$(ps | grep recoil | grep -v grep | sed 's/.*\(root\).*/\1/')"
    if [ "${CMD}" == "" ]; then
        echo "Recoil Game Server has crashed, trying a reinstall..."
        ${OPKG_PATH} install ${OLD_PKG_PATH}
        # try launching now
        if [ -x ${SERVER_APP_FILE} ]; then
            ash -c "sleep 25; ${SERVER_APP_FILE} >& /tmp/recoil.log" &
        else
            echo "Recoil Game Server reinstall failed, rebooting"
            reboot
        fi
    else
        echo "Recoil Game Server is running OK"
    fi
fi

# exit with non zero to preserve this script in memory for next boot
exit 1
