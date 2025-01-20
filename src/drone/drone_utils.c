#include "drone_utils.h"

// Define global variables
FILE *log_file = NULL;
FILE *repulsive_force_log_file = NULL;
sem_t *sem_grid = NULL;
sem_t *sem_g = NULL;
sem_t *sem_drone = NULL;

bool resources_exist = false;
Grid *grid = NULL;
Globals *globals = NULL;
Drone *drone = NULL;
Config *config = NULL;

pid_t child1_pid = -1;
pid_t child2_pid = -1;
pid_t drone_pid = -1;

int fd = -1;
char input = '\0';
bool force_updated = false;
bool vel_updated = false;

// Resource Setup
void setup_resources()
{
    // Attach and map shared memory
    int shm_grid_fd = attach_shared_memory(SHM_GRID_NAME, SHM_GRID_SIZE);
    void *grid_addr = map_shared_memory(shm_grid_fd, SHM_GRID_SIZE);
    grid = (Grid *)grid_addr;

    int shm_g_fd = attach_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    int shm_drone_fd = attach_shared_memory(SHM_DRONE_NAME, SHM_DRONE_SIZE);
    void *drone_addr = map_shared_memory(shm_drone_fd, SHM_DRONE_SIZE);
    drone = (Drone *)drone_addr;

    int shm_config_fd = attach_shared_memory(SHM_CONFIG_NAME, SHM_CONFIG_SIZE);
    void *config_addr = map_shared_memory(shm_config_fd, SHM_CONFIG_SIZE);
    config = (Config *)config_addr;

    srand(time(NULL));

    // Open semaphore
    sem_grid = open_semaphore(SEM_GRID_NAME);
    sem_g = open_semaphore(SEM_G_NAME);
    sem_drone = open_semaphore(SEM_DRONE_NAME);

    LOG_MESSAGE(log_file, "shared memories got attached!");

    resources_exist = true;

    drone_pid = getpid();
    globals->drone_pid = drone_pid;

    // open named pipe in read mode (non-blocking)
    fd = open(config->Pipes.DronePipe, O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open drone pipe");
    }
    LOG_MESSAGE(log_file, "opened fd in read mode!");

    // Set the drone in the shared memory to be (1,1)
    drone->drone_pos.x = 1;
    drone->drone_pos.y = 1;
    drone->drone_pos_1.x = 2;
    drone->drone_pos_1.y = 2;
    grid->drone_pos.x = 1;
    grid->drone_pos.y = 1;
    // LOG_MESSAGE(log_file, "Drone position set to (1, 1)");
    // set_grid_point(grid, DRONE, log_file, config, drone->drone_pos.x, drone->drone_pos.y, 254);
    LOG_MESSAGE(log_file, "Parent: All resources are initialized successfully.");
}

// Signal handler function for SIGINT
void handle_sigint(int sig)
{
    // if this is the parent, do this and exit, otherwise exit directly
    if (getpid() == drone_pid)
    {
        printf("\nReceived SIGINT. Cleaning up resources...\n");
        // Kill child processes if they exist
        if (child1_pid > 0)
        {
            printf("Terminating child process 1 (PID: %d)\n", child1_pid);
            kill(child1_pid, SIGTERM); // Gracefully terminate child 1
        }
        if (child2_pid > 0)
        {
            printf("Terminating child process 2 (PID: %d)\n", child2_pid);
            kill(child2_pid, SIGTERM); // Gracefully terminate child 2
        }

        // Optionally wait for child processes to ensure they exit
        if (child1_pid > 0)
        {
            waitpid(child1_pid, NULL, 0);
        }
        if (child2_pid > 0)
        {
            waitpid(child2_pid, NULL, 0);
        }

        if (resources_exist)
        {
            detach_shared_memory(grid, SHM_GRID_SIZE);
            detach_shared_memory(globals, SHM_G_SIZE);
            detach_shared_memory(drone, SHM_DRONE_SIZE);
            detach_shared_memory(config, SHM_CONFIG_SIZE);
            printf("Shared memory detached.\n");
        }
        close(fd);
        exit(0);
    }
    else
    {
        printf("Child process %d received SIGINT. Exiting.\n", getpid());
        exit(0);
    }
}

