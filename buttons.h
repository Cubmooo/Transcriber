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
    void clearRequested();  
 
private:
    QPushButton *makeButton(const QString &text, int width);

    QPushButton *playPauseButton;
    QPushButton *clearButton;
    bool paused = false;
};