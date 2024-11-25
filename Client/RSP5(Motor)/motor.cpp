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

#define MOTOR_PIN 27    // GPIO 27 사용
#define PWM_RANGE 255   // PWM 범위 설정
#define FIXED_SPEED 120 // 고정 PWM 값
#define CAN_INTERFACE "can0" // CAN 인터페이스 이름

class MotorController {
private:
    int motorPin;
    bool isEnabled;
    int canSocket;

public:
    MotorController(int pin = MOTOR_PIN) : motorPin(pin), isEnabled(false) {
        // WiringPi 초기화
        if (wiringPiSetupGpio() == -1) {
            printf("WiringPi 초기화 실패\n");
            exit(1);
        }

        // 소프트웨어 PWM 설정
        if (softPwmCreate(motorPin, 0, PWM_RANGE) != 0) {
            printf("PWM 설정 실패\n");
            exit(1);
        }

        // CAN 초기화
        setupCAN();

        // 초기 상태: 모터 정지
        softPwmWrite(motorPin, 0);
    }

    ~MotorController() {
        // 모터 정지
        softPwmWrite(motorPin, 0);
        close(canSocket);
    }

    void setupCAN() {
        // CAN 초기화 명령 실행
        printf("CAN 초기화 시작...\n");

        // CAN 소켓 생성
        canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (canSocket < 0) {
            perror("CAN 소켓 생성 실패");
            exit(1);
        }

        // CAN 인터페이스 설정
        struct ifreq ifr;
        strcpy(ifr.ifr_name, CAN_INTERFACE);
        if (ioctl(canSocket, SIOCGIFINDEX, &ifr) < 0) {
            perror("CAN 인터페이스 설정 실패");
            exit(1);
        }

        // CAN 소켓 바인딩
        struct sockaddr_can addr;
        addr.can_family = PF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("CAN 소켓 바인딩 실패");
            exit(1);
        }

        // CAN 필터 설정 (ID: 0x123만 수신)
        struct can_filter rfilter[1];
        rfilter[0].can_id = 0x123;
        rfilter[0].can_mask = CAN_SFF_MASK;
        setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        printf("CAN 초기화 완료\n");
    }

    void setMotorSpeed(int speed) {
        // 속도값 범위 제한
        if (speed < 0) speed = 0;
        if (speed > PWM_RANGE) speed = PWM_RANGE;

        // PWM 신호 출력
        softPwmWrite(motorPin, speed);
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
            
            // 모든 데이터 바이트 출력
            for(int i = 0; i < frame.can_dlc; i++) {
                printf("데이터[%d]: %d\n", i, frame.data[i]);
            }

            // ID 0x123 메시지 처리
            if (frame.can_id == 0x123) {
                if (frame.data[2] == 1) {
                    printf("얼굴 인식 성공!\n");
                    printf("비밀번호 입력 성공!\n");    //Qt 메세지로 띄우기
                    printf("모터 작동 시작 (PWM: %d)\n", FIXED_SPEED);
                    setMotorSpeed(FIXED_SPEED);
                    
                    // 1초 대기
                    sleep(1);
                    
                    // 1초 후 모터 정지
                    setMotorSpeed(0);
                    printf("잠금 장치 구동 완료\n");
                }
                else if (frame.data[2] == 0) {
                    setMotorSpeed(0);
                    printf("비밀번호가 틀렸습니다.\n");     //Qt 메세지로 띄우기
                }
            }
        }
    }
};

int main() {
    printf("CAN 모터 제어 프로그램 시작 (GPIO 27)\n");
    system("sudo ifconfig can0 down");
    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");
    
    MotorController motor;
    
    while(1) {
        motor.processCANMessages();
    }

    return 0;
}