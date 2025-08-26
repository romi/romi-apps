#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-camera > /home/romi/romi-camera.log 2>&1 &
echo $! > /home/romi/romi-camera.pid

exit 0
