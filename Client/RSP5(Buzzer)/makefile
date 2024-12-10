# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Iinc

# Linker flags
LDFLAGS = -lwiringPi -pthread

# Source directories
SRC_DIR = src
INC_DIR = inc

# Output directories
OBJ_DIR = obj

# Source files
SRCS = $(SRC_DIR)/SoundManager.cpp \
       $(SRC_DIR)/LEDController.cpp \
       $(SRC_DIR)/CANCommunicationManager.cpp \
       main.cpp

# Object files
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

# Executable name
TARGET = buzzer

# Default target
all: $(OBJ_DIR) $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets
.PHONY: all clean