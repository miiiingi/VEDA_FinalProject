#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

using namespace std;

class CANController {
private:
    const string PASSWORD = "1234";
    const int MAX_PASSWORD_LENGTH = 20;
    int socket_fd;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

public:
    CANController() : socket_fd(-1) {
        memset(&frame, 0, sizeof(struct can_frame));
    }

    ~CANController() {
        closeConnection();
    }

    bool initializeCAN() {
        system("sudo ip link set can0 type can bitrate 100000");
        system("sudo ifconfig can0 up");
        return true;
    }

    bool createSocket() {
        socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (socket_fd < 0) {
            cerr << "Socket PF_CAN failed" << endl;
            return false;
        }
        return true;
    }

    bool configureDevice() {
        strcpy(ifr.ifr_name, "can0");
        if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
            cerr << "ioctl failed" << endl;
            return false;
        }
        return true;
    }

    bool bindSocket() {
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            cerr << "bind failed" << endl;
            return false;
        }
        return true;
    }

    void disableFilters() {
        setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    }

    void prepareFrame(bool isPasswordCorrect) {
        frame.can_id = 0x123;
        frame.can_dlc = 8;
        frame.data[0] = isPasswordCorrect ? 1 : 0;
        
        for(int i = 1; i < 8; i++) {
            frame.data[i] = 0;
        }
    }

    void printFrameInfo() const {
        cout << "can_id  = 0x" << hex << frame.can_id << dec << endl;
        cout << "can_dlc = " << static_cast<int>(frame.can_dlc) << endl;
        cout << "Verification result = " << static_cast<int>(frame.data[0]) << endl;
    }

    bool sendFrame() {
        int nbytes = write(socket_fd, &frame, sizeof(frame));
        if(nbytes != sizeof(frame)) {
            cerr << "Send Error frame[0]!" << endl;
            return false;
        }
        return true;
    }

    void closeConnection() {
        if(socket_fd >= 0) {
            close(socket_fd);
            system("sudo ifconfig can0 down");
        }
    }

    bool verifyPassword(const string& input_password) {
        return input_password == PASSWORD;
    }
};

int main() {
    CANController controller;
    string input_password;

    cout << "CAN Password Verification Demo\n";
    
    // Initialize CAN interface
    if (!controller.initializeCAN()) {
        return 1;
    }

    // Get password from user
    cout << "Please enter password: ";
    getline(cin, input_password);

    // Create and configure socket
    if (!controller.createSocket() || 
        !controller.configureDevice() || 
        !controller.bindSocket()) {
        return 1;
    }

    // Disable filters
    controller.disableFilters();

    // Verify password and prepare frame
    bool isPasswordCorrect = controller.verifyPassword(input_password);
    cout << (isPasswordCorrect ? "Password correct!\n" : "Password incorrect!\n");
    
    controller.prepareFrame(isPasswordCorrect);
    controller.printFrameInfo();

    // Send frame
    if (!controller.sendFrame()) {
        return 1;
    }

    return 0;
}