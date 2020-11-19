#include "overlaywidget.h"
#include "ui_overlaywidget.h"

#include "AudioFunctions.h"
#include <QDebug>

OverlayWidget::OverlayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverlayWidget)
{
    ui->setupUi(this);

    /////////////////////////////////////////////////////

    const auto outputList = GetAudioDeviceList(AudioDeviceDirection::OUTPUT);
    //const auto inputList = GetAudioDeviceList(AudioDeviceDirection::INPUT);

    qDebug() << "-----";
    qDebug() << "Found Output devices:";
    for (const auto& kv : outputList) {
        // List only connected or present devices
        if(kv.second.state != AudioDeviceState::CONNECTED && kv.second.state != AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION){
            continue;
        }

        qDebug() << kv.first.data();
        qDebug() << kv.second.id.data();
        qDebug() << kv.second.interfaceName.data();
        qDebug() << kv.second.endpointName.data();
        qDebug() << kv.second.displayName.data();


        std::string stateString;

        if(kv.second.state == AudioDeviceState::CONNECTED){
            stateString = "CONNECTED";
        } else if(kv.second.state == AudioDeviceState::DEVICE_NOT_PRESENT){
            stateString = "DEVICE_NOT_PRESENT";
        } else if(kv.second.state == AudioDeviceState::DEVICE_DISABLED){
            stateString = "DEVICE_DISABLED";
        } else if(kv.second.state == AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION){
            stateString = "DEVICE_PRESENT_NO_CONNECTION";
        } else {
            qCritical() << "Error AudioDeviceState";
        }

        qDebug() << stateString.data();

        qDebug() << "---";
    }
    qDebug() << "-----";


    std::string default_default = GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::DEFAULT);
    std::string default_communication = GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::COMMUNICATION);

    qDebug() << "default default: " << default_default.data();
    qDebug() << "default communication" << default_communication.data();
    qDebug() << "-----";
}

OverlayWidget::~OverlayWidget()
{
    delete ui;
}

void OverlayWidget::on_pushButton_clicked()
{
    QApplication::quit();
}

void OverlayWidget::on_pushButton_3_clicked()
{

}

void OverlayWidget::on_pushButton_4_clicked()
{
    const auto outputList = GetAudioDeviceList(AudioDeviceDirection::OUTPUT);
    //const auto inputList = GetAudioDeviceList(AudioDeviceDirection::INPUT);

    qDebug() << "-----";
    qDebug() << "Found Output devices:";
    for (const auto& kv : outputList) {
        qDebug() << kv.first.data();
    }
    qDebug() << "-----";
}

void OverlayWidget::on_pushButton_released()
{

}

void OverlayWidget::on_comboBox_output_wearinghmd_currentIndexChanged(int index)
{
    QString selectedDeviceID = QString::fromStdString(outputDevices.at(index - 1).id);
    qDebug() << "Selected comboBox_output_wearinghmd entry with Index " << index << " and DeviceID " << selectedDeviceID;

    AudioStateController::SharedInstance()->m_prefs.output_wearinghmd = selectedDeviceID;

    AudioStateController::SharedInstance()->saveSettings();
}

void OverlayWidget::on_comboBox_output_notwearinghmd_currentIndexChanged(int index)
{
    QString selectedDeviceID = QString::fromStdString(outputDevices.at(index - 1).id);
    qDebug() << "Selected comboBox_output_notwearinghmd entry with Index " << index << " and DeviceID " << selectedDeviceID;

    AudioStateController::SharedInstance()->m_prefs.output_notwearinghmd = selectedDeviceID;

    AudioStateController::SharedInstance()->saveSettings();
}

void OverlayWidget::on_comboBox_input_wearinghmd_currentIndexChanged(int index)
{
    QString selectedDeviceID = QString::fromStdString(inputDevices.at(index - 1).id);
    qDebug() << "Selected comboBox_input_wearinghmd entry with Index " << index << " and DeviceID " << selectedDeviceID;

    AudioStateController::SharedInstance()->m_prefs.input_wearinghmd = selectedDeviceID;

    AudioStateController::SharedInstance()->saveSettings();
}

void OverlayWidget::on_comboBox_input_notwearinghmd_currentIndexChanged(int index)
{
    QString selectedDeviceID = QString::fromStdString(inputDevices.at(index - 1).id);
    qDebug() << "Selected comboBox_input_notwearinghmd entry with Index " << index << " and DeviceID " << selectedDeviceID;

    AudioStateController::SharedInstance()->m_prefs.input_notwearinghmd = selectedDeviceID;

    AudioStateController::SharedInstance()->saveSettings();
}
