#include "map_utils.h"

void handle_resize(int sig)
{
    endwin();  // End the current ncurses window session
    refresh(); // Refresh the screen
    clear();   // Clear the screen
}

int main()
{
    log_file = initialize_log_file("map.txt");
    LOG_MESSAGE(log_file, "program started!");
    signal(SIGINT, handle_sigint_map);
    signal(SIGWINCH, handle_resize);
    setup_resources();

    // map process
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

    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);

    return 0;
}
