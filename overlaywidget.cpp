#include "overlaywidget.h"
#include "ui_overlaywidget.h"

#include "AudioFunctions.h"
#include "audiostatecontroller.h"
#include <QDebug>

OverlayWidget::OverlayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverlayWidget)
{
    ui->setupUi(this);

    /////////////////////////////////////////////////////

    const auto outputList = GetAudioDeviceList(AudioDeviceDirection::OUTPUT);
    const auto inputList = GetAudioDeviceList(AudioDeviceDirection::INPUT);



    qDebug() << "-----";
    qDebug() << "Found Output devices:";
    for (const auto& kv : outputList) {
        // List only connected or present devices
        if(kv.second.state != AudioDeviceState::CONNECTED && kv.second.state != AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION){
            continue;
        }

        outputDevices.append(kv.second);

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


    qDebug() << "-----";
    qDebug() << "Found Input devices:";
    for (const auto& kv : inputList) {
        // List only connected or present devices
        if(kv.second.state != AudioDeviceState::CONNECTED && kv.second.state != AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION){
            continue;
        }

        inputDevices.append(kv.second);

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


    std::string default_output_default = GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::DEFAULT);
    std::string default_output_communication = GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::COMMUNICATION);

    qDebug() << "default output default:           " << default_output_default.data();
    qDebug() << "default output communication:     " << default_output_communication.data();
    qDebug() << "-----";

    std::string default_input_default = GetDefaultAudioDeviceID(AudioDeviceDirection::INPUT, AudioDeviceRole::DEFAULT);
    std::string default_input_communication = GetDefaultAudioDeviceID(AudioDeviceDirection::INPUT, AudioDeviceRole::COMMUNICATION);

    qDebug() << "default input default:           " << default_input_default.data();
    qDebug() << "default input communication:     " << default_input_communication.data();
    qDebug() << "-----";


    //AudioStateController::SharedInstance()->m_prefs.default_output_default = QString::fromStdString(default_output_default);
    //AudioStateController::SharedInstance()->m_prefs.default_output_communication = QString::fromStdString(default_output_communication);
    //AudioStateController::SharedInstance()->saveSettings();

    // 0 is None so we start at 1
    int index_output = 1;
    for (const auto& kv : outputDevices) {
        ui->comboBox_output_wearinghmd->addItem(QString::fromStdString(kv.displayName));
        ui->comboBox_output_notwearinghmd->addItem(QString::fromStdString(kv.displayName));

        if(AudioStateController::SharedInstance()->m_prefs.output_wearinghmd == QString::fromStdString(kv.id)){
            qDebug() << "Found saved output_wearinghmd " << QString::fromStdString(kv.id) << " at " << index_output;
            ui->comboBox_output_wearinghmd->setCurrentIndex(index_output);
        }

        if(AudioStateController::SharedInstance()->m_prefs.output_notwearinghmd == QString::fromStdString(kv.id)){
            qDebug() << "Found saved output_notwearinghmd " << QString::fromStdString(kv.id) << " at " << index_output;
            ui->comboBox_output_notwearinghmd->setCurrentIndex(index_output);
        }

        index_output++;
    }

    // 0 is None so we start at 1
    int index_input = 1;
    for (const auto& kv : inputDevices) {
        ui->comboBox_input_wearinghmd->addItem(QString::fromStdString(kv.displayName));
        ui->comboBox_input_notwearinghmd->addItem(QString::fromStdString(kv.displayName));

        if(AudioStateController::SharedInstance()->m_prefs.input_wearinghmd == QString::fromStdString(kv.id)){
            qDebug() << "Found saved input_wearinghmd " << QString::fromStdString(kv.id) << " at " << index_input;
            ui->comboBox_input_wearinghmd->setCurrentIndex(index_input);
        }

        if(AudioStateController::SharedInstance()->m_prefs.input_notwearinghmd == QString::fromStdString(kv.id)){
            qDebug() << "Found saved input_notwearinghmd " << QString::fromStdString(kv.id) << " at " << index_input;
            ui->comboBox_input_notwearinghmd->setCurrentIndex(index_input);
        }

        index_input++;
    }

}

OverlayWidget::~OverlayWidget()
{
    delete ui;
}

void OverlayWidget::on_pushButton_quit_clicked()
{
    QApplication::quit();
}

void OverlayWidget::on_pushButton_save_clicked()
{
    AudioStateController::SharedInstance()->saveSettings();
}

void OverlayWidget::on_pushButton_refresh_clicked()
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

void OverlayWidget::on_pushButton_clear_settings_clicked()
{
    AudioStateController::SharedInstance()->clearSettings();
}
