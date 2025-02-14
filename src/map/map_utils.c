#include "map_utils.h"

FILE *log_file = NULL;
sem_t *sem_grid = NULL;
sem_t *sem_g = NULL;
sem_t *sem_drone = NULL;

bool resources_exist = false;
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
