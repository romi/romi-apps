#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-imu > /home/romi/romi-imu.log 2>&1 &
echo $! > /home/romi/romi-imu.pid

exit 0
