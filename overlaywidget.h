#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QtWidgets/QWidget>
#include "AudioFunctions.h"

namespace Ui {
class OverlayWidget;
}

class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget *parent = 0);
    ~OverlayWidget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_released();

    void on_pushButton_4_clicked();

    void on_comboBox_output_wearinghmd_currentIndexChanged(int index);
    void on_comboBox_output_notwearinghmd_currentIndexChanged(int index);

    void on_comboBox_input_wearinghmd_currentIndexChanged(int index);
    void on_comboBox_input_notwearinghmd_currentIndexChanged(int index);

    void on_pushButton_save_clicked();

private:
    Ui::OverlayWidget *ui;

    QList<AudioDeviceInfo> outputDevices;
    QList<AudioDeviceInfo> inputDevices;
};

#endif // OVERLAYWIDGET_H
