#include "../include/Motor.h"
#include "Motor.h"
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

Motor::Motor(int pin) : motorPin(pin) {
    if (wiringPiSetup() == -1) {
        printf("WiringPi 초기화 실패\n");
        exit(1);
    }

    if (softPwmCreate(motorPin, 0, 100) != 0) {
        printf("PWM 설정 실패\n");
        exit(1);
    }
    
    softPwmWrite(motorPin, 0);
}

Motor::~Motor() {
    stop();
}

void Motor::setDegree(int degree) {
    if (degree > 180) degree = 180;
    if (degree < 0) degree = 0;

    float duty = MOTOR_MIN_DUTY + (degree * (MOTOR_MAX_DUTY - MOTOR_MIN_DUTY) / 180.0);
    softPwmWrite(motorPin, (int)duty);
    usleep(300000);
    softPwmWrite(motorPin, 0);
    
    printf("모터 각도 설정: %d도\n", degree);
}

void Motor::stop() {
    softPwmWrite(motorPin, 0);
}