#!/usr/bin/env bash

if [ -e "/home/romi/romi-registry.pid" ]; then
    pid=$(cat /home/romi/romi-registry.pid)
    if [ -e "/proc/$pid/cmdline" ]; then
        kill -9 $pid
    fi
fi
exit 0
