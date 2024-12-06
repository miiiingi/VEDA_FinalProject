#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

class LEDController {
private:
    void blinkLED(int pin);

public:
    void initialize();
    void setGreenLED();
    void setRedLED();
    void setBlueLED();
    void setYellowLED();
};

#endif