// Function to calculate the diagonal force
float diag(float side)
{
    float sqrt2_half = 0.7071;
    return side * sqrt2_half;
}

// Function to implement the decreasing of the force
float slow_down(void)
{
    return 0.f;
}

void determine_drone_position(float drone_pos_x, float drone_pos_y, float obs_x, float obs_y, int result[2])
{
    // Determine left/right
    result[0] = (drone_pos_x < obs_x) ? 1 : 0;

    // Determine top/bottom
    result[1] = (drone_pos_y < obs_y) ? 1 : 0;
}

float repulsive_force(FILE *log_file, float distance, float function_scale,
                      float area_of_effect, float vel_x, float vel_y)
{
    float result;
    float inv_distance = 1 / distance;
    float inv_area_of_effect = 1 / area_of_effect;
    float distance_squared = distance * distance;
    float velocity_magnitude = sqrt(pow(vel_x, 2) + pow(vel_y, 2));

    LOG_MESSAGE(log_file, "Inputs - distance: %f, function_scale: %f, area_of_effect: %f, vel_x: %f, vel_y: %f",
                distance, function_scale, area_of_effect, vel_x, vel_y);
    LOG_MESSAGE(log_file, "Intermediate calculations - 1/distance: %f, 1/area_of_effect: %f, distance^2: %f, velocity magnitude: %f",
                inv_distance, inv_area_of_effect, distance_squared, velocity_magnitude);

    if (distance == 0.0)
    {
        result = 0.0;
        LOG_MESSAGE(log_file, "Result - repulsive_force: %f , the distance between the drone and the object is zero", result);
        return result;
    }

    if (velocity_magnitude)
    {
        result = function_scale * (inv_distance - inv_area_of_effect) *
                 (1 / distance_squared) * velocity_magnitude;
    }
    else
    {
        result = function_scale * (inv_distance - inv_area_of_effect) *
                 (1 / distance_squared);
    }

    LOG_MESSAGE(log_file, "Result - repulsive_force: %f", result);

    return result;
}

// Update wall repulsive force with clamping
void update_wall_force(Drone *drone)
{
    LOG_MESSAGE(log_file, "UPDATING WALL FORCE");
    drone->wall_force.x = 0.0f;
    drone->wall_force.y = 0.0f;
    if (drone->drone_pos.x < config->EffectRadius.Wall)
    {
        LOG_MESSAGE(log_file, "CLOSE TO LEFT WALL");
        drone->wall_force.x = repulsive_force(repulsive_force_log_file,
                                              drone->drone_pos.x, config->Forces.Wall, config->EffectRadius.Wall,
                                              drone->drone_vel.x, drone->drone_vel.y);
    }
    else if (drone->drone_pos.x > config->Map.Size.Width - config->EffectRadius.Wall)
    {
        LOG_MESSAGE(log_file, "CLOSE TO RIGHT WALL");

        drone->wall_force.y = -repulsive_force(repulsive_force_log_file,
                                               config->Map.Size.Width - drone->drone_pos.x, config->Forces.Wall,
                                               config->EffectRadius.Wall, drone->drone_vel.x, drone->drone_vel.y);
    }
    // Otherwise set it to 0
    else
    {
        drone->wall_force.x = 0;
    }

    if (drone->drone_pos.y < config->EffectRadius.Wall)
    {
        LOG_MESSAGE(log_file, "CLOSE TO TOP WALL");
        drone->wall_force.y = repulsive_force(repulsive_force_log_file,
                                              drone->drone_pos.y, config->Forces.Wall, config->EffectRadius.Wall,
                                              drone->drone_vel.x, drone->drone_vel.y);
    }
    else if (drone->drone_pos.y > config->Map.Size.Height - config->EffectRadius.Wall)
    {
        LOG_MESSAGE(log_file, "CLOSE TO BOTTOM WALL");
        drone->wall_force.y = -repulsive_force(repulsive_force_log_file,
                                               config->Map.Size.Height - drone->drone_pos.y, config->Forces.Wall,
                                               config->EffectRadius.Wall, drone->drone_vel.y, drone->drone_vel.y);
    }
    // Otherwise set it to 0
    else
    {
        drone->wall_force.y = 0;
    }

    // Clamp Wall Forces
    drone->wall_force.x = fminf(fmaxf(drone->wall_force.x, -config->Thresholds.MaxWallForce), config->Thresholds.MaxWallForce);
    drone->wall_force.y = fminf(fmaxf(drone->wall_force.y, -config->Thresholds.MaxWallForce), config->Thresholds.MaxWallForce);

    // Logging
    LOG_MESSAGE(log_file, "Wall Force (Clamped): X = %.2f, Y = %.2f", drone->wall_force.x, drone->wall_force.y);
}

