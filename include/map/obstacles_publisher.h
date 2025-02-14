#ifndef OBSTACLES_PUBLISHER_H
#define OBSTACLES_PUBLISHER_H

#include "Generated/src/map/ObstacleMessagePubSubTypes.hpp"
#include "Generated/src/map/ObstacleMessage.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>

#include <iostream>
#include <atomic>
#include <csignal>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

#define GRID_HEIGHT 30
#define GRID_WIDTH 100
#define OBSTACLES 15
#define TARGETS 15

struct force
{
    float x;
    float y;
};

struct pos
{
    float x;
    float y;
};

struct vel
{
    float x;
    float y;
};

typedef struct
{
    struct force user_force;
    struct force wall_force;
    struct force obs_force;
    struct force tar_force;
    struct force drone_force;
    struct pos drone_pos;
    struct pos drone_pos_1;
    struct pos drone_pos_2;
    struct vel drone_vel;
    char cmd[3];
    int cmd_front;
    int cmd_rear;
} Drone;

typedef struct
{
    unsigned char x;
    unsigned char y;
    unsigned char id; // Unique identifier for the target
    bool is_two_digit;
} Target;

typedef struct
{
    unsigned char x;
    unsigned char y;
} Obstacle;

typedef struct
{
    unsigned char grid[GRID_WIDTH][GRID_HEIGHT]; // Pointer to a 2D array
    unsigned char grid_actual_height;
    unsigned char grid_actual_width;
    struct pos drone_pos;
    unsigned char score;
    Target targets[TARGETS];
    unsigned char target_count;
    Obstacle obstacles[OBSTACLES];
    unsigned char obstacle_count;
} Grid;

typedef struct
{
    pid_t pub;
    pid_t sub;
    pid_t display_pid;
    pid_t server_pid;
    pid_t drone_pid;
    pid_t targets_pid;
    pid_t obstacles_pid;
    pid_t map_pid;
    int input;
} Globals;

// Signal Handling
void signal_handler(int sig);

#define SHM_GRID_NAME "/shared_memory_grid_b"
#define SEM_GRID_NAME "/shared_semaphore_grid_b"
#define SHM_GRID_SIZE sizeof(Grid)

#define SHM_G_NAME "/shared_memory_general_b"
#define SEM_G_NAME "/shared_semaphore_general_b"
#define SHM_G_SIZE sizeof(Globals)

#endif // OBSTACLES_PUBLISHER_H
