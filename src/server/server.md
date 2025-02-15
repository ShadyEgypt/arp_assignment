# Server Overview

This document provides an overview of a server application designed to manage multiple child processes and shared resources in a synchronized manner. The application utilizes shared memory, semaphores, and signals to coordinate tasks among parent and child processes. It also features real-time interaction with external configurations via a JSON file and IPC through named pipes.

## Main Components

- **`server.c`**: Contains the main function and the primary workflow of creating child processes, handling signals, and cleaning up resources.
- **`server_utils.c`**: Includes utility functions, definitions of shared resources, and implementations of tasks for child processes.

### Initialization

At startup, the server initializes necessary shared resources and logs a start message. It configures signal handlers for `SIGINT` and `SIGUSR1`, where `SIGINT` is used for graceful shutdown and `SIGUSR1` is ignored.

### Shared Resources

Shared resources are set up using the `setup_resources()` function, which includes:
- Creating and mapping shared memory for different data structures (Grid, Globals, Drone, Config, IsAwake).
- Initializing semaphores for managing access to shared resources.
- Parsing a JSON configuration file to populate settings in the shared `Config` structure.

### Child Processes

The application creates three child processes, each with a specific task:
1. **Child 1**: Initializes grid dimensions and default values for simulation entities based on the configuration file.
2. **Child 2**: Manages IPC through named pipes to receive input commands and forward them to another process or react to specific commands (e.g., shutdown on receiving 'p').
3. **Child 3**: Periodically resets certain simulation parameters and waits for other processes to be ready before performing periodic tasks.

Each child process exits after completing its designated task, while the parent process waits for all child processes to terminate before it proceeds to clean up resources.

### Cleanup

The `cleanup_resources()` function is called upon receiving a `SIGINT`. This function:
- Closes and unlinks named pipes.
- Destroys shared memories and semaphores.
- Terminates all child processes if they are still running and ensures they have exited.
- Logs and finalizes cleanup before the parent process exits.

### Signal Handling

- **`handle_sigint`**: Invoked on `SIGINT`, it performs a comprehensive cleanup of all resources, terminates child processes, and exits the application.
- Each child process is also designed to handle `SIGINT` appropriately by exiting immediately to ensure all processes terminate cleanly.

## Conclusion

This server application is structured to manage a complex set of operations involving multiple processes and shared resources. The design emphasizes robustness and clean termination, ensuring that all resources are properly released and child processes do not remain orphaned or locked in unresponsive states.

The use of shared memory and semaphores is critical for synchronizing access to common data structures, while signal handlers allow for graceful interruption and shutdown procedures. The application's capability to parse configuration from an external file adds flexibility to its deployment and runtime behavior.
