#!/usr/bin/env bash

/home/romi/romi-apps/build/bin/rcom-registry > /home/romi/rcom-registry.log 2>&1 &
echo $! > /home/romi/rcom-registry.pid

exit 0
