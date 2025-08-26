#!/usr/bin/env bash

if [ -e "/home/romi/romi-cnc.pid" ]; then
    pid=$(cat /home/romi/romi-cnc.pid)
    if [ -e "/proc/$pid/cmdline" ]; then
        kill -9 $pid
    fi
fi
exit 0
