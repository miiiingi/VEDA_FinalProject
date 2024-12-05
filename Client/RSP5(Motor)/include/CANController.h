#pragma once
#ifndef CAN_CONTROLLER_H
#define CAN_CONTROLLER_H

#include <linux/can.h>
#include <string>

class CANController {
private:
    int canSocket;
    void setupFilters();

public:
    CANController();
    ~CANController();
    void initialize();
    int getSocket() const { return canSocket; }
    bool receiveMessage(struct can_frame& frame);
    bool sendMessage(const struct can_frame& frame);
};

#endif