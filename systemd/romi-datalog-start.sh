#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-datalog --file /home/romi/datalog.csv > /home/romi/romi-datalog.log 2>&1 &
echo $! > /home/romi/romi-datalog.pid

exit 0
