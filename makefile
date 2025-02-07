TARGET = FAVE

CC = g++
CFLAGS = -g -Wall -Iinc -Iinc/external #TODO remove -g

LIBS = -ldl -lglfw -lGL 

SRC_DIR = src
OBJ_DIR = build
INC_DIR = inc

SRCS := $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/*.c) $(SRC_DIR)/external/glad.c $(SRC_DIR)/external/stb/stb.cpp ./main.c
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
	rm -f vgcore.*

.PHONY: all clean
