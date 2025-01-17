#ifndef CONFIG_STRUCT_H
#define CONFIG_STRUCT_H

#include <stdint.h>

// Keyboard Configuration
typedef struct
{
    int Height;
    int Width;
} KeyboardKey;

typedef struct
{
    int Height;
    int Width;
} KeyboardBox;

typedef struct
{
    KeyboardKey Key;
    KeyboardBox Box;
} KeyboardConfig;

// Map Configuration
typedef struct
{
    int Height;
    int Width;
    int TotalSize;
} MapSize;

typedef struct
{
    int Targets;
    int Obstacles;
} MapMaxEntities;

typedef struct
{
    MapSize Size;
    MapMaxEntities MaxEntities;
} MapConfig;

// Physics Configuration
typedef struct
{
    float Mass;
    float ViscousCoefficient;
    float IntegrationInterval;
} PhysicsConfig;

// Forces Configuration
typedef struct
{
    float Step;
    float Total;
    float Target;
    float Obstacle;
    float Wall;
} ForcesConfig;

// Effect Radius Configuration
typedef struct
{
    float Obstacle;
    float Target;
    float Wall;
} EffectRadiusConfig;

// Thresholds Configuration
typedef struct
{
    float MinDistance;
    float MinWallDistance;
    float ZeroThreshold;
    float MaxVelocity;
    float MaxObstacleForces;
    float MaxTargetForces;
    float MaxWallForce;
} ThresholdsConfig;

// Pipes Configuration
typedef struct
{
    char KeyboardPipe[64];
    char ServerPipe[64];
    char DronePipe[64];
} PipesConfig;

// Communication Configuration
typedef struct
{
    uint16_t MaxPayloadSize;
    uint16_t MaxResponseSize;
    struct
    {
        int Pending;
        int Processed;
    } RequestStatus;
    int TimeoutSeconds;
} CommunicationConfig;

// Main Configuration Struct
typedef struct
{
    KeyboardConfig Keyboard;
    MapConfig Map;
    PhysicsConfig Physics;
    ForcesConfig Forces;
    EffectRadiusConfig EffectRadius;
    ThresholdsConfig Thresholds;
    PipesConfig Pipes;
    CommunicationConfig Communication;
} Config;

#endif // CONFIG_STRUCT_H
