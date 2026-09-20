#pragma once
#include <QWidget>
 
class QPushButton;
 
class Buttons : public QWidget
{
    Q_OBJECT
 
public:
    explicit Buttons(QWidget *parent = nullptr);
 
signals:
    void pauseToggled(bool paused);
 
private:
    QPushButton *playPauseButton;
    bool paused = false;
};