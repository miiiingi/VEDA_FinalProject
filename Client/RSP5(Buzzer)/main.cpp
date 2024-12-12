#include <wiringPi.h>
#include "inc/SoundManager.h"
#include "inc/LEDController.h"
#include "inc/CANCommunicationManager.h"
#include "inc/Configure.h"

volatile bool interruptFlag = false;

void switchInterrupt() {
    interruptFlag = true; 
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

        canManager.processCANMessage(); 
    }

    soundManager.playResetTone();
    return 0;
}
