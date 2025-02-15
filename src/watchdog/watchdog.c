#include "globals.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

Globals *globals = NULL;
FILE *log_file = NULL;
IsAwake *isAwake = NULL;

// Function to terminate all tasks and exit
void terminate_server(const char *task_name)
{
    LOG_MESSAGE(log_file, "Timeout detected in %s. Terminating tasks.", task_name);
    printf("server_pid: %d\n", globals->server_pid);
    kill(globals->server_pid, SIGINT);

    fclose(log_file);
    exit(1);
}

void signal_handler(int sig)
{
    detach_shared_memory(isAwake, SHM_ISACTIVE_SIZE);
    detach_shared_memory(globals, SHM_G_SIZE);

    // Terminate server to terminate everything
    if (globals->server_pid != 0)
        kill(globals->server_pid, SIGINT);

    fclose(log_file);
    exit(1);
}

int main()
{
    log_file = initialize_log_file("watchdog.txt");
    // Set up SIGINT signal handler
    signal(SIGINT, signal_handler);

    int shm_g_fd = attach_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    int shm_isawake_fd = attach_shared_memory(SHM_ISACTIVE_NAME, SHM_ISACTIVE_SIZE);
    void *isawake_addr = map_shared_memory(shm_isawake_fd, SHM_ISACTIVE_SIZE);
    isAwake = (IsAwake *)isawake_addr;
    pid_t wd_pid = getpid();
    LOG_MESSAGE(log_file, "pid: %d", wd_pid);
    globals->watchdog_pid = wd_pid;
    while (!isAwake->start)
    {
        sleep(1);
    }
    LOG_MESSAGE(log_file, "All PIDs received. Starting monitoring loop...");

    while (1)
    {
        time_t current_time = time(NULL);

        // Check if any process has timed out (no data for more than TIMEOUT seconds)
        if (difftime(current_time, isAwake->server) > TIMEOUT)
        {
            LOG_MESSAGE(log_file, "server timeout");
            terminate_server("server");
        }
        if (difftime(current_time, isAwake->map) > TIMEOUT)
        {
            LOG_MESSAGE(log_file, "map timeout");
            terminate_server("map");
        }
        if (difftime(current_time, isAwake->display) > TIMEOUT)
        {
            LOG_MESSAGE(log_file, "display timeout");
            terminate_server("display");
        }
        if (difftime(current_time, isAwake->targets) > TIMEOUT)
        {
            LOG_MESSAGE(log_file, "targets timeout");
            terminate_server("targets");
        }
        if (difftime(current_time, isAwake->obstacles) > TIMEOUT)
        {
            LOG_MESSAGE(log_file, "obstacles timeout");
            terminate_server("obstacles");
        }
        if (difftime(current_time, isAwake->drone) > TIMEOUT)
        {
            LOG_MESSAGE(log_file, "drone timeout");
            terminate_server("drone");
        }

        // Wait for 1 second before next iteration
        sleep(10);
    }

    return 0;
}
