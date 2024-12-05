#pragma once
#ifndef DOORLOCKCONTROLLER_H
#define DOORLOCKCONTROLLER_H

#include "Motor.h"
#include "CANController.h"
#include <linux/can.h>

class DoorLockController {
private:
    Motor motor;
    CANController canController;
    void handleAuthenticationMessage(const struct can_frame& frame);
    void unlockDoor();
    void lockDoor();

public:
    DoorLockController();
    void processMessages();
    void run();
};

#endif