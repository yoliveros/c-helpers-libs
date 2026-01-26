CC = clang
I_FLAGS = -I$(VULKAN_SDK)/include 
L_FLAGS = -lvulkan -lxcb -L$(VULKAN_SDK)/lib
C_FLAGS = -Wall -Wextra -pedantic -std=c17 $(I_FLAGS) $(L_FLAGS)
DEV = -g -O0 -DDEBUG -fsanitize=address
PROD = -O2 -DNDEBUG

CONFIG ?= debug

ifeq ($(CONFIG), debug)
	C_FLAGS += $(DEV)
else
	C_FLAGS += $(PROD)
endif

NAME = Minecraft

SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(NAME)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

ARGS = $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))

.PHONY: all clean run prod dev

all: 
	$(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(C_FLAGS) 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

prod:
	$(MAKE) clean
	$(MAKE) CONFIG=prod run

dev: 
	$(MAKE) clean
	$(MAKE) CONFIG=debug run

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
