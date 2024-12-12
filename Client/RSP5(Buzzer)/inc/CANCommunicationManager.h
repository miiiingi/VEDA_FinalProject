#ifndef CANCOMMUNICATIONMANAGER_H
#define CANCOMMUNICATIONMANAGER_H

#include <atomic>
#include <thread>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include "LEDController.h"
#include "SoundManager.h"


class CANCommunicationManager {
private:
    int socketFd{-1};
    struct sockaddr_can addr{};
    struct ifreq ifr{};
    LEDController& ledController;
    SoundManager& soundManager;
    std::atomic<int> failCounter{1};
    std::thread soundThread;
    std::thread ledThread;

public:
    CANCommunicationManager(LEDController& lc, SoundManager& sm);
    void initializeCANSocket();
    bool processCANMessage();
    bool sendCANMessage(uint32_t can_id, uint8_t data);
    int getSocketFd() const;
    ~CANCommunicationManager();

private:
    void closeCANSocket();
};

#endif
