#include "../inc/CANCommunicationManager.h"
#include "../inc/LEDController.h"
#include "../inc/SoundManager.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

CANCommunicationManager::CANCommunicationManager(LEDController& lc, SoundManager& sm) 
    : ledController(lc), soundManager(sm) {}

void CANCommunicationManager::initializeCANSocket() {
    system("sudo ifconfig can0 down");
    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");

    socketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socketFd < 0) {
        throw std::runtime_error("CAN socket creation failed");
    }

    strcpy(ifr.ifr_name, "can0");
    if (ioctl(socketFd, SIOCGIFINDEX, &ifr) < 0) {
        closeCANSocket();
        throw std::runtime_error("CAN interface configuration failed");
    }

    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        closeCANSocket();
        throw std::runtime_error("CAN socket bind failed");
    }

    struct can_filter rfilter[2];
    rfilter[0].can_id = 0x001;
    rfilter[0].can_mask = CAN_SFF_MASK;
    rfilter[1].can_id = 0x003;
    rfilter[1].can_mask = CAN_SFF_MASK;
    setsockopt(socketFd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
}

bool CANCommunicationManager::processCANMessage() {
    struct can_frame frame;
    int nbytes = read(socketFd, &frame, sizeof(frame));
    
    if (nbytes > 0) {
        if (frame.data[0] == 3) {
            std::cout << "Open" << std::endl;    
            soundThread = std::thread(&SoundManager::playOpenTone, &soundManager);
            ledThread = std::thread(&LEDController::setGreenLED, &ledController);
            failCounter = 1;
        }
        else if(frame.data[0] == 4) {
            std::cout << "Closed" << std::endl; 
            ledThread = std::thread(&LEDController::setGreenLED, &ledController);   
            soundThread = std::thread(&SoundManager::playCloseTone, &soundManager);
            failCounter = 1;
        }
        else {
            if ((failCounter == 3)||(frame.data[0] == 0)||(frame.data[0]==2)) {
                std::cout << "Alarm" << std::endl;
                ledThread = std::thread(&LEDController::setRedLED, &ledController);
                soundThread = std::thread(&SoundManager::playAlarmTone, &soundManager);
                failCounter = 1;
            }
            else if (frame.data[0]==1){
                std::cout << "Retry password" << std::endl;
                ledThread = std::thread(&LEDController::setBlueLED, &ledController);
                soundThread = std::thread(&SoundManager::playFailTone, &soundManager);
                failCounter++;
            }
        }
        if (ledThread.joinable()) ledThread.join();
        if (soundThread.joinable()) soundThread.join();
    }
    return false;
}

bool CANCommunicationManager::sendCANMessage(uint32_t can_id, uint8_t data) {
    if (socketFd == -1) {
        std::cerr << "CAN 소켓이 초기화되지 않았습니다." << std::endl;
        return false;
    }
        
    struct can_frame frame;
    memset(&frame, 0, sizeof(struct can_frame));
    frame.can_id = can_id;
    frame.can_dlc = 1;
    frame.data[0] = data;

    std::cout << "CAN 메시지 전송 시도:" << std::endl;
    std::cout << "CAN ID: 0x" << std::hex << can_id << std::endl;
    std::cout << "데이터: " << (int)data << std::endl;
    
    int bytesWritten = write(socketFd, &frame, sizeof(frame));

    if (bytesWritten < 0) {
        std::cerr << "CAN 메시지 전송 실패. 오류: " << strerror(errno) << std::endl;
        return false;
    }
    else if (bytesWritten != sizeof(frame)) {
        std::cerr << "불완전한 CAN 프레임 전송" << std::endl;
        return false;
    }

    std::cout << "CAN 메시지 전송 성공!" << std::endl;
    return true;
}

int CANCommunicationManager::getSocketFd() const {
    return socketFd;
}

CANCommunicationManager::~CANCommunicationManager() {
    closeCANSocket();
    system("sudo ifconfig can0 down");
}

void CANCommunicationManager::closeCANSocket() {
    if (socketFd != -1) {
        close(socketFd);
        socketFd = -1;
    }
}
