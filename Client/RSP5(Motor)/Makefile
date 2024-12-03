# 컴파일러 설정
CXX = g++
CXXFLAGS = -Wall -std=c++11

# 라이브러리 설정
LIBS = -lwiringPi -lpthread

# 소스 파일 디렉토리
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# 소스 파일 목록
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)

# 헤더 파일 디렉토리 설정
INCLUDES = -I$(INC_DIR)

# 실행 파일 이름
TARGET = motor

# 기본 타겟
all: directories $(TARGET)

# 디렉토리 생성
directories:
	@mkdir -p $(OBJ_DIR)

# 실행 파일 생성 규칙
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

# 오브젝트 파일 생성 규칙
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $< > $(OBJ_DIR)/$*.d

# 의존성 파일 포함
-include $(DEPS)

# clean 타겟
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# 파일 구조 생성
init:
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(INC_DIR)
	@mkdir -p $(OBJ_DIR)
	@echo "Created directory structure"

# phony 타겟 선언
.PHONY: all clean directories init

# 디버그 빌드 설정
debug: CXXFLAGS += -g -DDEBUG
debug: all

# 릴리즈 빌드 설정
release: CXXFLAGS += -O2 -DNDEBUG
release: all

# 설치 규칙
install: $(TARGET)
	@sudo cp $(TARGET) /usr/local/bin
	@echo "Installation completed"

# 제거 규칙
uninstall:
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstallation completed"

# 테스트 규칙
test: $(TARGET)
	./$(TARGET)

# 도움말
help:
	@echo "Available targets:"
	@echo "  make        - Build the project"
	@echo "  make clean  - Remove build files"
	@echo "  make init   - Create directory structure"
	@echo "  make debug  - Build with debug symbols"
	@echo "  make release- Build with optimization"
	@echo "  make install- Install the program"
	@echo "  make test   - Run the program"
	@echo "  make help   - Show this help message"