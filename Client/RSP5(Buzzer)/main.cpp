#include <wiringPi.h>
#include "inc/SoundManager.h"
#include "inc/LEDController.h"
#include "inc/CANCommunicationManager.h"
#include "inc/Configure.h"

volatile bool interruptFlag = false; // 플래그 변수

void switchInterrupt() {
    interruptFlag = true; // 플래그 설정
}

int main() {
    wiringPiSetup();

    SoundManager soundManager;
    LEDController ledController;
    soundManager.initialize();
    ledController.initialize();
    CANCommunicationManager canManager(ledController, soundManager);

    pinMode(SW, INPUT);
    pullUpDnControl(SW, PUD_UP);
    wiringPiISR(SW, INT_EDGE_FALLING, switchInterrupt);

    canManager.initializeCANSocket();

    while (1) {
        if (interruptFlag) {
            interruptFlag = false; 
            soundManager.playDingDongTone();
            canManager.sendCANMessage(0x002, 0b00000101);
            std::cout << "dingdong" << std::endl;
        }

        canManager.processCANMessage(); // CAN 메시지 처리
    }

    soundManager.playResetTone();
    return 0;
}
