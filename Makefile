# ══════════════════════════════════════════════════════════════════════════════
# Makefile – Hybrid Inventory Manager
# C files compile as C  (CC / CFLAGS)
# C++ files compile as C++ (CXX / CXXFLAGS)
# Final link is performed by the C++ driver (CXX) to pull in the C++ runtime.
# ══════════════════════════════════════════════════════════════════════════════

# ── Toolchain ────────────────────────────────────────────────────────────────
CC      := gcc
CXX     := g++
LD      := $(CXX)           # link with g++ so the C++ runtime is included

# ── Flags ────────────────────────────────────────────────────────────────────
CFLAGS   := -std=c11   -Wall -Wextra -pedantic -Iinclude
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Iinclude
LDFLAGS  :=            # add -lm or other libs here if needed

# ── Directories ───────────────────────────────────────────────────────────────
SRC_DIR  := src
OBJ_DIR  := build
BIN_DIR  := bin

# ── Sources ───────────────────────────────────────────────────────────────────
C_SRCS   := $(SRC_DIR)/inventory.c
CXX_SRCS := $(SRC_DIR)/InventoryManager.cpp \
             $(SRC_DIR)/main.cpp

# ── Objects ───────────────────────────────────────────────────────────────────
C_OBJS   := $(patsubst $(SRC_DIR)/%.c,   $(OBJ_DIR)/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CXX_SRCS))
ALL_OBJS := $(C_OBJS) $(CXX_OBJS)

# ── Target ────────────────────────────────────────────────────────────────────
TARGET   := $(BIN_DIR)/inventory_manager

# ── Default target ─────────────────────────────────────────────────────────
.PHONY: all
all: $(TARGET)

# ── Link ─────────────────────────────────────────────────────────────────────
$(TARGET): $(ALL_OBJS) | $(BIN_DIR)
	$(LD) $(ALL_OBJS) $(LDFLAGS) -o $@
	@echo ""
	@echo "  Build successful → $@"
	@echo "  Run with:  ./$(TARGET)"
	@echo ""

# ── Compile C sources ────────────────────────────────────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ── Compile C++ sources ──────────────────────────────────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ── Directory creation ───────────────────────────────────────────────────────
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ── Housekeeping ─────────────────────────────────────────────────────────────
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) inventory.dat
	@echo "  Cleaned build artefacts and data file."

.PHONY: run
run: all
	./$(TARGET)

# ── Help ─────────────────────────────────────────────────────────────────────
.PHONY: help
help:
	@echo ""
	@echo "  Targets:"
	@echo "    make          – build the executable"
	@echo "    make run      – build and launch"
	@echo "    make clean    – remove build/ bin/ and inventory.dat"
	@echo "    make help     – this message"
	@echo ""
