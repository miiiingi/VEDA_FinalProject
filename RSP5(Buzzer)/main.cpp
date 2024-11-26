#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <wiringPi.h>
#include <softTone.h>

// Sound Management Class
class SoundManager {
private:
    static constexpr int SPKR = 6;

    const std::vector<int> resetNote{0, 0, 0};
    const std::vector<int> openNote{523, 659, 784, 0};
    const std::vector<int> failNote{784, 659, 523, 0};
    const std::vector<int> alarmNote{880, 440, 880, 440, 880, 440, 880, 440, 880, 440, 880, 440, 0};
    const std::vector<int> dingdongNote{784, 659, 0};

public:
    void initialize() {
        pinMode(SPKR, OUTPUT);
    }

    void playResetTone() {
        playTone(resetNote);
    }

    void playOpenTone() {
        playTone(openNote);
    }

    void playFailTone() {
        playTone(failNote);
    }

    void playAlarmTone() {
        playTone(alarmNote);
    }

    void playDingDongTone() {
        playTone(dingdongNote);
    }

private:
    void playTone(const std::vector<int>& notes) {
        softToneCreate(SPKR);
        for (auto note : notes) {
            if (note == 0) break;
            softToneWrite(SPKR, note);
            delay(200);
        }
        softToneStop(SPKR);
    }
};

// LED Management Class
class LEDController {
private:
    static constexpr int RLED = 1;
    static constexpr int GLED = 2;
    static constexpr int BLED = 28;
    static constexpr int YLED = 29;
    
    
    std::atomic<bool> ledG{false};
    std::atomic<bool> ledR{false};
    std::atomic<bool> ledB{false};
    std::atomic<bool> ledY{false};


public:
    void initialize() {
        pinMode(RLED, OUTPUT);
        pinMode(GLED, OUTPUT);
        pinMode(BLED, OUTPUT);
        pinMode(YLED, OUTPUT);
    }

    void setGreenLED(bool state) {
        ledG = state;
    }

    void setRedLED(bool state) {
        ledR = state;
    }

    void setBlueLED(bool state) {
        ledB = state;
    }

    void setYellowLED(bool state) {
        ledY = state;
    }

    void manageLights() {
        while (true) {
            if (ledG) {
                blinkGreenLED();
                ledG = false;
            }
            else if (ledR) {
                blinkRedLED();
                ledR = false;
            }
            else if (ledB) {
                blinkBlueLED();
                ledB = false;
            }
            else if (ledY) {
                blinkYellowLED();
                ledY = false;
            }
            std::this_thread::yield();
        }
    }

private:
    void blinkGreenLED() {
        for (int i = 0; i < 5; i++) {   
            digitalWrite(GLED, HIGH);
            delay(1000);
            digitalWrite(GLED, LOW);
            delay(1000);
        }        
        digitalWrite(GLED, LOW);
    }

    void blinkRedLED() {
        for (int i = 0; i < 5; i++) {
            digitalWrite(RLED, HIGH);
            delay(1000);
            digitalWrite(RLED, LOW);
            delay(1000);
        }
        digitalWrite(RLED, LOW);
    }

    void blinkBlueLED() {
        for (int i = 0; i < 5; i++) {
            digitalWrite(BLED, HIGH);
            delay(1000);
            digitalWrite(BLED, LOW);
            delay(1000);
        }
        digitalWrite(BLED, LOW);
    }

    void blinkYellowLED() {
        for (int i = 0; i < 5; i++) {
            digitalWrite(YLED, HIGH);
            delay(1000);
            digitalWrite(YLED, LOW);
            delay(1000);
        }
        digitalWrite(YLED, LOW);
    }
};

// Switch Event Handler Class
class SwitchHandler {
private:
    static constexpr int SW = 5;
    SoundManager& soundManager;

public:
    SwitchHandler(SoundManager& sm) : soundManager(sm) {}

    void initialize() {
        pinMode(SW, INPUT);
    }

    void handleSwitchEvents() {
        while (true) {
            if (digitalRead(SW) == LOW) {
                delay(50);
                if (digitalRead(SW) == LOW) {
                    std::cout << "Switch pressed" << std::endl;
                    soundManager.playDingDongTone();
                }    
                delay(100);
            }
            std::this_thread::yield();
        }
    }
};

