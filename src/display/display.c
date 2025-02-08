#include "display_utils.h"

int main(int argc, char *argv[])
{
    log_file = initialize_log_file("display.txt");
    LOG_MESSAGE(log_file, "program started!");
    // Register signal handlers
    signal(SIGINT, handle_sigint);

    int shm_drone_fd = attach_shared_memory(SHM_DRONE_NAME, SHM_DRONE_SIZE);
    void *drone_addr = map_shared_memory(shm_drone_fd, SHM_DRONE_SIZE);
    drone = (Drone *)drone_addr;

    int shm_g_fd = attach_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    int shm_config_fd = attach_shared_memory(SHM_CONFIG_NAME, SHM_CONFIG_SIZE);
    void *config_addr = map_shared_memory(shm_config_fd, SHM_CONFIG_SIZE);
    config = (Config *)config_addr;

    sem_drone = open_semaphore(SEM_DRONE_NAME);
    sem_g = open_semaphore(SEM_G_NAME);

    LOG_MESSAGE(log_file, "shared memories got attached!");

    acquire_semaphore(sem_g);
    pid_t display_pid = getpid();
    globals->display_pid = display_pid;
    release_semaphore(sem_g);
    LOG_MESSAGE(log_file, "wrote my pid in the shared memory globals!");

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    LOG_MESSAGE(log_file, "ncurses initialized!");

    int grid_height = config->Keyboard.Box.Height;
    int grid_width = config->Keyboard.Box.Width;
    int cell_height = config->Keyboard.Key.Height;
    int cell_width = config->Keyboard.Key.Width;
    layout.left_split = setup_win(LINES, COLS / 2 - 1, 0, 0);
    layout.right_split = setup_win(LINES, COLS / 2 - 1, COLS / 2, 0);
    getmaxyx(layout.left_split, parent_height, parent_width);

    start_x_l = (parent_height - grid_height) / 2;
    start_y_l = (parent_width - grid_width) / 2 + 1;
    start_y_r = (parent_height - 14) / 2;
    start_x_r = (parent_width - 15) / 2;

    layout.tl_win = setup_win(cell_height, cell_width, start_y_l, start_x_l);
    layout.tc_win = setup_win(cell_height, cell_width, start_y_l + 6, start_x_l);
    layout.tr_win = setup_win(cell_height, cell_width, start_y_l + 12, start_x_l);
    layout.cl_win = setup_win(cell_height, cell_width, start_y_l, start_x_l + 4);
    layout.cc_win = setup_win(cell_height, cell_width, start_y_l + 6, start_x_l + 4);
    layout.cr_win = setup_win(cell_height, cell_width, start_y_l + 12, start_x_l + 4);
    layout.bl_win = setup_win(cell_height, cell_width, start_y_l, start_x_l + 8);
    layout.bc_win = setup_win(cell_height, cell_width, start_y_l + 6, start_x_l + 8);
    layout.br_win = setup_win(cell_height, cell_width, start_y_l + 12, start_x_l + 8);
    // Setting the "titles" of the splits
    mvwprintw(layout.left_split, 0, 1, "INPUT DISPLAY");
    mvwprintw(layout.right_split, 0, 1, "DYNAMICS DISPLAY");

    fd = open(config->Pipes.ServerPipe, O_WRONLY);
    if (fd == -1)
    {
        perror("Failed to open server pipe");
        exit(EXIT_FAILURE);
    }
    LOG_MESSAGE(log_file, "opened fd in write mode!");

    grid_height = config->Keyboard.Box.Height;
    grid_width = config->Keyboard.Box.Width;
    cell_height = config->Keyboard.Key.Height;
    cell_width = config->Keyboard.Key.Width;
    while (1)
    {
        refresh_win(layout.left_split, LINES, COLS / 2 - 1, 0, 0);
        refresh_win(layout.right_split, LINES, COLS / 2, COLS / 2, 0);
        getmaxyx(layout.left_split, parent_height, parent_width);

        start_x_l = (parent_height - grid_height) / 2;
        start_y_l = (parent_width - grid_width) / 2 + 1;
        start_y_r = (parent_height - 14) / 2;
        start_x_r = (parent_width - 15) / 2;
        refresh_win(layout.tl_win, cell_height, cell_width, start_y_l, start_x_l);
        refresh_win(layout.tc_win, cell_height, cell_width, start_y_l + 6, start_x_l);
        refresh_win(layout.tr_win, cell_height, cell_width, start_y_l + 12, start_x_l);
        refresh_win(layout.cl_win, cell_height, cell_width, start_y_l, start_x_l + 4);
        refresh_win(layout.cc_win, cell_height, cell_width, start_y_l + 6, start_x_l + 4);
        refresh_win(layout.cr_win, cell_height, cell_width, start_y_l + 12, start_x_l + 4);
        refresh_win(layout.bl_win, cell_height, cell_width, start_y_l, start_x_l + 8);
        refresh_win(layout.bc_win, cell_height, cell_width, start_y_l + 6, start_x_l + 8);
        refresh_win(layout.br_win, cell_height, cell_width, start_y_l + 12, start_x_l + 8);
        // Setting the "titles" of the splits
        mvwprintw(layout.left_split, 0, 1, "INPUT DISPLAY");
        mvwprintw(layout.right_split, 0, 1, "DYNAMICS DISPLAY");

        format_left_win(globals->input, layout);
        format_right_win(drone, layout, start_y_r, start_x_r);
        // Refreshing all the windows
        wrefresh(layout.right_split);
        refresh_left_win(layout);

        // Getting user input if present
        input = getch();
        // Check if the input is valid (not ERR and not a control character like ESC)
        if (input == 'p' || input == 'q' || input == 'w' || input == 'e' || input == 'a' || input == 's' || input == 'd' || input == 'z' || input == 'x' || input == 'c')
        {
            globals->input = input;
            // Valid input, write it to the pipe
            if (write(fd, &input, sizeof(input)) == -1)
            {
                perror("Failed to write to server pipe");
            }
        }
        else
        {
            // No valid input or ESC pressed, skip writing to pipe
            continue;
        }
        // Log the action of writing input
        LOG_MESSAGE(log_file, "Input '%c' written to the pipe", input);
        sleep(2);
    }

    return 0;
}