// Update obstacle repulsive force
void update_obstacle_force(Drone *drone, Grid *grid, int obstacles_num)
{
    LOG_MESSAGE(log_file, "UPDATING OBSTACLES FORCE");
    int result[2];
    drone->obs_force.x = 0.0f;
    drone->obs_force.y = 0.0f;
    float drone_pos_x = grid->drone_pos.x;
    float drone_pos_y = grid->drone_pos.y;

    // log drone position
    LOG_MESSAGE(log_file, "Drone Position: X = %.2f, Y = %.2f", drone_pos_x, drone_pos_y);
    for (int i = 0; i < obstacles_num; i++)
    {
        float obs_x = grid->obstacles[i].x;
        float obs_y = grid->obstacles[i].y;
        // log obstacle position
        LOG_MESSAGE(log_file, "Obstacle %d Position: X = %.2f, Y = %.2f", i, obs_x, obs_y);
        float distance = sqrt(pow(obs_x - drone_pos_x, 2) +
                              pow(obs_y - drone_pos_y, 2));
        LOG_MESSAGE(log_file, "Distance to obstacle %d: %.2f", i, distance);
        // If it's quite close but not too much then apply the force.
        if (distance < config->EffectRadius.Obstacle && distance > 30)
        {
            double x_distance = obs_x - drone_pos_x;
            double y_distance = obs_y - drone_pos_y;

            // Compute the magnitude of the repulsive force
            double force =
                repulsive_force(repulsive_force_log_file, distance, config->Thresholds.RepulsiveForce, config->EffectRadius.Obstacle,
                                drone->drone_vel.x, drone->drone_vel.y);
            LOG_MESSAGE(log_file, "Repulsive Force for obstacle %d: %.2f", i, force);
            determine_drone_position(drone_pos_x, drone_pos_y, obs_x, obs_y, result); // Compute the direction of the repulsive force
            double angle = atan2(y_distance, x_distance);
            // the drone is on the left of the obs
            if (result[0])
            {
                drone->obs_force.x -= cos(angle) * force;
            }
            // the drone is on the right of the obs
            else
            {
                drone->obs_force.x += cos(angle) * force;
            }
            // the drone is on top of the obs
            if (result[1])
            {
                drone->obs_force.y -= sin(angle) * force;
            }
            // the drone is below the obs
            else
            {
                drone->obs_force.y += sin(angle) * force;
            }

            // Cap the force at a certain threshold.
            if (drone->obs_force.x > config->Thresholds.MaxObstacleForces)
                drone->obs_force.x = config->Thresholds.MaxObstacleForces;
            if (drone->obs_force.x < -config->Thresholds.MaxObstacleForces)
                drone->obs_force.x = -config->Thresholds.MaxObstacleForces;
            if (drone->obs_force.y > config->Thresholds.MaxObstacleForces)
                drone->obs_force.y = config->Thresholds.MaxObstacleForces;
            if (drone->obs_force.y < -config->Thresholds.MaxObstacleForces)
                drone->obs_force.y = -config->Thresholds.MaxObstacleForces;
        }
    }

    LOG_MESSAGE(log_file, "Obstacle Force: X = %.2f, Y = %.2f", drone->obs_force.x, drone->obs_force.y);
}

