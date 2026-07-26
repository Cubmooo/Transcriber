#include "globals.h"

void magReggression(){
    std::vector<std::pair<int, double>> realTimeList;
    BPMTimeList.emplace_back(0.0,0);
    double averageGap = 0.0;
    float previousBeatLength = 0.0;
    int beatLengthValue = 0;

    while(true){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvBPM.wait(lock, [] { return getBMPReady; });
            realTimeList = sharedRealTimeList;
            getBMPReady = false;
        }
        int noPlayedNotes = realTimeList.size();
        if (noPlayedNotes == 1){
            continue;
        }
        int windowSize = std::min(noPlayedNotes - 1, 8);
        double sumGaps = 0.0;
        for (int i = noPlayedNotes - windowSize; i < noPlayedNotes; ++i) {
            sumGaps += realTimeList[i].second;
        }
        double averageGap = sumGaps / windowSize;

        /*if (averageGap <= 0.001) continue;
        double estimatedBPM = 60.0 / averageGap;*/

        double absoluteTime = 0.0;

        float BPS = 1 / averageGap;
        for(const auto& pair : realTimeList) {
            absoluteTime += pair.second;
        }

        std::lock_guard<std::mutex> lock(bpmMtx);
        bpmReady = true;
        cvBPM.notify_one();
        BPMTimeList.emplace_back(realTimeList.back().first, absoluteTime * BPS);

        std::cout << "(" << BPMTimeList.back().first << ", " << BPMTimeList.back().second << ")\n";
        /*std::cout << "beatLengthValue:" << beatLengthValue << "\n" <<std::flush;
        std::cout << "BPMlist: " <<BPMTimeList.back().first << "," << BPMTimeList.back().second << "\n" << std::flush;*/
    }
}