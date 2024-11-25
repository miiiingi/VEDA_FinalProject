#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define PASSWORD "1234"
#define MAX_PASSWORD_LENGTH 20

int main()
{
    int ret;
    int s, nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    char input_password[MAX_PASSWORD_LENGTH];

    memset(&frame, 0, sizeof(struct can_frame));
    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");
    printf("CAN Password Verification Demo\r\n");
    
    // Get password from user
    printf("Please enter password: ");
    scanf("%s", input_password);
    
    //1.Create socket
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("socket PF_CAN failed");
        return 1;
    }
    
    //2.Specify can0 device
    strcpy(ifr.ifr_name, "can0");
    ret = ioctl(s, SIOCGIFINDEX, &ifr);
    if (ret < 0) {
        perror("ioctl failed");
        return 1;
    }
    
    //3.Bind the socket to can0
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind failed");
        return 1;
    }
    
    //4.Disable filtering rules, do not receive packets, only send
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    //5.Set send data
    frame.can_id = 0x123;
    frame.can_dlc = 8;
    
    // Compare password and set data accordingly
    if (strcmp(input_password, PASSWORD) == 0) {
        printf("Password correct!\r\n");
        frame.data[0] = 1;  // 1 means password correct
    } else {
        printf("Password incorrect!\r\n");
        frame.data[0] = 0;  // 0 means password incorrect
    }
    
    // Fill remaining data bytes with zeros
    for(int i = 1; i < 8; i++) {
        frame.data[i] = 0;
    }
    
    printf("can_id  = 0x%X\r\n", frame.can_id);
    printf("can_dlc = %d\r\n", frame.can_dlc);
    printf("Verification result = %d\r\n", frame.data[0]);
    
    //6.Send message
    nbytes = write(s, &frame, sizeof(frame)); 
    if(nbytes != sizeof(frame)) {
        printf("Send Error frame[0]!\r\n");
        system("sudo ifconfig can0 down");
    }
    
    //7.Close the socket and can0
    close(s);
    system("sudo ifconfig can0 down");
    return 0;
}
