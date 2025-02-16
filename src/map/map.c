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
    // Attach and map shared memory
    int shm_grid_fd = attach_shared_memory(SHM_GRID_NAME, SHM_GRID_SIZE);
    void *grid_addr = map_shared_memory(shm_grid_fd, SHM_GRID_SIZE);
    grid = (Grid *)grid_addr;

    int shm_g_fd = attach_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    int shm_config_fd = attach_shared_memory(SHM_CONFIG_NAME, SHM_CONFIG_SIZE);
    void *config_addr = map_shared_memory(shm_config_fd, SHM_CONFIG_SIZE);
    config = (Config *)config_addr;
    srand(time(NULL));

    // Open semaphore
    sem_grid = open_semaphore(SEM_GRID_NAME);
    sem_g = open_semaphore(SEM_G_NAME);
    // g1 = sem_grid;
    // g2 = sem_g;
    // g_grid = grid;

    resources_exist = true;

    acquire_semaphore(sem_g);
    pid_t map_pid = getpid();
    globals->map_pid = map_pid;
    release_semaphore(sem_g);

    // Setting up ncurses
    initscr();
    cbreak();
    curs_set(0);
    start_color();
    init_color(10, 1000, 647, 0);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, 10, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_BLACK, COLOR_WHITE);

    while (1)
    {
        if (LINES < GRID_HEIGHT + 1 || COLS < GRID_WIDTH + 2)
        {
            if (instruction_window == NULL)
            {
                instruction_window = setup_win(3, COLS, 0, 0);
                mvwprintw(instruction_window, 1, 1, "Resize terminal to at least %dx%d to view the game correctly.", config->Map.Size.Height, config->Map.Size.Width);
            }
            wrefresh(instruction_window);
        }
        else
        {
            if (instruction_window != NULL)
            {
                destroy_win(instruction_window);
                instruction_window = NULL;
            }
            uint x, y;
            uint starty = (uint)(LINES - GRID_HEIGHT) / 2;
            uint startx = (uint)(COLS - GRID_WIDTH) / 2;
            if (game_window == NULL)
            {
                // Ensure all windows are set up
                game_window = setup_win(LINES - 1, COLS - 2, 2, 2);
            }
            wclear(game_window);
            mvprintw(0, 3, "MAP DISPLAY");
            mvprintw(0, 25, "Score: %d", grid->score);
            mvprintw(0, 50, "Press Ctrl+C to exit.");
            wrefresh(game_window);

            acquire_semaphore(sem_grid);
            // Draw Targets
            wattron(game_window, COLOR_PAIR(1));
            for (uint i = 0; i < grid->target_count; i++)
            {
                Target target = grid->targets[i];
                x = target.x;
                y = target.y;
                mvwprintw(game_window, y, x, "%d", target.id);
                if (target.is_two_digit)
                {
                    // Split two-digit target into two digits
                    uint first_digit = target.id / 10;
                    uint second_digit = target.id % 10;
                    grid->grid[y][x] = first_digit;
                    grid->grid[y][x + 1] = second_digit;
                }
                else
                {
                    // Single-digit target drawing
                    grid->grid[y][x] = target.id;
                }
            }
            wattroff(game_window, COLOR_PAIR(1));
            wrefresh(game_window);
            release_semaphore(sem_grid);

            acquire_semaphore(sem_grid);
            // Draw Obstacles
            for (uint i = 0; i < grid->obstacle_count; i++)
            {
                Obstacle obstacle = grid->obstacles[i];
                x = obstacle.x;
                y = obstacle.y;
                grid->grid[obstacle.y][obstacle.x] = 255;
                wattron(game_window, COLOR_PAIR(2));
                mvwprintw(game_window, y, x, "O");
                wattroff(game_window, COLOR_PAIR(2));
            }
            wrefresh(game_window);
            release_semaphore(sem_grid);

            // Draw walls
            // Top and bottom borders
            wattron(game_window, COLOR_PAIR(2));
            for (uint x = 0; x < GRID_WIDTH; x++)
            {
                mvwaddch(game_window, 0, x, '*');               // Top border
                mvwaddch(game_window, GRID_HEIGHT - 1, x, '*'); // Bottom border
            }

            // Left and right borders
            for (uint y = 0; y < GRID_HEIGHT; y++)
            {
                mvwaddch(game_window, y, 0, '*');              // Left border
                mvwaddch(game_window, y, GRID_WIDTH - 1, '*'); // Right border
            }
            wattroff(game_window, COLOR_PAIR(2));

            uint drone_x = grid->drone_pos.x;
            uint drone_y = grid->drone_pos.y;
            // Draw Drone (Assuming single drone at specific coordinates)
            grid->grid[drone_y][drone_x] = 254;
            wattron(game_window, COLOR_PAIR(3));
            mvwprintw(game_window, drone_y, drone_x, "+");
            wattroff(game_window, COLOR_PAIR(3));
            // Refresh the window
            wrefresh(game_window);
            refresh();
        }
        sleep(1);
    }

    return 0;
}
