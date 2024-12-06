#ifndef MOTOR_H
#define MOTOR_H

class Motor {
private:
    int motorPin;
    static const int MOTOR_MIN_DUTY = 5;
    static const int MOTOR_MAX_DUTY = 25;

public:
    Motor(int pin = 1);
    ~Motor();
    void setDegree(int degree);
    void stop();
};

#endif