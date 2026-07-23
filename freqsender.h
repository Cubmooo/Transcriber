#pragma once

#include <QObject>
#include <QTimer>

class Sender : public QObject
{
    Q_OBJECT

public:
    Sender();

public slots: 
    void checkBPMTimeList();

signals:
    void newFreqRecived(double freq);

private:
    QTimer *timer;
};