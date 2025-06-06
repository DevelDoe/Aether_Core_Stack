# Project: Seer
# Makefile

CC = gcc
CFLAGS = -O2 -Iinclude
LDFLAGS = -lwebsockets -ljson-c
SRC = src/main.c src/logger.c src/cssh_client.c src/manifest.c src/utils.c
BIN = seer
INSTALL_PATH = /usr/local/bin/$(BIN)

all: $(BIN)

$(BIN): $(SRC)
	@echo "🔨 Building binary..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ Build done."

install: $(BIN)
	@echo "🚚 Installing to $(INSTALL_PATH)"
	install -m 755 $(BIN) $(INSTALL_PATH)
	@echo "✅ Install complete."

clean:
	@echo "🧹 Cleaning up..."
	rm -f $(BIN)
	rm -f $(INSTALL_PATH)
