#pragma once
#include <QWidget>
#include <string>
#include <vector> 
 
class QPushButton;
class QComboBox;
class QLabel;
 
class Buttons : public QWidget
{
    Q_OBJECT
 
public:
    explicit Buttons(QWidget *parent = nullptr);
    void setBPM(double bpm);
    void setInputDevices(const std::vector<std::pair<int, std::string>> &devices, int selected);
    void setVolume(double rms);     
 
signals:
    void pauseToggled(bool paused);
    void clearRequested();
    void bpmScaleRequested(double factor);
    void inputDeviceSelected(int device);
 
private:
    QPushButton *makeButton(const QString &text, int width);

    QPushButton *playPauseButton;
    QPushButton *clearButton;
    QPushButton *bpmButton;
    QPushButton *bpmUpButton;
    QPushButton *bpmDownButton;
    bool paused = false;
    QComboBox *inputBox;
    QLabel *volumeMeter;
    int litLines = -1;
};