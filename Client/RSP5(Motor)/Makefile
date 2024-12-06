# 컴파일러 설정
CXX = g++
CXXFLAGS = -Wall -std=c++11

# 디렉토리 설정
SRCDIR = src
INCDIR = include
OBJDIR = obj

# 라이브러리 설정
LIBS = -lwiringPi

# Include 경로 설정
INCLUDES = -I$(INCDIR)

# 소스 파일들
SRCS = $(wildcard $(SRCDIR)/*.cpp)

# 오브젝트 파일들 (obj 디렉토리에 생성)
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# 실행 파일 이름
TARGET = motor

# obj 디렉토리 생성
$(shell mkdir -p $(OBJDIR))

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

# 오브젝트 파일 생성 규칙
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# clean 타겟
clean:
	rm -rf $(OBJDIR) $(TARGET)

# 실행 타겟
run: $(TARGET)
	sudo ./$(TARGET)

# CAN 설정 타겟
setup_can:
	sudo ifconfig can0 down
	sudo ip link set can0 type can bitrate 100000
	sudo ifconfig can0 up

# .PHONY 설정
.PHONY: all clean run setup_can