#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>

#define SERVO_PIN 1     // 서보 모터 핀
#define SERVO_MIN_DUTY 5  // 최소 듀티비 (%)
#define SERVO_MAX_DUTY 25 // 최대 듀티비 (%)
#define CAN_INTERFACE "can0" // CAN 인터페이스 이름
#define MOTOR_DELAY 5 // 모터 방향 전환 전 대기 시간(초)

class ServoController {
private:
    int servoPin;
    int canSocket;

public:
    ServoController(int pin = SERVO_PIN) : servoPin(pin) {
        // WiringPi 초기화
        if (wiringPiSetup() == -1) {
            printf("WiringPi 초기화 실패\n");
            exit(1);
        }

        // 소프트웨어 PWM 설정
        if (softPwmCreate(servoPin, 0, 100) != 0) {
            printf("PWM 설정 실패\n");
            exit(1);
        }

        // CAN 초기화
        setupCAN();

        // 초기 상태: 서보 중립
        softPwmWrite(servoPin, 0);
    }

    ~ServoController() {
        // 서보 중립
        softPwmWrite(servoPin, 0);
        close(canSocket);
    }

    void setupCAN() {
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

    void setServoDegree(int degree) {
        if (degree > 180) degree = 180;
        if (degree < 0) degree = 0;

        float duty = SERVO_MIN_DUTY + (degree * (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 180.0);
        softPwmWrite(servoPin, (int)duty);
        usleep(300000); // 0.3초 대기
        softPwmWrite(servoPin, 0);
        
        printf("서보 모터 각도 설정: %d도\n", degree);
    }

    void processCANMessages() {
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
                    
                    // 잠금 해제 - 서보 모터 150도
                    printf("잠금 해제 - 서보 모터 구동\n");
                    setServoDegree(150);
                    sleep(MOTOR_DELAY);
                    
                    // 자동 잠금 메시지 전송
                    frame.can_id = 0x002;
                    frame.can_dlc = 3;
                    frame.data[0] = 0b00000100;
                    write(canSocket, &frame, sizeof(frame));

                    frame.can_id = 0x001;
                    write(canSocket, &frame, sizeof(frame));

                    // 잠금 - 서보 모터 50도
                    printf("자동 잠금 - 서보 모터 구동\n");
                    setServoDegree(50);
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
};

int main() {
    printf("CAN 서보 모터 제어 프로그램 시작 (WiringPi Pin 1)\n");
    system("sudo ifconfig can0 down");
    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");
    
    ServoController servo;
    
    while(1) {
        servo.processCANMessages();
    }
    return 0;
}