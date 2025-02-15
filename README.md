# Table of contents
- [Creators](#creators)
- [ARP: Assignment 2](#arp-assignment-2)
  * [How to run](#how-to-run)
    + [Building dependencies](#building-dependencies)
    + [Command to run the program](#command-to-run-the-program)
  * [How does it work](#how-does-it-work)
    + [Architecture](#architecture)
    + [Active components](#active-components)
      - [Server](src/server/server.md)
      - [Map](src/map/map.md)
      - [Drone](src/drone/drone.md)
      - [Display](src/display/display.md)
      - [Target](src/target/target.md)
      - [Obstacles](src/obstacles/obstacles.md)
      - [Watchdog](src/watchdog/watchdog.md)

  * [Other components, directories and files](#other-components-directories-and-files)



# Creators 
Shady Abdelmalek - S7086805

Francesca Amato - S7827998

Group id: 6 

# ARP: Assignment 2
## How to run
### Building dependencies
To build this project the following dependencies are needed:
+ make
+ c compiler
+ libncurses
### Commands to compile and run the program
Simply execute the run.sh script in the main folder by typing in the shell:
```
sudo apt update
sudo apt install konsole
sudo apt install libncurses5-dev libncursesw5-dev    
chmod u+x run.sh
./run.sh
```
## How does it work
### Architecture
![architecture](imgs/arch1.jpg?raw=true)
### Active components
The active components of this project are:
- Server
- Map
- Drone
- Display
- Watchdog
- Target
- Obstacles


## Other components, directories and files
The project is structured as follows:
```
├── appsettings.json
├── Binary
│   ├── appsettings.json
│   ├── display
│   ├── drone
│   ├── logs
│   │   ├── display.txt
│   │   ├── drone.txt
│   │   ├── map.txt
│   │   ├── obstacles.txt
│   │   ├── repulsive_force.txt
│   │   ├── server.txt
│   │   ├── targets.txt
│   │   └── watchdog.txt
│   ├── map
│   ├── obstacles
│   ├── server
│   ├── targets
│   └── watchdog
├── build
│   ├── display
│   │   ├── display.o
│   │   └── display_utils.o
│   ├── drone
│   │   ├── drone.o
│   │   └── drone_utils.o
│   ├── map
│   │   ├── map.o
│   │   ├── map_utils.o
│   │   ├── obstacles.o
│   │   └── targets.o
│   ├── server
│   │   ├── server.o
│   │   └── server_utils.o
│   ├── utils.o
│   └── watchdog
│       └── watchdog.o
├── clean_logs.sh
├── include
│   ├── config_struct.h
│   ├── display
│   │   └── display_utils.h
│   ├── drone
│   │   └── drone_utils.h
│   ├── dynamics_struct.h
│   ├── globals.h
│   ├── map
│   │   └── map_utils.h
│   ├── server
│   │   └── server_utils.h
│   └── utils.h
├── logs
│   ├── display.txt
│   ├── drone.txt
│   ├── map.txt
│   ├── obstacles.txt
│   ├── repulsive_force.txt
│   ├── server.txt
│   ├── targets.txt
│   └── watchdog.txt
├── Makefile
├── README.md
├── run.sh
└── src
    ├── display
    │   ├── display.c
    │   └── display_utils.c
    ├── drone
    │   ├── drone.c
    │   ├── drone.md
    │   └── drone_utils.c
    ├── map
    │   ├── map.c
    │   ├── map.md
    │   ├── map_utils.c
    │   ├── obstacles.c
    │   └── targets.c
    ├── server
    │   ├── server.c
    │   ├── server.md
    │   └── server_utils.c
    ├── utils.c
    └── watchdog
        └── watchdog.c

20 directories, 64 files
```
