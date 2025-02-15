#!/bin/bash

# Change to the directory containing the binaries
cd ./Binary

# Launch each binary in a new Konsole tab
konsole --new-tab -e ./server &
sleep 2
konsole --new-tab -e ./display &
sleep 2
konsole --new-tab -e ./map &
sleep 2
konsole --new-tab -e ./obstacles &
sleep 2
konsole --new-tab -e ./targets &
sleep 2
konsole --new-tab -e ./drone &
sleep 2
konsole --new-tab -e ./watchdog &

echo "All binaries are running in separate Konsole tabs."