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
#include <iomanip>
#include <atomic>
#include <mutex>
#include <errno.h>
#include <openssl/sha.h>
#include <fstream>
#include <sstream>



using namespace std;

const int TCP_PORT = 5100;
const int BUFFER_SIZE = 1024;
const string SERVER_IP = "192.168.1.19";  // 젯슨 나노 IP
const string PASSWORD_HASH_FILE = "password_hash.txt";

// 비밀번호 해시 함수
string hashPassword(const string& password){
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256((unsigned char*)password.c_str(), password.size(), hash);
	ostringstream os;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		os << hex << setw(2) << setfill('0') << (int)hash[i];
	}
	return os.str();
}

// 비밀번호 해시 저장 함수
void savePasswordHashToFile(const string& hash, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << hash;
        file.close();
    }
}

// 비밀번호 해시 로드 함수
string loadPasswordHashFromFile(const string& filename) {
    ifstream file(filename);
    string hash;
    if (file.is_open()) {
        getline(file, hash);
        file.close();
    }
    return hash;
}



class AccessControlSystem {
private:
    // 비밀번호 관련 변수
    string stored_password_hash;	

    // const string CORRECT_PASSWORD = "1234";
    // const int MAX_PASSWORD_LENGTH = 20;

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

    // CAN 데이터 수신 관련 변수
    std::atomic<bool> is_running;
    std::thread can_receive_thread;
    std::mutex can_receive_mutex;

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

public:
    AccessControlSystem() : 
        can_socket_fd(-1), 
        client_socket(-1), 
        face_recognition_result(false),
        password_verification_result(false),
        final_authentication_status(false),
        is_running(false) {
        memset(&frame, 0, sizeof(struct can_frame));
        memset(&server_addr, 0, sizeof(server_addr));
	initializePasswordHash();
    }

    // 초기 비밀번호 해시 초기화 메서드
    void initializePasswordHash() {
        string existing_hash = loadPasswordHashFromFile(PASSWORD_HASH_FILE);

        if (existing_hash.empty()) {
            // 첫 실행 시 기본 비밀번호로 해시 생성
            string initial_password = "1234";
            string initial_hash = hashPassword(initial_password);
            savePasswordHashToFile(initial_hash, PASSWORD_HASH_FILE);
            stored_password_hash = initial_hash;
            cout << "초기 비밀번호 해시 생성 완료" << endl;
        } else {
            stored_password_hash = existing_hash;
        }
    }

    // 비밀번호 변경 메서드
    bool changePassword(const string& old_password, const string& new_password) {
        // 기존 비밀번호 검증
        string old_hash = hashPassword(old_password);

        if (old_hash == stored_password_hash) {
            // 새 비밀번호 해시 생성 및 저장
            string new_hash = hashPassword(new_password);
            savePasswordHashToFile(new_hash, PASSWORD_HASH_FILE);
            stored_password_hash = new_hash;

            cout << "비밀번호 변경 성공!" << endl;
            return true;
        }

        cout << "비밀번호 변경 실패: 기존 비밀번호가 일치하지 않습니다." << endl;
        return false;
    }

    // 비밀번호 검증 메서드 (해시 비교)
    bool verifyPassword(const string& input_password) {
        string input_hash = hashPassword(input_password);
        password_verification_result = (input_hash == stored_password_hash);
        return password_verification_result;
    }


    ~AccessControlSystem() {
        // Stop the CAN receive thread before closing connections
        stopCANReceiveThread();
        closeConnections();
    }

    // CAN 통신 초기화 메서드
    bool initializeCAN() {
	system("sudo ifconfig can0 down");
        // CAN 인터페이스 활성화 명령 실행 결과 확인
        int can_up_result = system("sudo ip link set can0 type can bitrate 100000");
        if (can_up_result != 0) {
            cerr << "CAN 인터페이스 설정 실패 (비트레이트): " << can_up_result << endl;
            return false;
        }

        can_up_result = system("sudo ifconfig can0 up");
        if (can_up_result != 0) {
            cerr << "CAN 인터페이스 UP 실패: " << can_up_result << endl;
            return false;
        }

        can_socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (can_socket_fd < 0) {
            cerr << "CAN 소켓 생성 실패: " << strerror(errno) << endl;
            return false;
        }

        strcpy(ifr.ifr_name, "can0");
        if (ioctl(can_socket_fd, SIOCGIFINDEX, &ifr) < 0) {
            cerr << "ioctl 실패: " << strerror(errno) 
                 << " (can0 인터페이스 존재 여부 확인)" << endl;
            return false;
        }

        can_addr.can_family = AF_CAN;
        can_addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(can_socket_fd, (struct sockaddr *)&can_addr, sizeof(can_addr)) < 0) {
            cerr << "CAN 소켓 바인딩 실패: " << strerror(errno) << endl;
            return false;
        }

