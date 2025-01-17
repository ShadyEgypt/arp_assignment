# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -I/usr/include/cjson -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/display -I$(INCLUDE_DIR)/drone -I$(INCLUDE_DIR)/map
LDFLAGS = -lncurses -lrt -pthread -lm -lcjson
LOG_DIR = logs
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

# Directories for Modules
DISPLAY_SRC = $(SRC_DIR)/display
DRONE_SRC = $(SRC_DIR)/drone
MAP_SRC = $(SRC_DIR)/map
SERVER_SRC = $(SRC_DIR)/server
UTILS_SRC = $(SRC_DIR)

# Targets
all: $(LOG_DIR) $(BUILD_DIR) display server targets obstacles map drone

# Ensure necessary directories exist
$(LOG_DIR):
	mkdir -p $(LOG_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/display $(BUILD_DIR)/drone $(BUILD_DIR)/map $(BUILD_DIR)/server

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
	$(CC) -o display $^ $(CFLAGS) $(LDFLAGS)

server: $(BUILD_DIR)/server/server.o $(BUILD_DIR)/server/server_utils.o $(BUILD_DIR)/utils.o
	$(CC) -o server $^ $(CFLAGS) $(LDFLAGS)

map: $(BUILD_DIR)/map/map.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/map/map_utils.o
	$(CC) -o map $^ $(CFLAGS) $(LDFLAGS)

targets: $(BUILD_DIR)/map/targets.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/map/map_utils.o
	$(CC) -o targets $^ $(CFLAGS) $(LDFLAGS)

obstacles: $(BUILD_DIR)/map/obstacles.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/map/map_utils.o
	$(CC) -o obstacles $^ $(CFLAGS) $(LDFLAGS)

drone: $(BUILD_DIR)/drone/drone.o $(BUILD_DIR)/drone/drone_utils.o $(BUILD_DIR)/utils.o
	$(CC) -o drone $^ $(CFLAGS) $(LDFLAGS)

# Compile object files for each module

# DISPLAY MODULE
$(BUILD_DIR)/display/display.o: $(DISPLAY_SRC)/display.c $(INCLUDE_DIR)/display/display_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/display
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/display -I$(INCLUDE_DIR)

$(BUILD_DIR)/display/display_utils.o: $(DISPLAY_SRC)/display_utils.c $(INCLUDE_DIR)/display/display_utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/display
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/display -I$(INCLUDE_DIR)

# DRONE MODULE
$(BUILD_DIR)/drone/drone.o: $(DRONE_SRC)/drone.c $(INCLUDE_DIR)/drone/drone_utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/drone
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/drone -I$(INCLUDE_DIR)

$(BUILD_DIR)/drone/drone_utils.o: $(DRONE_SRC)/drone_utils.c $(INCLUDE_DIR)/drone/drone_utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/drone
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/drone -I$(INCLUDE_DIR)

# MAP MODULE
$(BUILD_DIR)/map/map.o: $(MAP_SRC)/map.c $(INCLUDE_DIR)/map/map_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/map
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/map -I$(INCLUDE_DIR)

$(BUILD_DIR)/map/obstacles.o: $(MAP_SRC)/obstacles.c $(INCLUDE_DIR)/map/map_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/map
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/map -I$(INCLUDE_DIR)

$(BUILD_DIR)/map/targets.o: $(MAP_SRC)/targets.c $(INCLUDE_DIR)/map/map_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/map
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/map -I$(INCLUDE_DIR)

$(BUILD_DIR)/map/map_utils.o: $(MAP_SRC)/map_utils.c $(INCLUDE_DIR)/map/map_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/map
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/map -I$(INCLUDE_DIR)

# SERVER MODULE
$(BUILD_DIR)/server/server.o: $(SERVER_SRC)/server.c $(INCLUDE_DIR)/server/server_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/server
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/server -I$(INCLUDE_DIR) 

$(BUILD_DIR)/server/server_utils.o: $(SERVER_SRC)/server_utils.c $(INCLUDE_DIR)/server/server_utils.h $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)/server
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)/server -I$(INCLUDE_DIR) 

# UTILS MODULE
$(BUILD_DIR)/utils.o: $(UTILS_SRC)/utils.c $(INCLUDE_DIR)/utils.h $(INCLUDE_DIR)/globals.h | $(BUILD_DIR)
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR)

# Clean up build and log files
clean:
	rm -rf $(BUILD_DIR)/* $(LOG_DIR)/* display server targets obstacles map drone

# Phony Targets
.PHONY: all clean