// Update target attractive force
void update_target_force(Drone *drone, Grid *grid, int targets_num)
{
    LOG_MESSAGE(log_file, "UPDATING TARGETS FORCE");
    int result[2];
    drone->tar_force.x = 0.0f;
    drone->tar_force.y = 0.0f;

    float drone_pos_x = grid->drone_pos.x;
    float drone_pos_y = grid->drone_pos.y;
    for (int i = 0; i < targets_num; i++)
    {
        float tar_x = grid->targets[i].x;
        float tar_y = grid->targets[i].y;
        LOG_MESSAGE(log_file, "Target %d Position: X = %.2f, Y = %.2f", i, tar_x, tar_y);
        float distance = sqrt(pow(tar_x - drone_pos_x, 2) +
                              pow(tar_y - drone_pos_y, 2));
        LOG_MESSAGE(log_file, "Distance to target %d: %.2f", i, distance);

        // If it's quite close but not too much then apply the force.
        if (distance < config->EffectRadius.Target && distance > 1)
        {
            double x_distance = tar_x - drone_pos_x;
            double y_distance = tar_y - drone_pos_y;

            // Compute the magnitude of the repulsive force
            double force =
                repulsive_force(repulsive_force_log_file, distance, config->Thresholds.RepulsiveForce, config->EffectRadius.Target,
                                drone->drone_vel.x, drone->drone_vel.y);
            LOG_MESSAGE(log_file, "Attractive Force for target %d: %.2f", i, force);

            determine_drone_position(drone_pos_x, drone_pos_y, tar_x, tar_y, result); // Compute the direction of the repulsive force
            // Compute the direction of the repulsive force
            double angle = atan2(y_distance, x_distance);

            // the drone is on the left of the tar
            if (result[0])
            {
                drone->tar_force.x += cos(angle) * force;
            }
            // the drone is on the right of the tar
            else
            {
                drone->tar_force.x -= cos(angle) * force;
            }
            // the drone is on top of the tar
            if (result[1])
            {
                drone->tar_force.y += sin(angle) * force;
            }
            // the drone is below the tar
            else
            {
                drone->tar_force.y -= sin(angle) * force;
            }

            // Cap the force at a certain threshold.
            if (drone->tar_force.x > config->Thresholds.MaxTargetForces)
                drone->tar_force.x = config->Thresholds.MaxTargetForces;
            if (drone->tar_force.x < -config->Thresholds.MaxTargetForces)
                drone->tar_force.x = -config->Thresholds.MaxTargetForces;
            if (drone->tar_force.y > config->Thresholds.MaxTargetForces)
                drone->tar_force.y = config->Thresholds.MaxTargetForces;
            if (drone->tar_force.y < -config->Thresholds.MaxTargetForces)
                drone->tar_force.y = -config->Thresholds.MaxTargetForces;
        }
    }

    LOG_MESSAGE(log_file, "Target Force: X = %.2f, Y = %.2f", drone->tar_force.x, drone->tar_force.y);
}

// Update user force based on keyboard input
void update_user_force(Drone *drone, char cmd)
{

    LOG_MESSAGE(log_file, "UPDATING USER FORCE with input '%c'", cmd);

    bool ret = true;
    float X_FORCE = 0.0f, Y_FORCE = 0.0f;

    switch (cmd)
    {
    case 'q':
        X_FORCE -= diag(config->Forces.Step);
        Y_FORCE -= diag(config->Forces.Step);
        break;
    case 'w':
        Y_FORCE -= config->Forces.Step;
        break;
    case 'e':
        X_FORCE += diag(config->Forces.Step);
        Y_FORCE -= diag(config->Forces.Step);
        break;
    case 'a':
        X_FORCE -= config->Forces.Step;
        break;
    case 's':
        X_FORCE = slow_down();
        Y_FORCE = slow_down();
        break;
    case 'd':
        X_FORCE += config->Forces.Step;
        break;
    case 'z':
        X_FORCE -= diag(config->Forces.Step);
        Y_FORCE += diag(config->Forces.Step);
        break;
    case 'x':
        Y_FORCE += config->Forces.Step;
        break;
    case 'c':
        X_FORCE += diag(config->Forces.Step);
        Y_FORCE += diag(config->Forces.Step);
        break;
    case ' ':
        X_FORCE = slow_down();
        Y_FORCE = slow_down();
        break;
    default:
        ret = false;
    }

    drone->user_force.x = X_FORCE;
    drone->user_force.y = Y_FORCE;

    LOG_MESSAGE(log_file, "User Force: X = %.2f, Y = %.2f", X_FORCE, Y_FORCE);
}

