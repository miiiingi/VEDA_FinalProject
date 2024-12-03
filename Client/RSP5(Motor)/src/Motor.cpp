#include "../include/Motor.h"
#include "../include/Constants.h"
#include <iostream>
#include <cstdlib>

Motor::Motor(int pin) : motorPin(pin), isEnabled(false) {
    if (wiringPiSetupGpio() == -1) {
        throw std::runtime_error("WiringPi 초기화 실패");
    }

    if (softPwmCreate(motorPin, 0, PWM_RANGE) != 0) {
        throw std::runtime_error("PWM 설정 실패");
    }

    stop();
}

Motor::~Motor() {
    stop();
}

void Motor::setSpeed(int speed) {
    if (speed < -PWM_RANGE) speed = -PWM_RANGE;
    if (speed > PWM_RANGE) speed = PWM_RANGE;

    softPwmWrite(motorPin, abs(speed));

    if (speed < 0) {
        std::cout << "모터 역방향 회전 (PWM: " << abs(speed) << ")\n";
    } else if (speed > 0) {
        std::cout << "모터 정방향 회전 (PWM: " << speed << ")\n";
    } else {
        std::cout << "모터 정지\n";
    }
}

void Motor::stop() {
    setSpeed(0);
}