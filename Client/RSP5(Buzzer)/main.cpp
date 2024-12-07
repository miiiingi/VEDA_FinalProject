#include <wiringPi.h>
#include "inc/SoundManager.h"
#include "inc/LEDController.h"
#include "inc/CANCommunicationManager.h"

#define SW 5

// 전역 포인터를 사용해 인터럽트 핸들러에서 접근
CANCommunicationManager* globalCanManager = nullptr;

void switchInterrupt() {
    if (globalCanManager) {
        SoundManager sound;
        globalCanManager->sendCANMessage(0x002, 0b00000101);
        std::cout << "dingdong" << std::endl;
    }
}

int main() {    
    wiringPiSetup(); 
    SoundManager soundManager;
    LEDController ledController;
    soundManager.initialize();
    ledController.initialize();
    CANCommunicationManager canManager(ledController, soundManager);
    globalCanManager = &canManager;
    pinMode(SW, INPUT);
    pullUpDnControl(SW, PUD_UP);
    wiringPiISR(SW, INT_EDGE_FALLING, switchInterrupt);

    while(1) {
        canManager.initializeCANSocket();
            
        while(true) {
            canManager.processCANMessage();
        }
    }
    
    return 0;
}