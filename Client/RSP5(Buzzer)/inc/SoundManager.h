#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H
#include <vector>

class SoundManager {
private:
    const std::vector<int> resetNote{0, 0, 0};
    const std::vector<int> openNote{523, 659, 784, 0};
    const std::vector<int> closeNote{784, 659, 523, 0};
    const std::vector<int> alarmNote{880, 440, 880, 440, 880, 440, 880, 440, 880, 440, 880, 440, 0};
    const std::vector<int> dingdongNote{784, 659, 0};
    const std::vector<int> failNote{500, 1000, 0};

private:
    void playTone(const std::vector<int>& notes);

public:
    void initialize();
    void playResetTone();
    void playOpenTone();
    void playCloseTone();
    void playFailTone();
    void playAlarmTone();
    void playDingDongTone();
};

#endif