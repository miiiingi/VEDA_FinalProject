# Makefile for Keypad Module

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 \
           -Wall \
           -Wextra \
           -Werror \
           -pedantic \
           -O2 \
           -pthread

# Library flags
LDFLAGS = -lssl \
          -lcrypto \
          -pthread

# Target executable
TARGET = KeypadModule

# Source files
SRCS = KeypadModule.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Install dependencies (for Debian/Ubuntu)
install-deps:
	sudo apt-get update
	sudo apt-get install -y \
		g++ \
		libssl-dev \
		can-utils \
		net-tools

# Phony targets
.PHONY: all clean install-deps0