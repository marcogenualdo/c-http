# --- Variables ---
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread -ldl

# Target names
SERVER_BIN = server
PLUGIN_SO = handlers.so
CONTROL_OBJ = control.o

# --- Default Target ---
all: $(SERVER_BIN) $(PLUGIN_SO)

# --- Compilation Rules ---

# 1. Build the server binary
# It depends on the main source and the control object
$(SERVER_BIN): server.c $(CONTROL_OBJ)
	$(CC) $(CFLAGS) server.c $(CONTROL_OBJ) $(LDFLAGS) -o $(SERVER_BIN)

# 2. Build the control object (Static part)
$(CONTROL_OBJ): control.c control.h
	$(CC) $(CFLAGS) -c control.c -o $(CONTROL_OBJ)

# 3. Build the dynamic plugin
$(PLUGIN_SO): handlers.c
	$(CC) $(CFLAGS) -fPIC -shared handlers.c -o $(PLUGIN_SO)

# --- Utility Targets ---

# Clean up build artifacts
clean:
	rm -f $(SERVER_BIN) $(PLUGIN_SO) $(CONTROL_OBJ) /tmp/server.control

# Hot Reload via CLI (requires the server to be running)
reload:
	@echo "Sending SIGUSR1 to server..."
	pkill -USR1 -f ./$(SERVER_BIN) || echo "Server not running."

# Test with our Unix Socket command
test-reload:
	@echo "Sending RELOAD command via Unix Socket..."
	echo "RELOAD" | socat - UNIX-CONNECT:/tmp/server.control
