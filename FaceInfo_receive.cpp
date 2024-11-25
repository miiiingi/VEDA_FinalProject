#include <iostream>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const int TCP_PORT = 5100;
const int BUFFER_SIZE = 1024;
const string SERVER_IP = "192.168.1.19";  // 젯슨 나노 IP

void receiveMessages(int socket) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        ssize_t receivedBytes = recv(socket, buffer, BUFFER_SIZE, 0);
        if (receivedBytes > 0) {
            cout << "\n[서버] " << buffer << endl;
            memset(buffer, 0, BUFFER_SIZE);  // 버퍼 초기화
        } else if (receivedBytes == 0) {
            cout << "서버 연결 종료." << endl;
            break;
        } else {
            cerr << "수신 실패: " << strerror(errno) << endl;
            break;
        }
    }
}

int main() {
    // 소켓 생성
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "소켓 생성 실패: " << strerror(errno) << endl;
        return -1;
    }

    // 서버 주소 구조체 준비
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(TCP_PORT);

    // IP 주소 변환
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "잘못된 주소 또는 지원되지 않는 주소" << endl;
        close(clientSocket);
        return -1;
    }

    // 서버 연결
    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        cerr << "연결 실패: " << strerror(errno) << endl;
        close(clientSocket);
        return -1;
    }

    cout << "서버에 연결됨!" << endl;

    // 수신 스레드 시작
    thread receiver(receiveMessages, clientSocket);

    // 메시지 전송 루프
    while (true) {
        char message[BUFFER_SIZE];
        cout << "[나] 보낼 메시지를 입력하세요 (종료하려면 'exit'): ";
        cin.getline(message, BUFFER_SIZE);

        // "exit" 입력 시 종료
        if (strcmp(message, "exit") == 0) {
            cout << "클라이언트 종료..." << endl;
            break;
        }

        // 메시지 전송
        ssize_t sentBytes = send(clientSocket, message, strlen(message), 0);
        if (sentBytes < 0) {
            cerr << "전송 실패: " << strerror(errno) << endl;
            break;
        }
    }

    // 소켓 닫기 및 스레드 종료
    close(clientSocket);
    receiver.detach();

    return 0;
}
