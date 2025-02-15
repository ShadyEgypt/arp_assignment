#include "map_utils.h"

FILE *log_file = NULL;
sem_t *sem_grid = NULL;
sem_t *sem_g = NULL;
sem_t *sem_drone = NULL;

bool resources_exist = false;
int map_fd = -1;

Grid *grid = NULL;
Globals *globals = NULL;
Drone *drone = NULL;
Config *config = NULL;

pid_t child1_pid = -1;
pid_t child2_pid = -1;
WINDOW *game_window;
WINDOW *instruction_window;

WINDOW *setup_win(uint h, uint w, uint startx, uint starty)
{
    WINDOW *local_win;

    local_win = newwin(h, w, starty, startx);
    return local_win;
}

void destroy_win(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(local_win);
    delwin(local_win);
}

// map functions
void setup_resources()
{
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

    map_fd = open(MAP_FIFO, O_RDONLY | O_NONBLOCK);
    if (map_fd == -1)
    {
        perror("Failed to open map pipe");
    }

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
}

void draw_game()
{
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

    // Draw Obstacles
    wattron(game_window, COLOR_PAIR(2));
    for (uint i = 0; i < grid->obstacle_count; i++)
    {
        Obstacle obstacle = grid->obstacles[i];
        x = obstacle.x;
        y = obstacle.y;
        grid->grid[obstacle.y][obstacle.x] = 255;
        mvwprintw(game_window, y, x, "O");
    }
    wattroff(game_window, COLOR_PAIR(2));
    wrefresh(game_window);

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

void child1_task()
{
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
            draw_game();
            char message[100];
            snprintf(message, sizeof(message), "%s is alive at %ld\n", "map", time(NULL));

            if (write(map_fd, message, strlen(message) + 1) == -1)
            {
                fprintf(stderr, "Error writing to %s: %s\n", MAP_FIFO, strerror(errno));
            }
        }
        usleep(100000); // Reduce CPU usage
    }
}

void handle_sigint_map(int sig)
{
    printf("\nReceived SIGINT. Cleaning up resources...\n");
    if (child1_pid > 0)
    {
        printf("Terminating child process 1 (PID: %d)\n", child1_pid);
        kill(child1_pid, SIGTERM); // Gracefully terminate child 1
    }
    if (child2_pid > 0)
    {
        printf("Terminating child process 1 (PID: %d)\n", child2_pid);
        kill(child2_pid, SIGTERM); // Gracefully terminate child 1
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
        detach_shared_memory(config, SHM_CONFIG_SIZE);
        printf("Shared memory detached and destroyed.\n");
    }
    // Cleanup ncurses
    delwin(game_window);
    delwin(game_window);
    delwin(game_window);
    endwin();
    close(map_fd);
    exit(0); // Exit the program
}
// targets and obstacles
void reset_targets(Grid *grid)
{
    if (!grid)
    {
        fprintf(stderr, "Error: Grid pointer is NULL.\n");
        return;
    }

    if (!grid->targets)
    {
        fprintf(stderr, "Error: Grid->targets pointer is NULL.\n");
        return;
    }

    grid->target_count = 0;
    memset(grid->targets, 0, sizeof(Target) * TARGETS);
    // Iterate over each cell in the 2D grid
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (grid->grid[j][i] < 250)
            {
                grid->grid[j][i] = 0; // Reset cells with the value 254
            }
        }
    }
    printf("All targets have been reset.\n");
}

void reset_obstacles(Grid *grid)
{
    if (!grid)
    {
        fprintf(stderr, "Error: Grid pointer is NULL.\n");
        return;
    }

    if (!grid->obstacles)
    {
        fprintf(stderr, "Error: Grid->obstacles pointer is NULL.\n");
        return;
    }

    grid->obstacle_count = 0; // Reset obstacle count
    memset(grid->obstacles, 0, sizeof(Obstacle) * OBSTACLES);
    // Iterate over each cell in the 2D grid
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (grid->grid[j][i] = 255)
            {
                grid->grid[j][i] = 0; // Reset cells with the value 254
            }
        }
    }
    printf("All obstacles have been reset.\n");
}

