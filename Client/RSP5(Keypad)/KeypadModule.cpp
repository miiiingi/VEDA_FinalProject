#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/can.h>
#include <linux/can/raw.h>

using namespace std;

const int TCP_PORT = 5100;
const int BUFFER_SIZE = 1024;
const string SERVER_IP = "192.168.1.19";  // 젯슨 나노 IP

class AccessControlSystem {
private:
    const string CORRECT_PASSWORD = "1234";
    const int MAX_PASSWORD_LENGTH = 20;

    // CAN 통신 관련 변수
    int can_socket_fd;
    struct sockaddr_can can_addr;
    struct ifreq ifr;
    struct can_frame frame;

    // 이더넷 통신 관련 변수
    int client_socket;
    sockaddr_in server_addr;

    // 상태 변수
    bool face_recognition_result;
    bool password_verification_result;
    bool final_authentication_status;

public:
    AccessControlSystem() : 
        can_socket_fd(-1), 
        client_socket(-1), 
        face_recognition_result(false),
        password_verification_result(false),
        final_authentication_status(false) {
        memset(&frame, 0, sizeof(struct can_frame));
        memset(&server_addr, 0, sizeof(server_addr));
    }

    ~AccessControlSystem() {
        closeConnections();
    }

    // CAN 통신 초기화 메서드
    bool initializeCAN() {
        system("sudo ip link set can0 type can bitrate 100000");
        system("sudo ifconfig can0 up");

        can_socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (can_socket_fd < 0) {
            cerr << "CAN 소켓 생성 실패" << endl;
            return false;
        }

        strcpy(ifr.ifr_name, "can0");
        if (ioctl(can_socket_fd, SIOCGIFINDEX, &ifr) < 0) {
            cerr << "ioctl 실패" << endl;
            return false;
        }

        can_addr.can_family = AF_CAN;
        can_addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(can_socket_fd, (struct sockaddr *)&can_addr, sizeof(can_addr)) < 0) {
            cerr << "CAN 소켓 바인딩 실패" << endl;
            return false;
        }

        setsockopt(can_socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
        return true;
    }

    // 이더넷 통신 초기화 메서드
    bool initializeEthernet() {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            cerr << "소켓 생성 실패" << endl;
            return false;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TCP_PORT);

        if (inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr) <= 0) {
            cerr << "잘못된 주소" << endl;
            return false;
        }

        if (connect(client_socket, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            cerr << "서버 연결 실패" << endl;
            return false;
        }

        cout << "젯슨 나노 서버에 연결됨!" << endl;
        return true;
    }

    // 이더넷으로부터 얼굴인식 결과 수신
    bool receiveFaceRecognitionResult() {
        char buffer[BUFFER_SIZE] = {0};
        ssize_t receivedBytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (receivedBytes > 0) {
            face_recognition_result = (buffer[0] == '1');
            return true;
        }
        return false;
    }

    // 비밀번호 검증
    bool verifyPassword(const string& input_password) {
        password_verification_result = (input_password == CORRECT_PASSWORD);
        return password_verification_result;
    }

    // 최종 인증 상태 결정
    void determineFinalAuthentication() {
        final_authentication_status = (face_recognition_result && password_verification_result);
    }

    // CAN 프레임 준비 및 전송
    bool sendAuthenticationStatus() {
        frame.can_id = 0x123;
        frame.can_dlc = 3;
        frame.data[0] = face_recognition_result ? 1 : 0;
        frame.data[1] = password_verification_result ? 1 : 0;
        frame.data[2] = final_authentication_status ? 1 : 0;

        int nbytes = write(can_socket_fd, &frame, sizeof(frame));
        if (nbytes != sizeof(frame)) {
            cerr << "CAN 프레임 전송 실패!" << endl;
            return false;
        }
        return true;
    }

    // 키패드 입력 대기 및 처리 메서드
    void waitForSlashAndPassword() {
        char input;
        string password;

        cout << "키패드 대기 중 ('/' 입력을 대기합니다)..." << endl;

        // '/' 입력 대기
        while (cin.get(input)) {
            if (input == '/') {
                // 얼굴인식 결과 수신
                if (!receiveFaceRecognitionResult()) {
                    cerr << "얼굴인식 결과 수신 실패" << endl;
                    continue;
                }

                // 비밀번호 입력
                // cout << "비밀번호를 입력하세요: ";
                getline(cin, password);

                // 비밀번호 검증
                verifyPassword(password);

                // 최종 인증 상태 결정
                determineFinalAuthentication();

                // CAN으로 인증 상태 전송
                sendAuthenticationStatus();

                // 결과 출력
                cout << "얼굴인식 결과: " << (face_recognition_result ? "성공" : "실패") << endl;
                cout << "비밀번호 검증: " << (password_verification_result ? "성공" : "실패") << endl;
                cout << "최종 인증 상태: " << (final_authentication_status ? "승인" : "거부") << endl;
            }
        }
    }

    // 연결 종료 메서드
    void closeConnections() {
        if (client_socket >= 0) {
            close(client_socket);
        }
        if (can_socket_fd >= 0) {
            close(can_socket_fd);
            system("sudo ifconfig can0 down");
        }
    }

    // 시스템 초기화 및 실행
    bool initializeSystem() {
        return initializeCAN() && initializeEthernet();
    }
};

int main() {
    AccessControlSystem accessControl;

    // 시스템 초기화
    if (!accessControl.initializeSystem()) {
        cerr << "시스템 초기화 실패" << endl;
        return 1;
    }

    // 키패드 입력 대기 및 처리
    accessControl.waitForSlashAndPassword();

    return 0;
}