// CAN Communication Class
class CANCommunicationManager {
private:
    int socketFd{-1};
    struct sockaddr_can addr{};
    struct ifreq ifr{};
    LEDController& ledController;
    SoundManager& soundManager;
    std::atomic<int> failCounter{0};
    std::atomic<bool> running{true};
    static constexpr int RECONNECT_DELAY_MS = 1000;  // 재연결 대기 시간 (1초)

public:
    CANCommunicationManager(LEDController& lc, SoundManager& sm) 
        : ledController(lc), soundManager(sm) {}

    void initializeCANSocket() {
        closeCANSocket();  // 기존 소켓이 있다면 닫기
        
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

        // Set receive filter
        struct can_filter rfilter[1];
        rfilter[0].can_id = 0x123;
        rfilter[0].can_mask = CAN_SFF_MASK;
        setsockopt(socketFd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    }

    void startCommunication() {
        while (running) {
            try {
                initializeCANSocket();
                std::cout << "CAN connection ready" << std::endl;
                
                struct can_frame frame;
                int nbytes = read(socketFd, &frame, sizeof(frame));
                
                if (nbytes > 0) {
                    processCANMessage(frame);
                } else {
                    throw std::runtime_error("CAN read error");
                }
            }
            catch (const std::exception& e) {
                std::cerr << "CAN error: " << e.what() << std::endl;
            }
            
            closeCANSocket();
            //std::cout << "CAN socket closed, waiting to reconnect..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
        }
    }

    void stop() {
        running = false;
    }

    ~CANCommunicationManager() {
        stop();
        closeCANSocket();
        system("sudo ifconfig can0 down");
    }

private:
    void closeCANSocket() {
        if (socketFd != -1) {
            close(socketFd);
            socketFd = -1;
        }
    }

    void processCANMessage(const struct can_frame& frame) {
        std::cout << "CAN ID: 0x" << std::hex << frame.can_id 
                  << ", Data Length: " << std::dec << static_cast<int>(frame.can_dlc) 
                  << ", First Byte: " << static_cast<int>(frame.data[0]) << std::endl;

        if ((frame.data[0] == 1) && (frame.data[1] == 1)) {
            std::cout << "Open" << std::endl;    
            ledController.setGreenLED(true);
            //soundManager.playOpenTone();
        }
        else {
            if ((failCounter == 3)||((frame.data[1]==0)&&(frame.data[0]==0))) {
                std::cout << "Alarm" << std::endl;
                ledController.setRedLED(true);
                //soundManager.playAlarmTone();
                failCounter = 0;
            }
            else if (frame.data[0]==0){
                std::cout << "Retry password" << std::endl;
                ledController.setBlueLED(true);
                //soundManager.playFailTone();
                failCounter++;
            }
            else{
                std::cout << "Retry face reconition" << std::endl;
                ledController.setYellowLED(true);
                //soundManager.playFailTone();
                failCounter++;
            }
        }
    }
};

// Main Controller Class
class DeviceController {
private:
    SoundManager soundManager;
    LEDController ledController;
    SwitchHandler switchHandler;
    CANCommunicationManager canManager;

    std::thread lightThread;
    std::thread switchThread;
    std::thread canThread;

public:
    DeviceController() : 
        switchHandler(soundManager), 
        canManager(ledController, soundManager) {
        
        // Initialize components
        wiringPiSetup();
        soundManager.initialize();
        ledController.initialize();
        switchHandler.initialize();
    }

    void start() {
        try {
            // Play reset tone
            soundManager.playResetTone();

            // Start threads
            lightThread = std::thread(&LEDController::manageLights, &ledController);
            switchThread = std::thread(&SwitchHandler::handleSwitchEvents, &switchHandler);
            canManager.initializeCANSocket();
            canThread = std::thread(&CANCommunicationManager::startCommunication, &canManager);

            // Wait for threads
            lightThread.join();
            switchThread.join();
            canThread.join();
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    ~DeviceController() {
        // Detach threads if they are joinable
        if (lightThread.joinable()) lightThread.detach();
        if (switchThread.joinable()) switchThread.detach();
        if (canThread.joinable()) canThread.detach();
    }
};

int main() {
    try {
        DeviceController controller;
        controller.start();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