// Helper function to check if adjacent cells are occupied
bool is_adjacent_occupied(Grid *grid, uint x, uint y)
{
    uint dx[] = {-1, 1, 0, 0}; // Left, Right, Up, Down
    uint dy[] = {0, 0, -1, 1};

    for (uint i = 0; i < 4; i++)
    {
        uint nx = x + dx[i];
        uint ny = y + dy[i];

        if (nx >= 0 && nx < config->Map.Size.Width && ny >= 0 && ny < config->Map.Size.Height)
        {
            if (is_point_occupied(grid, nx, ny, config->Map.Size.Height, config->Map.Size.Width))
            {
                return true; // Adjacent point is occupied
            }
        }
    }
    return false; // No adjacent points occupied
}

void set_obstacles_randomly(Grid *grid, FILE *log_file)
{
    srand(time(NULL) + 1); // Slightly different seed for randomness

    uint placed_obstacles = 0;
    while (placed_obstacles < OBSTACLES)
    {
        uint x = (uint)(rand() % GRID_WIDTH);
        uint y = (uint)(rand() % GRID_HEIGHT);

        if (is_point_occupied(grid, x, y, GRID_HEIGHT, GRID_WIDTH) || is_adjacent_occupied(grid, x, y))
        {
            continue; // Skip occupied or adjacent points
        }

        grid->obstacles[placed_obstacles].x = x;
        grid->obstacles[placed_obstacles].y = y;
        grid->obstacle_count++;

        LOG_MESSAGE(log_file, "Placed obstacle %d at (%d, %d)", placed_obstacles, x, y);
        placed_obstacles++;
    }

    LOG_MESSAGE(log_file, "Obstacle Count: %d", grid->obstacle_count);

    LOG_MESSAGE(log_file, "%d obstacles have been randomly placed on the grid.", placed_obstacles);
}

void set_targets_randomly(Grid *grid, FILE *log_file)
{
    uint placed_targets = 0;
    uint target_id = 1;
    char log_buffer[512];
    srand(time(NULL)); // Seed the random number generator

    LOG_MESSAGE(log_file, "Starting target placement...");
    LOG_MESSAGE(log_file, "config->Map.MaxEntities.Targets: %d", TARGETS);
    while (placed_targets < TARGETS)
    {
        LOG_MESSAGE(log_file, "placed targets: %d", placed_targets);
        uint x = rand() % GRID_WIDTH;
        uint y = rand() % GRID_HEIGHT;

        LOG_MESSAGE(log_file, "Checking position (%d, %d)", x, y);

        // Skip invalid or occupied positions
        if ((x == 1 && y == 1) || is_adjacent_occupied(grid, x, y) || is_point_occupied(grid, x, y, GRID_HEIGHT, GRID_WIDTH))
        {
            LOG_MESSAGE(log_file, "Skipping invalid position (%d, %d)", x, y);
            continue;
        }

        bool is_two_digit = (target_id >= 10); // Two-digit starts from 10

        if (is_two_digit && x + 1 < GRID_WIDTH &&
            !is_point_occupied(grid, x + 1, y, GRID_HEIGHT, GRID_WIDTH) &&
            !is_adjacent_occupied(grid, x + 1, y))
        {
            // Two-digit target placement
            uint first_digit = target_id / 10;
            uint second_digit = target_id % 10;

            // Store only one entry in the targets array
            grid->targets[placed_targets].x = x;
            grid->targets[placed_targets].y = y;
            grid->targets[placed_targets].id = target_id;
            grid->targets[placed_targets].is_two_digit = true;

            LOG_MESSAGE(log_file, "Two-digit target %d placed at (%d, %d) and (%d, %d).",
                        target_id, x, y, x + 1, y);

            target_id++;
            placed_targets++;
        }
        else if (!is_two_digit && !is_point_occupied(grid, x, y, GRID_HEIGHT, GRID_WIDTH))
        {

            // Store one entry in the targets array
            grid->targets[placed_targets].x = x;
            grid->targets[placed_targets].y = y;
            grid->targets[placed_targets].id = target_id;
            grid->targets[placed_targets].is_two_digit = false;

            LOG_MESSAGE(log_file, "Single-digit target %d placed at (%d, %d).",
                        target_id, x, y);

            target_id++;
            placed_targets++;
        }

        LOG_MESSAGE(log_file, "%d targets (single and two-digit) have been sequentially placed on the grid.",
                    placed_targets);

        LOG_MESSAGE(log_file, "Target placement complete.");
        grid->target_count += 1;
    }
}