        // 소켓 옵션 설정 시 오류 확인
        int recv_own_msgs = 1; // 자체 메시지 수신 허용
        if (setsockopt(can_socket_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs)) < 0) {
            cerr << "소켓 옵션 설정 실패: " << strerror(errno) << endl;
        }

        cout << "CAN 인터페이스 초기화 성공!" << endl;
        return true;
    }

    // 이더넷 통신 초기화 메서드
    bool initializeEthernet() {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            cerr << "소켓 생성 실패: " << strerror(errno) << endl;
            return false;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TCP_PORT);

        if (inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr) <= 0) {
            cerr << "잘못된 주소: " << strerror(errno) << endl;
            return false;
        }

        if (connect(client_socket, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            cerr << "서버 연결 실패: " << strerror(errno) << endl;
            return false;
        }

        cout << "젯슨 나노 서버에 연결됨!" << endl;
        return true;
    }

    // CAN 데이터 수신 메서드
    void receiveCANData() {
        struct can_frame rx_frame;
        int nbytes;
        int error_count = 0;
        const int MAX_CONSECUTIVE_ERRORS = 10;

        while (is_running) {
            // 타임아웃 설정을 위한 select() 사용
            fd_set rdfs;
            struct timeval tv;

            FD_ZERO(&rdfs);
            FD_SET(can_socket_fd, &rdfs);

            tv.tv_sec = 1;  // 1초 타임아웃
            tv.tv_usec = 0;

            int ret = select(can_socket_fd + 1, &rdfs, NULL, NULL, &tv);
            
            if (ret < 0) {
                cerr << "select() 오류: " << strerror(errno) << endl;
                error_count++;
                if (error_count > MAX_CONSECUTIVE_ERRORS) {
                    cerr << "연속된 오류로 CAN 수신 중단!" << endl;
                    break;
                }
                continue;
            }
            
            if (ret == 0) {
                // 타임아웃 (데이터 없음)
                cout << "CAN 수신 대기 중... (1초 타임아웃)" << endl;
                continue;
            }

            // 데이터 읽기
            nbytes = read(can_socket_fd, &rx_frame, sizeof(rx_frame));
            
            if (nbytes < 0) {
                cerr << "CAN 데이터 읽기 오류: " << strerror(errno) << endl;
                error_count++;
                if (error_count > MAX_CONSECUTIVE_ERRORS) {
                    cerr << "연속된 오류로 CAN 수신 중단!" << endl;
                    break;
                }
                continue;
            }

            if (nbytes == sizeof(struct can_frame)) {
                std::lock_guard<std::mutex> lock(can_receive_mutex);
                
                // 상세한 CAN 프레임 정보 출력
                cout << "CAN 프레임 수신 (상세):" << endl;
                cout << "- 프레임 ID: 0x" << hex << rx_frame.can_id << endl;
                cout << "- 데이터 길이: " << dec << static_cast<int>(rx_frame.can_dlc) << endl;
                
                cout << "- 데이터 내용: ";
                for (int i = 0; i < rx_frame.can_dlc; ++i) {
                    cout << hex << setw(2) << setfill('0') 
                         << static_cast<int>(rx_frame.data[i]) << " ";
                }
                cout << endl;

                // 이더넷으로 데이터 전송 로직
                char eth_buffer[BUFFER_SIZE] = {0};
                int data_to_send = rx_frame.data[0];
                snprintf(eth_buffer, sizeof(eth_buffer), "%d", data_to_send);

                ssize_t sent_bytes = send(client_socket, eth_buffer, strlen(eth_buffer), 0);
                if (sent_bytes < 0) {
                    cerr << "이더넷 데이터 전송 실패: " << strerror(errno) << endl;
                } else {
                    cout << "이더넷으로 " << sent_bytes << "바이트 전송 성공" << endl;
                }
            }
        }
    }

    // CAN 수신 스레드 시작 메서드
    void startCANReceiveThread() {
        is_running = true;
        can_receive_thread = std::thread(&AccessControlSystem::receiveCANData, this);
        
        // 스레드 분리 (자동으로 리소스 정리)
        can_receive_thread.detach();
    }

    // CAN 수신 스레드 중지 메서드
    void stopCANReceiveThread() {
        is_running = false;
        if (can_receive_thread.joinable()) {
            can_receive_thread.join();
        }
    }

    // 이더넷으로부터 얼굴인식 결과 수신
    bool receiveFaceRecognitionResult() {
        char buffer[BUFFER_SIZE] = {0};
        ssize_t receivedBytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (receivedBytes > 0) {
            // 디버그: 수신된 얼굴인식 결과 출력
            cout << "디버그: 얼굴인식 결과 수신 (문자열): [" 
                 << buffer << "]" << endl;
            
            // 디버그: 수신된 바이트 수와 첫 번째 문자 출력
            cout << "디버그: 수신된 바이트 수: " << receivedBytes 
                 << ", 첫 번째 문자: " << (int)buffer[0] << endl;
            
            face_recognition_result = (buffer[0] == '1');
            return true;
        }
        cerr << "얼굴인식 결과 수신 실패: " << strerror(errno) << endl;
        return false;
    }



    // 최종 인증 상태 결정
    void determineFinalAuthentication() {
        final_authentication_status = (face_recognition_result && password_verification_result);
    }

    // CAN 프레임 및 이더넷으로 인증 상태 전송
    bool sendAuthenticationStatus() {
        // 인증 상태를 1바이트로 인코딩
        uint8_t auth_status = 0;
        
        // 비밀번호 일치 여부를 2번째 비트에 설정
        if (password_verification_result) {
            auth_status |= (1 << 1);
        }
        
        // 얼굴 인식 결과를 1번째 비트에 설정
        if (face_recognition_result) {
            auth_status |= (1 << 0);
        }

        // CAN 통신으로 데이터 전송
        frame.can_id = 0x001;
        frame.can_dlc = 1;
        frame.data[0] = auth_status;

        int nbytes = write(can_socket_fd, &frame, sizeof(frame));
        if (nbytes != sizeof(frame)) {
            cerr << "CAN 프레임 전송 실패: " << strerror(errno) << endl;
            return false;
        }

        // 이더넷으로 젯슨 나노 서버에 전송
        char eth_buffer[2] = {0};
        snprintf(eth_buffer, sizeof(eth_buffer), "%d", auth_status);

        // 디버그: 전송 버퍼 내용 출력
        cout << "디버그: 전송 버퍼 내용 (hex): ";
        for (int i = 0; i < sizeof(eth_buffer); ++i) {
            cout << hex << setw(2) << setfill('0') 
                 << static_cast<int>(static_cast<unsigned char>(eth_buffer[i])) << " ";
        }
        cout << endl;

        // 디버그: 전송 버퍼 내용 출력 (문자열)
        cout << "디버그: 전송 버퍼 내용 (문자열): [" 
             << eth_buffer << "]" << endl;

        // 디버그: auth_status 상세 정보 출력
        cout << "디버그: auth_status 값: " << dec 
             << static_cast<int>(auth_status) 
             << " (얼굴인식: " << face_recognition_result 
             << ", 비밀번호: " << password_verification_result << ")" << endl;

        ssize_t sent_bytes = send(client_socket, eth_buffer, 1, 0);
        if (sent_bytes != 1) {
            cerr << "이더넷으로 인증 상태 전송 실패: " << strerror(errno) << endl;
            return false;
        }

        return true;
    }

    // 키패드 입력 대기 및 처리 메서드
    void waitForSlashAndPassword() {
        char input;
        string password;

        cout << "키패드 대기 중 ('/' 입력을 대기합니다)..." << endl;

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

                // CAN 및 이더넷으로 인증 상태 전송
                sendAuthenticationStatus();

                // 결과 출력
                cout << "얼굴인식 결과: " << (face_recognition_result ? "성공" : "실패") << endl;
                cout << "비밀번호 검증: " << (password_verification_result ? "성공" : "실패") << endl;
                cout << "최종 인증 상태: " << (final_authentication_status ? "승인" : "거부") << endl;
            }
        }
    }

    // 시스템 초기화 및 실행
    bool initializeSystem() {
        if (!initializeCAN() || !initializeEthernet()) {
            cerr << "시스템 초기화 실패" << endl;
            return false;
        }

        // CAN 데이터 수신 스레드 시작
        startCANReceiveThread();

        return true;
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

