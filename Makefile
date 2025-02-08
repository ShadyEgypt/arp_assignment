# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -I/usr/include/cjson -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/display -I$(INCLUDE_DIR)/drone -I$(INCLUDE_DIR)/map
LDFLAGS = -lncurses -lrt -pthread -lm -lcjson
LOG_DIR = logs
BUILD_DIR = build
BINARY_DIR = Binary
SRC_DIR = src
INCLUDE_DIR = include

# Directories for Modules
DISPLAY_SRC = $(SRC_DIR)/display
DRONE_SRC = $(SRC_DIR)/drone
MAP_SRC = $(SRC_DIR)/map
SERVER_SRC = $(SRC_DIR)/server
UTILS_SRC = $(SRC_DIR)

# Targets
all: $(LOG_DIR) $(BUILD_DIR) $(BINARY_DIR) display server targets obstacles map drone

# Ensure necessary directories exist
$(LOG_DIR):
	mkdir -p $(LOG_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/display $(BUILD_DIR)/drone $(BUILD_DIR)/map $(BUILD_DIR)/server

$(BINARY_DIR):
	mkdir -p $(BINARY_DIR)

# Create Subdirectories Explicitly Before Each Compilation
$(BUILD_DIR)/display:
	mkdir -p $(BUILD_DIR)/display

$(BUILD_DIR)/drone:
	mkdir -p $(BUILD_DIR)/drone

$(BUILD_DIR)/map:
	mkdir -p $(BUILD_DIR)/map

$(BUILD_DIR)/server:
	mkdir -p $(BUILD_DIR)/server

# Build the executables
display: $(BUILD_DIR)/display/display.o $(BUILD_DIR)/display/display_utils.o $(BUILD_DIR)/utils.o
	$(CC) -o $(BINARY_DIR)/display $^ $(CFLAGS) $(LDFLAGS)

server: $(BUILD_DIR)/server/server.o $(BUILD_DIR)/server/server_utils.o $(BUILD_DIR)/utils.o
	$(CC) -o $(BINARY_DIR)/server $^ $(CFLAGS) $(LDFLAGS)

map: $(BUILD_DIR)/map/map.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/map/map_utils.o
	$(CC) -o $(BINARY_DIR)/map $^ $(CFLAGS) $(LDFLAGS)

targets: $(BUILD_DIR)/map/targets.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/map/map_utils.o
	$(CC) -o $(BINARY_DIR)/targets $^ $(CFLAGS) $(LDFLAGS)

obstacles: $(BUILD_DIR)/map/obstacles.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/map/map_utils.o
	$(CC) -o $(BINARY_DIR)/obstacles $^ $(CFLAGS) $(LDFLAGS)

drone: $(BUILD_DIR)/drone/drone.o $(BUILD_DIR)/drone/drone_utils.o $(BUILD_DIR)/utils.o
	$(CC) -o $(BINARY_DIR)/drone $^ $(CFLAGS) $(LDFLAGS)

# Compile object files for each module
# Add here your object file compilation rules as before...

# Clean up build and log files
clean:
	rm -rf $(BUILD_DIR)/* $(LOG_DIR)/* $(BINARY_DIR)/*

# Phony Targets
.PHONY: all clean
