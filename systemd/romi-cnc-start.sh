#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-cnc --directory /home/romi > /home/romi/romi-cnc.log 2>&1 &
echo $! > /home/romi/romi-cnc.pid

exit 0
