#include "display_utils.h"

int main(int argc, char *argv[])
{
    log_file = initialize_log_file("display.txt");
    LOG_MESSAGE(log_file, "program started!");
    // Register signal handlers
    signal(SIGINT, handle_sigint);

    setup_resources();
    // a process that displays info with ncurses
    child1_pid = fork();
    if (child1_pid == 0)
    {
        child1_task();
        exit(0);
    }

    // a process that sends the input char to the server
    child2_pid = fork();
    if (child2_pid == 0)
    {
        child2_task();
        exit(0);
    }
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);
    return 0;
}
