#include "CANController.h"
#include "Constants.h"
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <linux/can.h>
#include <linux/can/raw.h>  // SOL_CAN_RAW, CAN_RAW_FILTER를 위한 헤더

CANController::CANController() : canSocket(-1) {}

CANController::~CANController() {
    if (canSocket >= 0) {
        close(canSocket);
    }
}

void CANController::initialize() {
    std::cout << "CAN 초기화 시작...\n";
    
    canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (canSocket < 0) {
        throw std::runtime_error("CAN 소켓 생성 실패");
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, CAN_INTERFACE);
    if (ioctl(canSocket, SIOCGIFINDEX, &ifr) < 0) {
        throw std::runtime_error("CAN 인터페이스 설정 실패");
    }

    struct sockaddr_can addr;
    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("CAN 소켓 바인딩 실패");
    }

    setupFilters();
    std::cout << "CAN 초기화 완료\n";
}

void CANController::setupFilters() {
    struct can_filter rfilter[2];
    rfilter[0].can_id = 0x001;
    rfilter[0].can_mask = CAN_SFF_MASK;
    rfilter[1].can_id = 0x002;
    rfilter[1].can_mask = CAN_SFF_MASK;
    if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)) < 0) {
        throw std::runtime_error("CAN 필터 설정 실패");
    }
}

bool CANController::receiveMessage(struct can_frame& frame) {
    int nbytes = read(canSocket, &frame, sizeof(struct can_frame));
    return nbytes > 0;
}

bool CANController::sendMessage(const struct can_frame& frame) {
    return write(canSocket, &frame, sizeof(frame)) == sizeof(frame);
}