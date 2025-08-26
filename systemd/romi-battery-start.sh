#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-battery > /home/romi/romi-battery.log 2>&1 &
echo $! > /home/romi/romi-battery.pid

exit 0
