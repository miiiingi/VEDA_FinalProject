#include "../inc/LEDController.h"
#include "../inc/Configure.h"
#include <wiringPi.h>



void LEDController::initialize() {
    pinMode(RLED, OUTPUT);
    pinMode(GLED, OUTPUT);
    pinMode(BLED, OUTPUT);
    pinMode(YLED, OUTPUT);
}

void LEDController::setGreenLED() {
    blinkLED(GLED);
}

void LEDController::setRedLED() {
    blinkLED(RLED);
}

void LEDController::setBlueLED() {
    blinkLED(BLED);
}

void LEDController::setYellowLED() {
    blinkLED(YLED);
}

void LEDController::blinkLED(int pin) {
    for (int i = 0; i < 3; i++) {   
        digitalWrite(pin, HIGH);
        delay(600);
        digitalWrite(pin, LOW);
        delay(600);
    }        
    digitalWrite(pin, LOW);
}