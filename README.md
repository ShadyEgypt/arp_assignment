# ARP Assignment

### 1. Server
The Server orchestrates the entire simulation, managing communications, synchronizing operations, and ensuring that resources are shared effectively among all components.

#### Key Functions:
- **Initialization**: Sets up shared resources like memory and semaphores&#8203;:contentReference[oaicite:0]{index=0}.
- **Configuration**: Parses configurations from `appsettings.json`&#8203;:contentReference[oaicite:1]{index=1}.
- **Process Handling**: Manages child processes for diverse operational needs&#8203;:contentReference[oaicite:2]{index=2}.
- **Clean-up**: Handles graceful shutdown and resource cleanup upon receiving a SIGINT signal&#8203;:contentReference[oaicite:3]{index=3}.

### 2. Drone Dynamics
Responsible for the simulation of drone movement based on inputs and interactions with the environment.

#### Key Functions:
- **Movement Control**: Processes player commands to control drone movements&#8203;:contentReference[oaicite:4]{index=4}.
- **Collision Handling**: Detects and responds to collisions with obstacles and boundaries&#8203;:contentReference[oaicite:5]{index=5}.
- **Logging**: Maintains logs for movements and interactions&#8203;:contentReference[oaicite:6]{index=6}.

### 3. Obstacle and Target Generator
Generates and manages obstacles and targets within the game environment to keep the game challenging and engaging.

#### Key Functions:
- **Obstacle Management**: Dynamically places and resets obstacles in the game area&#8203;:contentReference[oaicite:7]{index=7}.
- **Target Management**: Handles placement and resetting of targets to be captured by the drone&#8203;:contentReference[oaicite:8]{index=8}.
- **Signal Handling**: Responds to signals for resetting game elements&#8203;:contentReference[oaicite:9]{index=9}&#8203;:contentReference[oaicite:10]{index=10}.

### 4. Map
Defines the spatial environment of the game, including boundaries and navigable spaces.

#### Key Functions:
- **Grid Management**: Manages a grid that represents the game map, including the placement of the drone, obstacles, and targets&#8203;:contentReference[oaicite:11]{index=11}.
- **Display Updates**: Updates the game display in real-time to reflect current game state&#8203;:contentReference[oaicite:12]{index=12}.
- **Resize Handling**: Adjusts the display when the terminal size changes&#8203;:contentReference[oaicite:13]{index=13}.

### 5. Watchdog (Implied Process)
Monitors the system for any operational anomalies or failures, ensuring that the system remains functional and stable throughout the game play.

#### Key Functions:
- **Monitoring**: Watches over system operations to detect and log failures.
- **Recovery**: Initiates recovery processes to restore functionality after a failure.

### 6. Display (Implied Process)
Updates the visual output to the player, showing the current state of the game, including the drone’s position, obstacles, targets, and score.

#### Key Functions:
- **Real-Time Updates**: Ensures that all visual elements are updated in real-time to reflect changes in the game state.
- **User Interface**: Provides a graphical or text-based interface for user interaction.
