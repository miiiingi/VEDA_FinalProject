#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

using namespace std;

// 상수 정의
const int TCP_PORT = 5100;
const int BUFFER_SIZE = 1024;
const string SERVER_IP = "192.168.1.19";  // 젯슨 나노 IP
const string PASSWORD = "1234";

// 공유 데이터를 위한 구조체
struct SharedData {
    atomic<bool> passwordMatch{false};
    atomic<bool> faceRecognition{false};
    mutex canMutex;
};

class AccessControlSystem {
private:
    int socket_fd;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    SharedData& sharedData;

public:
    AccessControlSystem(SharedData& sd) : socket_fd(-1), sharedData(sd) {
        memset(&frame, 0, sizeof(struct can_frame));
    }

    ~AccessControlSystem() {
        closeConnection();
    }

    bool initializeCAN() {
        system("sudo ip link set can0 type can bitrate 100000");
        system("sudo ifconfig can0 up");
        
        socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (socket_fd < 0) {
            cerr << "Socket PF_CAN failed" << endl;
            return false;
        }

        strcpy(ifr.ifr_name, "can0");
        if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
            cerr << "ioctl failed" << endl;
            return false;
        }

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            cerr << "bind failed" << endl;
            return false;
        }

        setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
        return true;
    }

    void sendCANFrame() {
        lock_guard<mutex> lock(sharedData.canMutex);
        
        frame.can_id = 0x123;
        frame.can_dlc = 8;
        frame.data[0] = sharedData.passwordMatch ? 1 : 0;
        frame.data[1] = sharedData.faceRecognition ? 1 : 0;
        frame.data[2] = (sharedData.passwordMatch && sharedData.faceRecognition) ? 1 : 0;
        
        for(int i = 3; i < 8; i++) {
            frame.data[i] = 0;
        }

        int nbytes = write(socket_fd, &frame, sizeof(frame));
        if(nbytes != sizeof(frame)) {
            cerr << "Send Error frame!" << endl;
        }

        cout << "\n현재 상태:" << endl;
        cout << "비밀번호 확인: " << (sharedData.passwordMatch ? "성공" : "실패") << endl;
        cout << "얼굴 인식: " << (sharedData.faceRecognition ? "성공" : "실패") << endl;
        cout << "최종 결과: " << ((sharedData.passwordMatch && sharedData.faceRecognition) ? "출입 허가" : "출입 거부") << endl;
    }

    void closeConnection() {
        if(socket_fd >= 0) {
            close(socket_fd);
            system("sudo ifconfig can0 down");
        }
    }
};

// 얼굴 인식 데이터 수신 스레드 함수
void receiveFaceRecognition(int socket, SharedData& sharedData, AccessControlSystem& acs) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        ssize_t receivedBytes = recv(socket, buffer, BUFFER_SIZE, 0);
        if (receivedBytes > 0) {
            // 수신된 데이터가 "1"이면 얼굴 인식 성공
            sharedData.faceRecognition = (buffer[0] == '1');
            acs.sendCANFrame();  // CAN 프레임 즉시 전송
            memset(buffer, 0, BUFFER_SIZE);
        } else if (receivedBytes == 0) {
            cout << "서버 연결 종료." << endl;
            break;
        } else {
            cerr << "수신 실패: " << strerror(errno) << endl;
            break;
        }
    }
}

// 키패드 입력 처리 스레드 함수
void handleKeypadInput(SharedData& sharedData, AccessControlSystem& acs) {
    while (true) {
        string input_password;
        cout << "\n비밀번호를 입력하세요 (종료하려면 'exit'): ";
        getline(cin, input_password);

        if (input_password == "exit") {
            cout << "프로그램 종료..." << endl;
            exit(0);
        }

        sharedData.passwordMatch = (input_password == PASSWORD);
        acs.sendCANFrame();  // CAN 프레임 즉시 전송
    }
}

int main() {
    SharedData sharedData;
    AccessControlSystem acs(sharedData);

    // CAN 초기화
    if (!acs.initializeCAN()) {
        cerr << "CAN 초기화 실패" << endl;
        return -1;
    }

    // TCP 소켓 생성 및 연결
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "소켓 생성 실패: " << strerror(errno) << endl;
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(TCP_PORT);

    if (inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "잘못된 주소 또는 지원되지 않는 주소" << endl;
        close(clientSocket);
        return -1;
    }

    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        cerr << "연결 실패: " << strerror(errno) << endl;
        close(clientSocket);
        return -1;
    }

    cout << "서버에 연결됨!" << endl;

    // 스레드 시작
    thread faceRecognitionThread(receiveFaceRecognition, clientSocket, ref(sharedData), ref(acs));
    thread keypadThread(handleKeypadInput, ref(sharedData), ref(acs));

    // 메인 스레드에서 스레드 종료 대기
    faceRecognitionThread.join();
    keypadThread.join();

    close(clientSocket);
    return 0;
}
