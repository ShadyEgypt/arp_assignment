#include "display_utils.h"

FILE *log_file = NULL;
sem_t *sem_g = NULL;
sem_t *sem_drone = NULL;

bool resources_exist = false;
int fd = 0;
int input = 0;
int parent_height = 0, parent_width = 0;
int start_y_l = 0, start_x_l = 0, start_y_r = 0, start_x_r = 0;

Globals *globals = NULL;
Drone *drone = NULL;
WindowLayout layout;
Config *config = NULL;

pid_t child1_pid = -1;
pid_t child2_pid = -1;

// Create the outer border of the window
WINDOW *setup_win(int height, int width, int startx, int starty)
{
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    return local_win;
}

// Destroy the map window
void destroy_win(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(local_win);
    delwin(local_win);
}

void setup_resources()
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    LOG_MESSAGE(log_file, "ncurses initialized!");

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

    layout.left_split = setup_win(LINES - 1, COLS / 2 - 1, 0, 0);

    layout.right_split = setup_win(LINES - 1, COLS / 2 - 1, COLS / 2, 0);

    getmaxyx(layout.left_split, parent_height, parent_width);

    start_y_l = (parent_height - config->Keyboard.Box.Height) / 2;
    start_x_l = (parent_width - config->Keyboard.Box.Width) / 2;
    start_y_r = (parent_height - 14) / 2;
    start_x_r = (parent_width - 15) / 2;

    layout.left_split = setup_win(LINES, COLS / 2 - 1, 0, 0);
    layout.right_split = setup_win(LINES, COLS / 2 - 1, COLS / 2, 0);
    layout.tl_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l, start_x_l);
    layout.tc_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 6, start_x_l);
    layout.tr_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 12, start_x_l);
    layout.cl_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l, start_x_l + 4);
    layout.cc_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 6, start_x_l + 4);
    layout.cr_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 12, start_x_l + 4);
    layout.bl_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l, start_x_l + 8);
    layout.bc_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 6, start_x_l + 8);
    layout.br_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 12, start_x_l + 8);
    fd = open(config->Pipes.ServerPipe, O_WRONLY);
    if (fd == -1)
    {
        perror("Failed to open server pipe");
        exit(EXIT_FAILURE);
    }
    LOG_MESSAGE(log_file, "opened fd in write mode!");
};

void child1_task()
{
    int grid_height = config->Keyboard.Box.Height;
    int grid_width = config->Keyboard.Box.Width;
    int cell_height = config->Keyboard.Key.Height;
    int cell_width = config->Keyboard.Key.Width;
    while (1)
    {
        getmaxyx(layout.left_split, parent_height, parent_width);

        start_x_l = (parent_height - config->Keyboard.Box.Height) / 2;
        start_y_l = (parent_width - config->Keyboard.Box.Width) / 2 + 1;
        start_y_r = (parent_height - 14) / 2;
        start_x_r = (parent_width - 15) / 2;
        destroy_win(layout.left_split);
        destroy_win(layout.right_split);
        layout.left_split = setup_win(LINES, COLS / 2 - 1, 0, 0);
        layout.right_split = setup_win(LINES, COLS / 2 - 1, COLS / 2, 0);
        layout.tl_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l, start_x_l);
        layout.tc_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 6, start_x_l);
        layout.tr_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 12, start_x_l);
        layout.cl_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l, start_x_l + 4);
        layout.cc_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 6, start_x_l + 4);
        layout.cr_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 12, start_x_l + 4);
        layout.bl_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l, start_x_l + 8);
        layout.bc_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 6, start_x_l + 8);
        layout.br_win = setup_win(config->Keyboard.Key.Height, config->Keyboard.Key.Width, start_y_l + 12, start_x_l + 8);
        // Setting the "titles" of the splits
        mvwprintw(layout.left_split, 0, 1, "INPUT DISPLAY");
        mvwprintw(layout.right_split, 0, 1, "DYNAMICS DISPLAY");
        // snprintf(log_msg, sizeof(log_msg), );
        // LOG_MESSAGE(log_file, "Input '%c' received", globals->input);
        format_left_win(globals->input, layout);
        format_right_win(drone, layout, start_y_r, start_x_r);
        // Refreshing all the windows
        wrefresh(layout.right_split);
        refresh_left_win(layout);
        sleep(2);
    }
};

