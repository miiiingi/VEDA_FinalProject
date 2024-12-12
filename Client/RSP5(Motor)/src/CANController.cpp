#include "../include/CANController.h"
#include "CANController.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>

const char* CANController::CAN_INTERFACE = "can0";

CANController::CANController(Motor& motorRef) : motor(motorRef) {
    setupCAN();
}

CANController::~CANController() {
    close(canSocket);
}

void CANController::setupCAN() {
    printf("CAN 초기화 시작...\n");

    canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (canSocket < 0) {
        perror("CAN 소켓 생성 실패");
        exit(1);
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, CAN_INTERFACE);
    if (ioctl(canSocket, SIOCGIFINDEX, &ifr) < 0) {
        perror("CAN 인터페이스 설정 실패");
        exit(1);
    }

    struct sockaddr_can addr;
    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("CAN 소켓 바인딩 실패");
        exit(1);
    }

    struct can_filter rfilter[2];
    rfilter[0].can_id = 0x001;
    rfilter[0].can_mask = CAN_SFF_MASK;
    rfilter[1].can_id = 0x002;
    rfilter[1].can_mask = CAN_SFF_MASK;
    setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    printf("CAN 초기화 완료\n");
}

void CANController::sendCANMessage(struct can_frame& frame) {
    write(canSocket, &frame, sizeof(frame));
}

void CANController::processMessages() {
    struct can_frame frame;
    int nbytes = read(canSocket, &frame, sizeof(struct can_frame));
    
    if (nbytes < 0) {
        perror("CAN 메시지 읽기 실패");
        return;
    }

    if (nbytes > 0) {
        printf("수신된 CAN 메시지:\n");
        printf("CAN ID: 0x%X\n", frame.can_id);
        printf("데이터 길이: %d\n", frame.can_dlc);
        
        for(int i = 0; i < frame.can_dlc; i++) {
            printf("데이터[%d]: %d\n", i, frame.data[i]);
        }

        if (frame.can_id == 0x001) {
            if (frame.data[0] == 0b00000011) {
                printf("얼굴 인식 성공!\n");
                printf("비밀번호 입력 성공!\n");
                
                printf("잠금 해제 - 모터 구동\n");
                motor.setDegree(150);
                sleep(MOTOR_DELAY);
                
                frame.can_id = 0x003;
                frame.can_dlc = 3;
                frame.data[0] = 0b00000100;
                sendCANMessage(frame);

                printf("자동 잠금 - 모터 구동\n");
                motor.setDegree(50);
                printf("잠금 장치 구동 완료\n");
            }
            else if (frame.data[0] == 0b00000000) {
                printf("얼굴이 일치하지 않습니다.\n");
                printf("비밀번호가 틀렸습니다.\n");
            }
            else if (frame.data[0] == 0b00000001) {
                printf("얼굴 인식 성공!\n");
                printf("비밀번호가 틀렸습니다.\n");
            }
            else if (frame.data[0] == 0b00000010) {
                printf("얼굴이 일치하지 않습니다.\n");
                printf("비밀번호 입력 성공!\n");
            }
        }
    }
}
