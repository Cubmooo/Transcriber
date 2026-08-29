#include "globals.h"
#include "bpm.h"

void magReggression()
{
    std::vector<std::pair<int, double>> realTimeList;
    double beat = 0.0;

    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvBPM.wait(lock, []
                       { return getBMPReady; });
            realTimeList = sharedRealTimeList;
            getBMPReady = false;
        }

        int noPlayedNotes = realTimeList.size();
        if (noPlayedNotes > 1){
            beat +=  realTimeList.back().second * findBPS(noPlayedNotes, realTimeList);
        }
        BPMTimeList.emplace_back(0, 0.0)
        {
            std::lock_guard<std::mutex> lock(bpmMtx);
            bpmReady = true;
            cvBPM.notify_one();
            if (!BPMTimeList.empty()){
                if (BPMTimeList.back().first != realTimeList.back().first){
                    BPMTimeList.emplace_back(realTimeList.back().first, beat);
                }
            }
        }
        std::cout << "(" << BPMTimeList.back().first << ", " << BPMTimeList.back().second << ")\n";
    }
}

float findBPS(int noPlayedNotes, std::vector<std::pair<int, double>>& realTimeList){
    int windowSize = std::min(noPlayedNotes - 1, 8);
    double sumGaps = 0.0;
    for (int i = noPlayedNotes - windowSize; i < noPlayedNotes; ++i){
        sumGaps += realTimeList[i].second;
    }
    double averageGap = sumGaps / windowSize;
    float BPS = 1 / averageGap;
    return BPS;
}