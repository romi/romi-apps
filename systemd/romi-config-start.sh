#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-config --config /home/romi/config.json > /home/romi/romi-config.log 2>&1 &
echo $! > /home/romi/romi-config.pid

exit 0
