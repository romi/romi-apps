#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/romi-registry > /home/romi/romi-registry.log 2>&1 &
echo $! > /home/romi/romi-registry.pid

exit 0
