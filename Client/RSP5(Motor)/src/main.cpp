#include "../include/DoorLockController.h"
#include <cstdlib>
#include <iostream>

int main() {
    std::cout << "CAN 모터 제어 프로그램 시작 (GPIO 27)\n";
    
    system("sudo ifconfig can0 down");
    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");
    
    try {
        DoorLockController controller;
        controller.run();
    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}