// Calculate total force from all individual forces
void calculate_total_force(Drone *drone)
{
    drone->drone_force.x = drone->user_force.x +
                           drone->wall_force.x +
                           drone->obs_force.x +
                           drone->tar_force.x;

    drone->drone_force.y = drone->user_force.y +
                           drone->wall_force.y +
                           drone->obs_force.y +
                           drone->tar_force.y;

    LOG_MESSAGE(log_file, "Total Force: X = %.2f, Y = %.2f", drone->drone_force.x, drone->drone_force.y);
}

void update_position(Drone *drone, Grid *grid)
{
    float pos_x;
    float pos_y;
    LOG_MESSAGE(log_file, "UPDATING POSITION");
    // acquire_semaphore(sem_drone);

    if (drone->drone_vel.x < config->Thresholds.ZeroThreshold &&
        drone->drone_vel.x > -config->Thresholds.ZeroThreshold &&
        drone->drone_force.x == 0.0)
    {
        drone->drone_pos.x = drone->drone_pos_1.x;
    }
    else
    {
        // Precompute constants to improve readability
        float mass_term = config->Physics.Mass / (config->Physics.IntegrationInterval * config->Physics.IntegrationInterval);
        float viscous_term = config->Physics.ViscousCoefficient / config->Physics.IntegrationInterval;
        float denominator = mass_term + viscous_term;
        float numerator = drone->drone_force.x - mass_term * (drone->drone_pos_2.x - 2 * drone->drone_pos_1.x) + viscous_term * drone->drone_pos_1.x;

        pos_x = numerator / denominator;
    }

    if (drone->drone_vel.y < config->Thresholds.ZeroThreshold &&
        drone->drone_vel.y > -config->Thresholds.ZeroThreshold &&
        drone->drone_force.y == 0.0)
    {
        drone->drone_pos.y = drone->drone_pos_1.y;
    }
    else
    {
        // Precompute constants to improve readability
        float mass_term = config->Physics.Mass / (config->Physics.IntegrationInterval * config->Physics.IntegrationInterval);
        float viscous_term = config->Physics.ViscousCoefficient / config->Physics.IntegrationInterval;
        float denominator = mass_term + viscous_term;
        float numerator = drone->drone_force.y - mass_term * (drone->drone_pos_2.y - 2 * drone->drone_pos_1.y) + viscous_term * drone->drone_pos_1.y;

        pos_y = numerator / denominator;
    }

    if (pos_x > config->Map.Size.Width)
        pos_x = config->Map.Size.Width - 1;
    else if (pos_x < 0)
        pos_x = 1;
    if (pos_y > config->Map.Size.Height)
        pos_x = config->Map.Size.Height - 1;
    else if (pos_y < 0)
        pos_y = 1;

    drone->drone_pos_2 = drone->drone_pos_1;
    drone->drone_pos_1 = drone->drone_pos;
    drone->drone_pos.x = CEIL_TO_INT(pos_x);
    drone->drone_pos.y = CEIL_TO_INT(pos_y);
    LOG_MESSAGE(log_file, "new positions: X_POS = %.2f, Y_POS = %.2f", drone->drone_pos.x, drone->drone_pos.y);

    // release_semaphore(sem_drone);
    // acquire_semaphore(sem_grid);
    set_grid_point(grid, FREE, log_file, config, drone->drone_pos_1.x, drone->drone_pos_1.y, 0);
    set_grid_point(grid, DRONE, log_file, config, drone->drone_pos.x, drone->drone_pos.y, 254);
    // release_semaphore(sem_grid);

    LOG_MESSAGE(log_file, "updated position of the drone in the grid!");
}

