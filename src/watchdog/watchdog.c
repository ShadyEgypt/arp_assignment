#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "globals.h"
#include "utils.h"

#define TIMEOUT 40

Globals *globals = NULL;
int server_fd = -1, map_fd = -1, display_fd = -1, drone_fd = -1, targets_fd = -1, obstacles_fd = -1;
FILE *log_file = NULL;

// Function to terminate all tasks and exit
void terminate_server(const char *task_name)
{
    LOG_MESSAGE(log_file, "Timeout detected in %s. Terminating tasks.", task_name);
    // Terminate server to terminate everything
    if (globals->server_pid != 0)
        kill(globals->server_pid, SIGINT);

    close(server_fd);
    close(map_fd);
    close(display_fd);
    close(targets_fd);
    close(obstacles_fd);
    close(drone_fd);

    fclose(log_file);
    exit(1);
}

void signal_handler(int sig)
{
    // Terminate server to terminate everything
    if (globals->server_pid != 0)
        kill(globals->server_pid, SIGINT);

    close(server_fd);
    close(map_fd);
    close(display_fd);
    close(targets_fd);
    close(obstacles_fd);
    close(drone_fd);

    fclose(log_file);
    exit(1);
}

void openPipes()
{
    server_fd = open(SERVER_FIFO, O_RDONLY | O_NONBLOCK);
    if (server_fd == -1)
    {
        perror("Failed to open server pipe");
    }

    map_fd = open(MAP_FIFO, O_RDONLY | O_NONBLOCK);
    if (map_fd == -1)
    {
        perror("Failed to open map pipe");
    }

    display_fd = open(DISPLAY_FIFO, O_RDONLY | O_NONBLOCK);
    if (display_fd == -1)
    {
        perror("Failed to open display pipe");
    }

    targets_fd = open(TARGETS_FIFO, O_RDONLY | O_NONBLOCK);
    if (targets_fd == -1)
    {
        perror("Failed to open targets pipe");
    }

    obstacles_fd = open(OBSTACLES_FIFO, O_RDONLY | O_NONBLOCK);
    if (obstacles_fd == -1)
    {
        perror("Failed to open obstacles pipe");
    }

    drone_fd = open(DRONE_FIFO, O_RDONLY | O_NONBLOCK);
    if (drone_fd == -1)
    {
        perror("Failed to open drone pipe");
    }
}

int main()
{
    log_file = initialize_log_file("watchdog.txt");
    // Set up SIGINT signal handler
    signal(SIGINT, signal_handler);

    int shm_g_fd = attach_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    LOG_MESSAGE(log_file, "All PIDs received. Starting monitoring loop...");

    // Timestamps for last data received from each task
    time_t last_time_server = time(NULL);
    time_t last_time_map = time(NULL);
    time_t last_time_display = time(NULL);
    time_t last_time_targets = time(NULL);
    time_t last_time_obstacles = time(NULL);
    time_t last_time_drone = time(NULL);

    // Buffer to store the read data
    char buffer[20];
    ssize_t bytesRead;
    openPipes();
    // Main loop to check the status of each task every second
    while (1)
    {
        time_t current_time = time(NULL);

        // Read from server pipe
        bytesRead = read(server_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            LOG_MESSAGE(log_file, "Received from Server: ");
            last_time_server = current_time; // Reset the timestamp
        }

        // Read from map pipe
        bytesRead = read(map_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            LOG_MESSAGE(log_file, "Received from Map: ");
            last_time_map = current_time; // Reset the timestamp
        }

        // Read from display pipe
        bytesRead = read(display_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            LOG_MESSAGE(log_file, "Received from Display: ");
            last_time_display = current_time; // Reset the timestamp
        }

        // Read from targets pipe
        bytesRead = read(targets_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            LOG_MESSAGE(log_file, "Received from Targets: ");
            last_time_targets = current_time; // Reset the timestamp
        }

        // Read from obstacles pipe
        bytesRead = read(obstacles_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            LOG_MESSAGE(log_file, "Received from Obstacles: ");
            last_time_obstacles = current_time; // Reset the timestamp
        }

        // Read from drone pipe
        bytesRead = read(drone_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            LOG_MESSAGE(log_file, "Received from Drone: ");
            last_time_drone = current_time; // Reset the timestamp
        }

        // Check if any process has timed out (no data for more than TIMEOUT seconds)
        if (difftime(current_time, last_time_server) > TIMEOUT)
        {
            terminate_server('server');
        }
        if (difftime(current_time, last_time_map) > TIMEOUT)
        {
            terminate_server('map');
        }
        if (difftime(current_time, last_time_display) > TIMEOUT)
        {
            terminate_server('display');
        }
        if (difftime(current_time, last_time_targets) > TIMEOUT)
        {
            terminate_server('targets');
        }
        if (difftime(current_time, last_time_obstacles) > TIMEOUT)
        {
            terminate_server('obstacles');
        }
        if (difftime(current_time, last_time_drone) > TIMEOUT)
        {
            terminate_server('drone');
        }

        // Wait for 1 second before next iteration
        sleep(1);
    }

    return 0;
}
