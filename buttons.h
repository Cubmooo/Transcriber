#pragma once
#include <QWidget>
 
class QPushButton;
 
class Buttons : public QWidget
{
    Q_OBJECT
 
public:
    explicit Buttons(QWidget *parent = nullptr);
    void setBPM(double bpm); 
 
signals:
    void pauseToggled(bool paused);
    void clearRequested();
    void bpmScaleRequested(double factor);   
 
private:
    QPushButton *makeButton(const QString &text, int width);

    QPushButton *playPauseButton;
    QPushButton *clearButton;
    QPushButton *bpmButton;
    QPushButton *bpmUpButton;
    QPushButton *bpmDownButton;  
    bool paused = false;
};