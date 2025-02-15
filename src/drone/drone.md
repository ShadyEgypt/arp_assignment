# Drone Control System Documentation

## Overview

This drone control system simulates drone dynamics within a virtual environment. It utilizes inter-process communication through shared memory and semaphores to manage drone interactions with the environment and user inputs.

## System Setup

### Setup Resources (`setup_resources`)

Initializes all necessary shared resources for the simulation, including shared memory mappings for drone, grid, and configuration data, and semaphore initialization for synchronized access.

### Signal Handling (`handle_sigint`)

Ensures resources are cleaned up properly on termination by handling the SIGINT signal. It facilitates graceful shutdowns by detaching shared memory and terminating child processes efficiently.

## Dynamics and Force Calculations

### Fundamental Dynamics

The system simulates drone dynamics based on forces acting upon it from various sources. Forces are calculated using the following principles and equations:

- **Diagonal Force**:
  - Diagonal Force (F_diag) = sqrt(2)/2 * side
  - Used to calculate forces acting diagonally based on the linear distance (side).

- **Repulsive Force** (`repulsive_force`):
  - Repulsive Force (F_rep) = scale * (1/distance - 1/effect_radius) * 1/distance^2 * velocity
  - This equation is used to determine the magnitude of the repulsive force based on the inverse-square law, modified by the drone's velocity and an effectiveness radius.

### Update Functions

- **Wall Interaction** (`update_wall_force`):
  - Applies horizontal and vertical forces when the drone approaches the boundaries of the simulation grid, calculated to prevent collisions and simulate natural drone behavior when near obstacles.

- **Obstacle Interaction** (`update_obstacle_force`):
  - Computes forces based on the proximity and position of static obstacles within the grid, using the repulsive force equation to adjust the drone's trajectory.

- **Target Attraction** (`update_target_force`):
  - Similar to obstacle interaction but with attractive forces guiding the drone towards designated targets, facilitating goal-oriented movement.

- **User Input** (`update_user_force`):
  - Directly translates user commands into movement forces, allowing real-time control over the drone's movement.

### Motion Updates

- **Force Combination** (`calculate_total_force`):
  - All individual forces (user, wall, obstacle, target) are summed up to produce a resultant force vector that dictates the drone's movement.

- **Position Update** (`update_position`):
  - Based on the accumulated forces and the drone's current velocity, the position is updated using a basic kinematic equation:

- **Velocity Update** (`update_velocity`):
  - Velocity is updated considering the forces applied and the drone's previous velocity, adjusted for environmental resistance and other factors.

## Main Execution Flow

- **Child Tasks**:
  - **Input Handling** (`child1_task`): Monitors and processes user inputs to dynamically adjust drone forces based on real-time commands.
  - **Force Processing** (`child2_task`): Continuously recalculates and applies forces based on the drone's interactions with the environment and targets.

- **Main Function** (`main`):
  - Sets up the environment, spawns child processes, and waits for their completion. Implements robust signal handling to ensure proper shutdown.

## Conclusion

This system provides a robust framework for simulating drone dynamics, focusing on real-time interaction and environmental responsiveness. The detailed explanation of equations and dynamics offers a clear understanding for developers looking to enhance or customize the system further.
