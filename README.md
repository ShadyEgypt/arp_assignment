
# Drone Simulation Game
## Table of Contents
[Overview](#Overview)

[Tools Used in This Project](#Tools-Used-in-This-Project).

[Components](#Components)

[Gameplay](#Gameplay).

[Logging](#Logging).
## Overview

This project simulates the operation of a drone in a dynamic environment. The drone can move in any direction, reach targets, and be deflected by obstacles. The objective is to provide a working simulation that demonstrates the drone's dynamics and its interaction with various environmental factors.

## Tools Used in This Project

- **VSCode**: Used as the main development environment since our project is coded in C.
- **Ubuntu 20.04**: Recommended by the professor to enhance the experience with POSIX (Portable Operating System Interface).
- **Ncurses**: Utilized for visually displaying the project on the terminal.
- **CMake**: A powerful build system generator used to automate the build process across different platforms.

## Components

The system consists of several components, each responsible for specific tasks within the simulation:

 1. **Server**
   - The server acts as the central hub for managing communication between different components of the system, such as the drone, obstacles, and the player interface...

 2. **Drone Dynamics**
   - This component is responsible for simulating the movement of the drone. It processes player input to control the drone's movement and handles interactions with obstacles and targets.

3. **Obstacle and Target Generator**
   - Regularly generates new obstacles and targets within the game environment. This ensures that the player faces new challenges as they move the drone. The game remains dynamic and engaging with the continuous appearance of obstacles and targets.

 4. **Watchdog**
   - The watchdog monitors the system to ensure that all processes are running smoothly. In case of errors or failures within any component, it logs the issue and terminate all the processes in the most secure way.

 5. **Map**
   - Defines the environment in which the drone operates. It includes a grid or coordinate system that determines the drone's position, the placement of obstacles, and the targets. The map also helps in determining valid movement directions and boundaries for the drone.

 6. **Display**
   - The display updates in real-time to show the current state of the game. It visualizes the drone's position, the obstacles, the targets, the player's score and the interactive keyboard.

## Add how to run the code!

## Gameplay

- **Player Controls:** The player controls the drone using a keypad. The controls correspond to directional inputs or other commands to move the drone through the environment. 

|  W  |  E  |  R  |
|----|----|----|
|  S  |  D  |  F  |
|  X  |  C  |  V  |


| ↖️  |  ↑  | ↗️  |
|----|----|----|
|  ←  | 🔴  |  →  |
| ↙️  |  ↓  | ↘️  |

- **Scoring System:** Each time the player reaches a target, the score counter increases. The goal is to accumulate as many points as possible before the game ends.
- **Game End:** The game can be interrupted either when the player presses 'q' or when an error occurs in one of the processes, causing a failure. The game will continue to run as long as no critical errors occur.

## Logging

Each process within the system generates its own log file. These logs record actions performed by the components and are useful for debugging. If any error occurs in a process that does not stop it entirely, the log file helps the developer identify potential issues without disrupting the gameplay.


