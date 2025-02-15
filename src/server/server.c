#include "server_utils.h"

int main()
{
    log_file = initialize_log_file("server.txt");
    LOG_MESSAGE(log_file, "program started!");
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, SIG_IGN);

    // Shared Resources Initialization
    setup_resources();
    parent_pid = getpid();
    child1_pid = fork();
    if (child1_pid == 0)
    {
        child1_task();
        exit(0);
    }
    waitpid(child1_pid, NULL, 0);

    child2_pid = fork();
    if (child2_pid == 0)
    {
        child2_task();
        exit(0);
    }

    child3_pid = fork();
    if (child3_pid == 0)
    {
        child3_task();
        exit(0);
    }

    // Parent Process waits for all children

    waitpid(child2_pid, NULL, 0);
    waitpid(child3_pid, NULL, 0);

    cleanup_resources();
    return 0;
}
