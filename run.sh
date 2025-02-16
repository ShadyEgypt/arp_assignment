#!/bin/bash

cd ./Binary

./server >./logs/server.log 2>&1 &
sleep 3
./map >./logs/map.log 2>&1 &
sleep 1
./display >./logs/display.log 2>&1 &
./obstacles >./logs/obstacles.log 2>&1 &
./targets >./logs/targets.log 2>&1 &
./drone >./logs/drone.log 2>&1 &

konsole --new-tab -e ./obstacles_publisher &
sleep 1
konsole --new-tab -e ./targets_subscriber &

echo "All binaries are running in separate Konsole tabs."