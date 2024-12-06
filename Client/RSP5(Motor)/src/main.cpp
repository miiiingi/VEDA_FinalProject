#include "../include/Motor.h"
#include "../include/CANController.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("CAN 모터 제어 프로그램 시작 (WiringPi Pin 1)\n");
    
    system("sudo ifconfig can0 down");
    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");
    
    Motor motor(1);
    CANController canController(motor);
    
    while(1) {
        canController.processMessages();
    }
    
    return 0;
}