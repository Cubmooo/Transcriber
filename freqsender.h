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
    void newFreqRecived(int note);
    void staveChangeNeeded(std::vector<std::pair<int, double>> checkBPMTimeList);

private:
    QTimer *timer;
};