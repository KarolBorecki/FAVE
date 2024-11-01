TARGET = FAVE

CXX = g++
CXXFLAGS = -g -Wall -Iinc -Iinc/external #TODO remove -g

LIBS = -ldl -lglfw -lGL 

SRC_DIR = src
OBJ_DIR = build
INC_DIR = inc

SRCS := $(wildcard $(SRC_DIR)/**/*.cpp) $(SRC_DIR)/external/glad.c $(SRC_DIR)/external/stb/stb.cpp ./main.cpp
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
	rm -f vgcore.*

.PHONY: all clean
