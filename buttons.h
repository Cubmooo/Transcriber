#pragma once
#include <QWidget>
#include <string>
#include <vector>
#include "pitchdial.h"
#include <QToolButton>
#include <QMenu>
 
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
    void setPitch(double freq);
 
signals:
    void pauseToggled(bool paused);
    void clearRequested();
    void bpmScaleRequested(double factor);
    void inputDeviceSelected(int device);
 
private:
    QPushButton *makeButton(const QString &content, int width, const QString &shortcut = QString());
    QPushButton *makeButtonImage(const QIcon &icon, int width, const QString &shortcut = QString());

    QPushButton *playPauseButton;
    QPushButton *clearButton;
    QPushButton *bpmButton;
    QPushButton *bpmUpButton;
    QPushButton *bpmDownButton;
    PitchDial *pitchDial; QLabel *pitchLabel;
    bool paused = false;
    QToolButton *inputButton; QMenu *inputMenu;
    QLabel *volumeMeter;
    int litLines = -1;
};