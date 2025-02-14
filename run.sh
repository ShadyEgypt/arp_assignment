#!/bin/bash

# Change to the directory containing the binaries
cd ./Binary

# Launch each binary in a new Konsole tab
konsole --new-tab -e ./server &
konsole --new-tab -e ./display &
konsole --new-tab -e ./map &
konsole --new-tab -e ./obstacles &
konsole --new-tab -e ./targets &
konsole --new-tab -e ./drone &
konsole --new-tab -e ./targets_publisher &
konsole --new-tab -e ./targets_subscriber &

echo "All binaries are running in separate Konsole tabs."
