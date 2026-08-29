#include "freqsender.h"

extern std::mutex bpmMtx;
extern bool bpmReady;
extern std::vector<std::pair<int, double>> BPMTimeList;

Sender::Sender()
{
    timer = new QTimer(this);

    connect(
        timer,
        &QTimer::timeout,
        this,
        &Sender::checkBPMTimeList);

    timer->start(50);
}

void Sender::checkBPMTimeList()
{
    int note;
    std::vector<std::pair<int, double>> bpmCopy;

    {
        std::lock_guard<std::mutex> lock(bpmMtx);

        if (!bpmReady)
            return;

        note = BPMTimeList.back().first;
        bpmCopy = BPMTimeList;
        bpmReady = false;
    }

    emit newFreqRecived(note);
    emit staveChangeNeeded(bpmCopy);
}