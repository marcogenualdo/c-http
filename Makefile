# --- Variables ---
CC      := gcc
CFLAGS  := -Wall -Wextra -g -I./src
LDFLAGS := -ldl -Wl,-rpath,'$$ORIGIN'

# Directories
SRC_DIR   := src
BUILD_DIR := build

# Target names (placed in root)
SERVER_BIN := chttp
PLUGIN_SO  := libchttp-handlers.so

# Object files (placed in build/)
STATIC_LIBS := $(BUILD_DIR)/worker.o $(BUILD_DIR)/requests.o
SERVER_OBJ  := $(BUILD_DIR)/server.o

# --- Default Target ---
.PHONY: all clean reload test-reload

all: $(SERVER_BIN) $(PLUGIN_SO)

# --- Compilation Rules ---

# 1. Link the server binary
# Uses $^ (all prerequisites) and $@ (target name)
$(SERVER_BIN): $(SERVER_OBJ) $(STATIC_LIBS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# 2. Build the dynamic plugin
# Note: we compile directly from src/ to root for simplicity here,
# but we use -fPIC which is required for shared libraries.
$(PLUGIN_SO): $(SRC_DIR)/handlers.c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@

# 3. Pattern Rule for Object Files
# This handles BOTH server.o and control.o automatically.
# The '| $(BUILD_DIR)' is an "order-only" prerequisite (it must exist,
# but its timestamp doesn't force a recompile).
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 4. Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# --- Utility Targets ---

clean:
	rm -rf $(BUILD_DIR) $(SERVER_BIN) $(PLUGIN_SO) /tmp/server.control

reload:
	@echo "Sending SIGUSR1 to server..."
	pkill -USR1 -f ./$(SERVER_BIN) || echo "Server not running."

test-reload:
	@echo "Sending RELOAD command via Unix Socket..."
	echo "RELOAD" | socat - UNIX-CONNECT:/tmp/server.control
