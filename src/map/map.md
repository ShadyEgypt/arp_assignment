# Simulation Environment Overview

![map](../../imgs/map.jpg?raw=true)

This simulation system is designed to manage a dynamic environment where obstacles and targets can be modified in real-time. The system leverages multiple processes, shared memory, semaphores, and signal handling to create an interactive map display that updates based on user interactions or predefined conditions.

  - **Color Coding**: The user interface uses specific colors to represent different elements within the application:
  - **Obstacles and Wall Points**: These are displayed in orange to distinguish them from other elements in the environment.
  - **Goals**: Goals are shown as green numbers, which can be either single-digit or two-digit, making them easily identifiable.
  - **Drone**: The drone is represented by a blue plus sign (`+`), ensuring it stands out against the background and other elements.

## Components

The system is divided into several key components:

1. **Main Process (`map.c`)** - Initializes the environment and spawns child processes.
2. **Child Processes (`obstacles.c`, `targets.c`)** - Handle specific tasks like managing obstacles and targets.
3. **Utilities (`map_utils.c`)** - Provides utility functions for shared memory management, semaphore operations, and more.

### Main Process (`map.c`)

The main process initializes the shared resources, sets up signal handlers, and starts the child processes for managing different aspects of the environment:

- **Signal Handling**: Captures `SIGINT` for graceful shutdown and `SIGWINCH` for handling terminal resize events.
- **Child Process Management**: Forks child processes and waits for them to terminate.
- **Resource Initialization**: Sets up shared memory segments and semaphores for inter-process communication.

### Map Process and Terminal Size Requirement

The map process (`map.c`) is responsible for the overall management and display of the simulation environment. A crucial aspect of this process is ensuring that the terminal size is adequate to display the entire simulation grid:

- **Terminal Size Check**: Before drawing the grid, the process checks if the terminal size is sufficient (`LINES` for rows and `COLS` for columns). If the terminal is too small, it displays a message instructing the user to resize the terminal. This check ensures that all elements of the grid are visible to the user without any truncation.
- **Display Logic**: The grid is drawn based on the dimensions specified in the shared memory configuration. This ensures that the display resolution of the simulation aligns with the size of the 2D grid vector or array, maintaining the integrity and consistency of the visual representation.


### Other Processes

#### `obstacles.c`

Manages obstacle data within the simulation. It responds to signals to reset or update obstacle positions and ensures that changes are safely communicated across processes using semaphores.

#### `targets.c`

Similar to `obstacles.c`, but focused on managing target positions within the simulation. It also responds to signals for resetting or updating targets.

### Utility Functions (`map_utils.c`)

Provides a suite of functions to facilitate operations such as:

- **Shared Memory Attach/Detach**: Functions to attach to or detach from shared memory segments.
- **Semaphore Operations**: Functions to open, acquire, and release semaphores.
- **Window Management**: Functions to create and destroy ncurses windows for displaying the game or simulation state.

## Workflow

1. **Initialization**: The main process sets up the logging, shared memory, and semaphores. It then initializes the ncurses window for displaying the map and spawns the child processes.
2. **Child Process Execution**: Each child process manages a specific aspect of the simulation (e.g., obstacles, targets). They continually check for size constraints of the terminal and update their respective elements on the map.
3. **Signal Handling**: All processes listen for specific signals (e.g., `SIGINT` for shutdown). Child processes also listen for custom signals to reset their data.
4. **Resource Cleanup**: On receiving a termination signal, processes clean up by detaching shared memory, closing semaphores, and shutting down ncurses windows.

## Detailed Process Flows for Targets and Obstacles

### Targets and Obstacles Management

The `obstacles.c` and `targets.c` files contain the processes responsible for managing the dynamic elements within the simulation grid. These processes operate independently but similarly by monitoring and updating the positions of obstacles and targets based on given conditions or signals.

#### Obstacles Process (`obstacles.c`)

- **Initialization**: Attaches to shared memory segments and opens semaphores for synchronization. This process uses these resources to read and modify the obstacle data within the grid.
- **Signal Handling**:
  - **SIGINT**: On receiving this signal, the process will detach from shared memory and exit, ensuring a clean shutdown.
  - **SIGUSR1**: This custom signal triggers the `reset_obstacles_handler`, which locks the relevant semaphore to ensure exclusive access to shared resources, resets the obstacle data, and optionally repopulates the grid with obstacles at random positions.
- **Obstacle Setting**: The function `set_obstacles_randomly` places obstacles at random positions within the grid, checking to avoid collision with other objects and ensuring they do not cluster near other obstacles.

#### Targets Process (`targets.c`)

- **Initialization**: Similar to the obstacles process, it attaches to shared memory segments and opens semaphores.
- **Signal Handling**:
  - **SIGINT**: Handles graceful shutdown by detaching from shared resources.
  - **SIGUSR1**: Activates `reset_targets_handler` which resets and randomly places targets within the grid, similar to the obstacles process but focusing on target elements.
- **Target Setting**: Targets are placed considering not just the immediate space but also ensuring there is no adjacent occupation, preserving the gameplay dynamics and difficulty.

Both processes log significant actions to a file, providing a trace of their operations which is useful for debugging and verifying the simulation's integrity.

### Conclusion

This system demonstrates a complex multi-process application where inter-process communication plays a crucial role in maintaining consistency and responsiveness of the simulation environment. The use of shared memory and semaphores allows for efficient real-time updates and coordination among processes.
