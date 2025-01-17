#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include "globals.h"
#include "utils.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <math.h>

extern FILE *log_file;
extern sem_t *sem_g;
extern sem_t *sem_drone;

typedef struct
{
    WINDOW *left_split;
    WINDOW *right_split;
    WINDOW *tl_win;
    WINDOW *tc_win;
    WINDOW *tr_win;
    WINDOW *cl_win;
    WINDOW *cc_win;
    WINDOW *cr_win;
    WINDOW *bl_win;
    WINDOW *bc_win;
    WINDOW *br_win;
} WindowLayout;

extern bool resources_exist;
extern int fd;
extern int input;
extern int parent_height, parent_width;
extern int start_y_l, start_x_l, start_y_r, start_x_r;
extern Globals *globals;
extern Drone *drone;
extern WindowLayout layout;
extern Config *config;

extern pid_t child1_pid;
extern pid_t child2_pid;

WINDOW *setup_win(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void setup_resources();
void child1_task();
void child2_task();
void handle_sigint(int sig);
void format_right_win(Drone *drone_current_data, WindowLayout layout, int start_y_r, int start_x_r);
void begin_format_input(int input, WindowLayout layout);
void end_format_input(int input, WindowLayout layout);
void format_left_win(int input, WindowLayout layout);
void refresh_left_win(WindowLayout layout);
#endif // DISPLAY_UTILS_H
