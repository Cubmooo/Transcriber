#include "freqsender.h"

extern std::mutex bpmMtx;
extern bool bpmReady;
extern std::vector<std::pair<double, int>> BPMTimeList;

Sender::Sender()
{
    timer = new QTimer(this);

    connect(
        timer,
        &QTimer::timeout,
        this,
        &Sender::checkBPMTimeList
    );

    timer->start(50);
}

void Sender::checkBPMTimeList()
{
    double freq;

    {
        std::lock_guard<std::mutex> lock(bpmMtx);
        if(!bpmReady)
            return;

        freq = BPMTimeList.back().first;
        bpmReady = false;
    }

    emit newFreqRecived(freq);
}