void child2_task()
{
    while (1)
    {

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
        usleep(10000);
    }
};

void handle_sigint(int sig)
{
    printf("\nReceived SIGINT. Cleaning up resources...\n");
    if (child1_pid > 0)
    {
        printf("Terminating child process 1 (PID: %d)\n", child1_pid);
        kill(child1_pid, SIGTERM); // Gracefully terminate child 1
    }
    // Optionally wait for child processes to ensure they exit
    if (child1_pid > 0)
    {
        waitpid(child1_pid, NULL, 0);
    }

    if (child2_pid > 0)
    {
        printf("Terminating child process 1 (PID: %d)\n", child1_pid);
        kill(child1_pid, SIGTERM); // Gracefully terminate child 1
    }
    // Optionally wait for child processes to ensure they exit
    if (child2_pid > 0)
    {
        waitpid(child1_pid, NULL, 0);
    }
    if (resources_exist)
    {
        detach_shared_memory(drone, SHM_DRONE_SIZE);
        detach_shared_memory(globals, SHM_G_SIZE);
        printf("Shared memory detached.\n");
    }
    close(fd);
    destroy_win(layout.left_split);
    destroy_win(layout.right_split);
    endwin();
    exit(0); // Exit the program
}

void format_right_win(Drone *drone_current_data, WindowLayout layout, int start_y_r, int start_x_r)
{
    // Displaying the current position of the drone with nice formatting
    mvwprintw(layout.right_split, start_y_r, start_x_r, "position {");
    mvwprintw(layout.right_split, start_y_r + 1, start_x_r, "\tx: %f",
              drone_current_data->drone_pos.x);
    mvwprintw(layout.right_split, start_y_r + 2, start_x_r, "\ty: %f",
              drone_current_data->drone_pos.y);
    mvwprintw(layout.right_split, start_y_r + 3, start_x_r, "}");

    // Displaying the current vel of the drone with nice formatting
    mvwprintw(layout.right_split, start_y_r + 4, start_x_r, "vel {");
    mvwprintw(layout.right_split, start_y_r + 5, start_x_r, "\tx: %f",
              drone_current_data->drone_vel.x);
    mvwprintw(layout.right_split, start_y_r + 6, start_x_r, "\ty: %f",
              drone_current_data->drone_vel.y);
    mvwprintw(layout.right_split, start_y_r + 7, start_x_r, "}");

    // Displaying the current force beeing applied on the drone only by the
    // user. So no border effects are taken into consideration while
    // displaying these values.
    mvwprintw(layout.right_split, start_y_r + 8, start_x_r, "force {");
    mvwprintw(layout.right_split, start_y_r + 9, start_x_r, "\tx: %f",
              drone_current_data->drone_force.x);
    mvwprintw(layout.right_split, start_y_r + 10, start_x_r, "\ty: %f",
              drone_current_data->drone_force.y);
    mvwprintw(layout.right_split, start_y_r + 11, start_x_r, "}");
}

// Function to set the pressed input green
void begin_format_input(int input, WindowLayout layout)
{
    switch (input)
    {
    case 'q':
        wattron(layout.tl_win, COLOR_PAIR(1));
        break;
    case 'w':
        wattron(layout.tc_win, COLOR_PAIR(1));
        break;
    case 'e':
        wattron(layout.tr_win, COLOR_PAIR(1));
        break;
    case 'a':
        wattron(layout.cl_win, COLOR_PAIR(1));
        break;
    case 's':
        wattron(layout.cc_win, COLOR_PAIR(1));
        break;
    case 'd':
        wattron(layout.cr_win, COLOR_PAIR(1));
        break;
    case 'z':
        wattron(layout.bl_win, COLOR_PAIR(1));
        break;
    case 'x':
        wattron(layout.bc_win, COLOR_PAIR(1));
        break;
    case 'c':
        wattron(layout.br_win, COLOR_PAIR(1));
        break;
    case ' ':
        wattron(layout.cc_win, COLOR_PAIR(1));
        break;
    }
}

