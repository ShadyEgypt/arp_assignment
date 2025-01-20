#include "drone_utils.h"

int main()
{
    log_file = initialize_log_file("drone.txt");
    repulsive_force_log_file = initialize_log_file("repulsive_force.txt");
    LOG_MESSAGE(log_file, "program started!");
    signal(SIGINT, handle_sigint);

    // Shared Resources Initialization
    setup_resources();

    child1_pid = fork();
    if (child1_pid == 0)
    {
        child1_task();
        exit(0);
    }

    child2_pid = fork();
    if (child2_pid == 0)
    {
        child2_task();
        exit(0);
    }

    // Parent Process waits for all children
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);

    return 0;
}
