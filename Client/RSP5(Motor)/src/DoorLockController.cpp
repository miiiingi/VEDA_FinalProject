#include "../include/DoorLockController.h"
#include "../include/Constants.h"
#include <iostream>
#include <unistd.h>

DoorLockController::DoorLockController() : motor(MOTOR_PIN) {
    canController.initialize();
}

void DoorLockController::handleAuthenticationMessage(const struct can_frame& frame) {
    if (frame.can_id != 0x001) return;

    switch(frame.data[0]) {
        case 0b00000011: // 얼굴 인식 + 비밀번호 성공
            std::cout << "얼굴 인식 성공!\n비밀번호 입력 성공!\n";
            unlockDoor();
            sleep(MOTOR_DELAY);
            lockDoor();
            break;
        case 0b00000000: // 얼굴x, 비번x
            std::cout << "얼굴이 일치하지 않습니다.\n비밀번호가 틀렸습니다.\n";
            motor.stop();
            break;
        case 0b00000001: // 얼굴o, 비번x
            std::cout << "얼굴 인식 성공!\n비밀번호가 틀렸습니다.\n";
            motor.stop();
            break;
        case 0b00000010: // 얼굴x, 비번o
            std::cout << "얼굴이 일치하지 않습니다.\n비밀번호 입력 성공!\n";
            motor.stop();
            break;
    }
}

void DoorLockController::unlockDoor() {
    std::cout << "잠금 해제 - 모터 정방향 구동 시작\n";
    motor.setSpeed(FIXED_SPEED);
    sleep(1);
    motor.stop();
}

void DoorLockController::lockDoor() {
    struct can_frame frame = {};
    frame.can_id = 0x001;
    frame.can_dlc = 3;
    frame.data[0] = 0b00000100;
    canController.sendMessage(frame);

    std::cout << "자동 잠금 - 모터 역방향 구동 시작\n";
    motor.setSpeed(REVERSE_SPEED);
    sleep(1);
    motor.stop();
    std::cout << "잠금 장치 구동 완료\n";
}

void DoorLockController::processMessages() {
    struct can_frame frame;
    if (canController.receiveMessage(frame)) {
        std::cout << "수신된 CAN 메시지:\n"
                  << "CAN ID: 0x" << std::hex << frame.can_id << std::dec << "\n"
                  << "데이터 길이: " << (int)frame.can_dlc << "\n";
        
        for(int i = 0; i < frame.can_dlc; i++) {
            std::cout << "데이터[" << i << "]: " << (int)frame.data[i] << "\n";
        }
        
        handleAuthenticationMessage(frame);
    }
}

void DoorLockController::run() {
    while(true) {
        processMessages();
    }
}