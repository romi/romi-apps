#!/usr/bin/env bash

if [ -e "/home/romi/rcom-registry.pid" ]; then
    pid=$(cat /home/romi/rcom-registry.pid)
    if [ -e "/proc/$pid/cmdline" ]; then
        kill -9 $pid
    fi
fi
exit 0
