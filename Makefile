
# Directories
SRC_DIR = src
OBJ_DIR = obj
DATA_STRUCTURES = data_structures
GAME = game
NETWORKING = networking
COMMON = common

CC = gcc
CFLAGS =\
	-Wall -Wextra -pthread\
	-Iinclude -Iinclude/$(DATA_STRUCTURES) -Iinclude/$(GAME) -Iinclude/$(NETWORKING) -Iinclude/$(COMMON)\
	-Ilantern/include
LDFLAGS = -Llantern -llantern -lncurses -pthread -g

# Source files
SRC =\
	$(SRC_DIR)/main.c\
	$(SRC_DIR)/$(COMMON)/constants.c $(SRC_DIR)/$(COMMON)/string_utils.c\
	$(SRC_DIR)/$(COMMON)/random_utils.c\
	$(SRC_DIR)/$(DATA_STRUCTURES)/function_stack.c $(SRC_DIR)/$(DATA_STRUCTURES)/hashmap.c\
	$(SRC_DIR)/$(DATA_STRUCTURES)/stack.c $(SRC_DIR)/$(DATA_STRUCTURES)/vector.c\
	$(SRC_DIR)/$(GAME)/ability.c $(SRC_DIR)/$(GAME)/champion.c $(SRC_DIR)/$(GAME)/feature.c\
	$(SRC_DIR)/$(GAME)/pawn.c $(SRC_DIR)/$(GAME)/place.c $(SRC_DIR)/$(GAME)/world.c\
	$(SRC_DIR)/$(GAME)/item.c $(SRC_DIR)/$(GAME)/deal.c\
	$(SRC_DIR)/$(GAME)/fight.c\
	$(SRC_DIR)/$(NETWORKING)/command_handlers.c $(SRC_DIR)/$(NETWORKING)/server.c\
	$(SRC_DIR)/$(NETWORKING)/user.c $(SRC_DIR)/$(NETWORKING)/account.c\
	$(SRC_DIR)/$(NETWORKING)/client.c $(SRC_DIR)/$(NETWORKING)/event_handlers.c\
	$(SRC_DIR)/$(NETWORKING)/combat_command_handler.c $(SRC_DIR)/$(NETWORKING)/gameworld_command_handler.c\
	$(SRC_DIR)/$(NETWORKING)/initial_command_handler.c

# Object files
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Targets
BIN = main

# Default target
all: $(BIN)

# Build binary
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile object code
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/$(DATA_STRUCTURES)
	@mkdir -p $(OBJ_DIR)/$(GAME)
	@mkdir -p $(OBJ_DIR)/$(NETWORKING)
	@mkdir -p $(OBJ_DIR)/$(COMMON)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean
