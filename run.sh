#!/bin/bash

# Change to the directory containing the binaries
cd ./Binary

# Launch each binary in a new Konsole tab
konsole --new-tab -e ./server &
sleep 3
konsole --new-tab -e ./display &
sleep 1
konsole --new-tab -e ./map &
sleep 1
konsole --new-tab -e ./obstacles &
sleep 1
konsole --new-tab -e ./targets &
sleep 1
konsole --new-tab -e ./drone &
sleep 1
konsole --new-tab -e ./targets_publisher &
sleep 1
konsole --new-tab -e ./obstacles_subscriber &

echo "All binaries are running in separate Konsole tabs."