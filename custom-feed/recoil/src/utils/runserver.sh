#!/bin/sh

date +%c-%H:%M:%S.%N
until ./zserver; do
    echo "Server 'zserver' crashed with exit code $?.  Respawning.." >&2
    sleep 5
    echo "---------------------------------------------------------------------------------"
    date +%c-%H:%M:%S.%N
done