void update_velocity(Drone *drone)
{
    char log_msg[255];
    // acquire_semaphore(sem_drone);
    LOG_MESSAGE(log_file, "updating velocities");

    float dx_dt = (drone->drone_pos.x - drone->drone_pos_1.x) / config->Physics.IntegrationInterval;
    float dy_dt = (drone->drone_pos.y - drone->drone_pos_1.y) / config->Physics.IntegrationInterval;

    LOG_MESSAGE(log_file, "dx/dt = %.2f, dy/dt = %.2f", dx_dt, dy_dt);
    // // Cap the velocity
    drone->drone_vel.x = fminf(fmaxf(dx_dt, -config->Thresholds.MaxVelocity), config->Thresholds.MaxVelocity);
    drone->drone_vel.y = fminf(fmaxf(dy_dt, -config->Thresholds.MaxVelocity), config->Thresholds.MaxVelocity);

    drone->drone_pos_1.x = drone->drone_pos.x;
    drone->drone_pos_1.y = drone->drone_pos.y;
    drone->cmd_front = 0;
    drone->cmd_rear = 0;
    // release_semaphore(sem_drone);
    LOG_MESSAGE(log_file, "updated velocities");
}

int enqueue_cmd(Drone *drone, char command)
{
    // Check if the queue is full
    if ((drone->cmd_rear + 1) % 3 == drone->cmd_front)
    {
        LOG_MESSAGE(log_file, "Command queue is full!\n");
    }
    drone->cmd[drone->cmd_rear] = command;       // Add command to rear
    drone->cmd_rear = (drone->cmd_rear + 1) % 3; // Move rear forward (circular buffer)
    LOG_MESSAGE(log_file, "Enqueued Command: %c", command);
}

char dequeue_cmd(Drone *drone)
{
    // Check if the queue is empty
    if (drone->cmd_front == drone->cmd_rear)
    {
        LOG_MESSAGE(log_file, "Command queue is empty!\n");
    }
    char command = drone->cmd[drone->cmd_front];   // Get the front command
    drone->cmd_front = (drone->cmd_front + 1) % 3; // Move front forward (circular buffer)
    LOG_MESSAGE(log_file, "Dequeued Command: %c", command);
    return command;
}

int is_cmd_empty(Drone *drone)
{
    return (drone->cmd_front == drone->cmd_rear);
}

void child1_task()
{
    while (1)
    {
        // Read the key from the named pipe (non-blocking)
        if (read(fd, &input, sizeof(char)) <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No input available, continue looping
                usleep(100000); // Optional sleep before retrying
                continue;
            }
            else
            {
                perror("Failed to read the input key");
                exit(EXIT_FAILURE);
            }
        }

        // Check if the input is invalid (e.g., unwanted character such as apostrophe)
        if (input == '\0' || input == '\n' || input == '\r')
        {
            usleep(100000);
            continue;
        }
        if (input == 'q' || input == 'w' || input == 'e' || input == 'a' || input == 's' || input == 'd' || input == 'z' || input == 'x' || input == 'c')
        {
            acquire_semaphore(sem_drone);
            enqueue_cmd(drone, input);
            release_semaphore(sem_drone);
            printf("[child1_task] cmd set to: '%c'\n", input);

            LOG_MESSAGE(log_file, "Input '%c' received from the pipe", input);
            input = '\0';
        }

        // sleep to prevent overloading
        usleep(1000000 * config->Physics.IntegrationInterval);
    }
}

void child2_task()
{
    while (1)
    {
        while (!is_cmd_empty(drone))
        {
            LOG_MESSAGE(log_file, "Equations are processing!");
            char cmd = dequeue_cmd(drone);
            // Update forces based on the grid's targets and obstacles
            update_user_force(drone, cmd);
            update_wall_force(drone);
            update_obstacle_force(drone, grid, grid->obstacle_count);
            update_target_force(drone, grid, grid->target_count);
            calculate_total_force(drone);
            // Update drone's state
            update_position(drone, grid);
            update_velocity(drone);

            // Log state after processing
            LOG_MESSAGE(log_file, "Processed input: '%c'", cmd);
        }
        // sleep to prevent high CPU usage
        usleep(1000000 * config->Physics.IntegrationInterval);
    }
}
