#ifndef CAN_CONTROLLER_H
#define CAN_CONTROLLER_H

#include <linux/can.h>
#include "Motor.h"

class CANController {
private:
    int canSocket;
    Motor& motor;
    static const char* CAN_INTERFACE;
    static const int MOTOR_DELAY = 5;

    void setupCAN();
    void sendCANMessage(struct can_frame& frame);

public:
    CANController(Motor& motorRef);
    ~CANController();
    void processMessages();
};

#endif