// Function to reset the color of the pressed input
void end_format_input(int input, WindowLayout layout)
{
    switch (input)
    {
    case 'q':
        wattroff(layout.tl_win, COLOR_PAIR(1));
        break;
    case 'w':
        wattroff(layout.tc_win, COLOR_PAIR(1));
        break;
    case 'e':
        wattroff(layout.tr_win, COLOR_PAIR(1));
        break;
    case 'a':
        wattroff(layout.cl_win, COLOR_PAIR(1));
        break;
    case 's':
        wattroff(layout.cc_win, COLOR_PAIR(2));
        break;
    case 'd':
        wattroff(layout.cr_win, COLOR_PAIR(1));
        break;
    case 'z':
        wattroff(layout.bl_win, COLOR_PAIR(1));
        break;
    case 'x':
        wattroff(layout.bc_win, COLOR_PAIR(1));
        break;
    case 'c':
        wattroff(layout.br_win, COLOR_PAIR(1));
        break;
    case ' ':
        wattroff(layout.cc_win, COLOR_PAIR(1));
        break;
    }
}

void format_left_win(int input, WindowLayout layout)
{
    // Begin the coloring of the pressed key if any key is pressed
    begin_format_input(input, layout);

    /// Creating the ascii arts
    // Up-left arrow
    mvwprintw(layout.tl_win, 1, 3, "_");
    mvwprintw(layout.tl_win, 2, 2, "'\\");

    // Up arrow
    mvwprintw(layout.tc_win, 1, 3, "A");
    mvwprintw(layout.tc_win, 2, 3, "|");

    // Up-right arrow
    mvwprintw(layout.tr_win, 1, 3, "_");
    mvwprintw(layout.tr_win, 2, 3, "/'");

    // Left arrow
    mvwprintw(layout.cl_win, 2, 2, "<");
    mvwprintw(layout.cl_win, 2, 3, "-");

    // Right arrow
    mvwprintw(layout.cr_win, 2, 3, "-");
    mvwprintw(layout.cr_win, 2, 4, ">");

    // Down-left arrow
    mvwprintw(layout.bl_win, 2, 2, "|/");
    mvwprintw(layout.bl_win, 3, 2, "'-");

    // Down arrow
    mvwprintw(layout.bc_win, 2, 3, "|");
    mvwprintw(layout.bc_win, 3, 3, "V");

    // Down-right arrow
    mvwprintw(layout.br_win, 2, 3, "\\|");
    mvwprintw(layout.br_win, 3, 3, "-'");

    // Break symbol
    mvwprintw(layout.cc_win, 2, 3, "X");

    // Disable the color for the next iteration
    end_format_input(input, layout);

    // Setting the symbols for the corners of the cells
    mvwprintw(layout.tc_win, 0, 0, ".");
    mvwprintw(layout.tr_win, 0, 0, ".");
    mvwprintw(layout.cl_win, 0, 0, "+");
    mvwprintw(layout.cc_win, 0, 0, "+");
    mvwprintw(layout.cr_win, 0, 0, "+");
    mvwprintw(layout.cr_win, 0, 6, "+");
    mvwprintw(layout.bl_win, 0, 0, "+");
    mvwprintw(layout.bc_win, 0, 0, "+");
    mvwprintw(layout.br_win, 0, 0, "+");
    mvwprintw(layout.br_win, 0, 6, "+");
    mvwprintw(layout.bc_win, 4, 0, "'");
    mvwprintw(layout.br_win, 4, 0, "'");

    // Signaling what's the button to close everything
    mvwprintw(layout.left_split, LINES - 3, 3, "Press p to close everything");
}

void refresh_left_win(WindowLayout layout)
{
    wrefresh(layout.right_split);
    wrefresh(layout.left_split);
    wrefresh(layout.tl_win);
    wrefresh(layout.tc_win);
    wrefresh(layout.tr_win);
    wrefresh(layout.cl_win);
    wrefresh(layout.cc_win);
    wrefresh(layout.cr_win);
    wrefresh(layout.bl_win);
    wrefresh(layout.bc_win);
    wrefresh(layout.br_win);
}