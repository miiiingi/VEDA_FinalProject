#include "../inc/SoundManager.h"
#include <wiringPi.h>
#include <softTone.h>

#define SPKR 6

void SoundManager::initialize() {
    pinMode(SPKR, OUTPUT);
    softToneCreate(SPKR);
}

void SoundManager::playResetTone() { playTone(resetNote); }
void SoundManager::playOpenTone() { playTone(openNote); }
void SoundManager::playCloseTone() { playTone(closeNote); }
void SoundManager::playFailTone() { playTone(failNote); }
void SoundManager::playAlarmTone() { playTone(alarmNote); }
void SoundManager::playDingDongTone() { playTone(dingdongNote); }

void SoundManager::playTone(const std::vector<int>& notes) {
    for (auto note : notes) {
        if (note == 0) break;
        softToneWrite(SPKR, note);
        delay(200);
    }
    softToneWrite(SPKR,0);
}