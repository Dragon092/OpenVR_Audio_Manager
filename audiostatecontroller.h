#ifndef AUDIOSTATECONTROLLER_H
#define AUDIOSTATECONTROLLER_H

#include <QString>
#include <AudioFunctions.h>

struct SPreferences
{
    QString output_wearinghmd;
    QString output_notwearinghmd;

    QString input_wearinghmd;
    QString input_notwearinghmd;
};

class AudioStateController
{
public:
    static AudioStateController *SharedInstance();
public:
    AudioStateController();

    void loadSettings();
    void saveSettings();
    void clearSettings();

    void setDevice(AudioDeviceDirection direction, std::string deviceID);

    SPreferences m_prefs;
};

#endif // AUDIOSTATECONTROLLER_H
