#include "audiostatecontroller.h"

#include <QSettings>
#include <QDebug>

AudioStateController *s_pSharedAudioStateController = NULL;

AudioStateController *AudioStateController::SharedInstance()
{
    if ( !s_pSharedAudioStateController )
    {
        s_pSharedAudioStateController = new AudioStateController();
    }
    return s_pSharedAudioStateController;
}

AudioStateController::AudioStateController()
{
    QSettings settings;
    qDebug() << "Settings fileName: " << settings.fileName();
    loadSettings();
}

void AudioStateController::loadSettings()
{
    qDebug() << "Loading settings...";

    QSettings settings;
    m_prefs.output_wearinghmd = settings.value("output_wearinghmd").toString();
    m_prefs.output_notwearinghmd = settings.value("output_notwearinghmd").toString();
    m_prefs.input_wearinghmd = settings.value("input_wearinghmd").toString();
    m_prefs.input_notwearinghmd = settings.value("input_notwearinghmd").toString();

    //qDebug() << "default_output_default: " << m_prefs.default_output_default;
    //qDebug() << "default_output_communication: " << m_prefs.default_output_communication;
}

void AudioStateController::saveSettings()
{
    qDebug() << "Saving settings...";

    QSettings settings;
    settings.setValue("output_wearinghmd", m_prefs.output_wearinghmd);
    settings.setValue("output_notwearinghmd", m_prefs.output_notwearinghmd);
    settings.setValue("input_wearinghmd", m_prefs.input_wearinghmd);
    settings.setValue("input_notwearinghmd", m_prefs.input_notwearinghmd);
}

void AudioStateController::setDevice(AudioDeviceDirection direction, std::string deviceID)
{
    std::string default_default = GetDefaultAudioDeviceID(direction, AudioDeviceRole::DEFAULT);
    std::string default_communication = GetDefaultAudioDeviceID(direction, AudioDeviceRole::COMMUNICATION);

    qDebug() << "default input default:           " << default_default.data();
    qDebug() << "default input communication:     " << default_communication.data();
    qDebug() << "new DeviceID:                    " << deviceID.data();
    qDebug() << "-----";

    if(deviceID != default_default){
        SetDefaultAudioDeviceID(direction, AudioDeviceRole::DEFAULT, deviceID);
    }

    if(deviceID != default_communication){
        SetDefaultAudioDeviceID(direction, AudioDeviceRole::COMMUNICATION, deviceID);
    }
}

void AudioStateController::clearSettings()
{
    qDebug() << "Clearing settings...";

    QSettings settings;
    settings.clear();
}
