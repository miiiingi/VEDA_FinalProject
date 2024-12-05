#pragma once
#ifndef MOTOR_H
#define MOTOR_H

#include <wiringPi.h>
#include <softPwm.h>

class Motor {
private:
    int motorPin;
    bool isEnabled;

public:
    Motor(int pin);
    ~Motor();
    void setSpeed(int speed);
    void stop();
};

#endif