#include "SwitchHandler.h"
#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <chrono>
#include <linux/can.h>

SwitchHandler::SwitchHandler(SoundManager& sm, CANCommunicationManager& cm) 
    : soundManager(sm), canManager(cm) {}

void SwitchHandler::initialize() {
    pinMode(SW, INPUT);
}

void SwitchHandler::handleSwitchEvents() {
    while (true) {
        if (digitalRead(SW) == LOW) {
            delay(50);
            if (digitalRead(SW) == LOW) {
                std::cout << "Switch pressed" << std::endl;
                sendCANMessage();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }    
        }
        std::this_thread::yield();
    }
}

void SwitchHandler