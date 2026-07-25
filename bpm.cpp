#include "globals.h"

/*void magReggression(){
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
        int noPlayedNotes = sharedRealTimeList.size();
        /*std::cout << noPlayedNotes << "\n" << std::flush;*
        if (noPlayedNotes == 1){
            realTimeList.emplace_back(realTimeList.back().first, realTimeList.back().second);
            continue;
        }
        averageGap = (realTimeList.back().second + (noPlayedNotes - 2) * averageGap) / (noPlayedNotes - 1);
        float BPM = 1 / averageGap;
        
        double ratio = (BPM * realTimeList.back().second);
        if (ratio <= 0.01) {
            ratio = 0.01; 
        }

        beatLengthValue = -floor(log2(ratio));
        if (beatLengthValue < -4) beatLengthValue = -4;
        if (beatLengthValue > 4) beatLengthValue = 4;
        std::cout << beatLengthValue << "averageGap" << std::endl;
        float beatLength = pow(2, beatLengthValue);

        double time = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();

        std::lock_guard<std::mutex> lock(bpmMtx);
        bpmReady = true;

        cvBPM.notify_one();

        if (!BPMTimeList.empty()){
            BPMTimeList.emplace_back(realTimeList.back().first, BPMTimeList.back().second + previousBeatLength);
        }
        else{BPMTimeList.emplace_back(realTimeList.back().first, previousBeatLength);}
        previousBeatLength = beatLength;
        /*std::cout << "beatLengthValue:" << beatLengthValue << "\n" <<std::flush;
        std::cout << "BPMlist: " <<BPMTimeList.back().first << "," << BPMTimeList.back().second << "\n" << std::flush;*
        std::cout << BPMTimeList.back().first << "  -  "<<BPMTimeList.back().second << std::endl;
        std::cout << realTimeList.back().second << " realTimeList.back().second " << std::endl;
    }
}*/

void magReggression() {
    std::vector<std::pair<int, double>> realTimeList;
    std::cout << "sasdasd" << std::flush;
    while(true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvBPM.wait(lock, [] { return getBMPReady; });
            realTimeList = sharedRealTimeList;
            getBMPReady = false;
        }

        int totalNotes = realTimeList.size();
        if (totalNotes < 2) continue;

        int windowSize = std::min(totalNotes - 1, 8);
        double sumGaps = 0.0;
        for (int i = totalNotes - windowSize; i < totalNotes; ++i) {
            sumGaps += realTimeList[i].second;
        }
        double averageGap = sumGaps / windowSize;
        
        if (averageGap <= 0.001) continue;
        double estimatedBPM = 60.0 / averageGap;

        double absoluteTime = 0.0;
        for(const auto& pair : realTimeList) {
            absoluteTime += pair.second;
        }

        double absoluteBeat = absoluteTime * (estimatedBPM / 60.0);

        int currentNote = realTimeList.back().first;

        {
            std::lock_guard<std::mutex> lock(bpmMtx);
            BPMTimeList.emplace_back(currentNote, absoluteBeat);
            bpmReady = true;
        }

        std::cout << currentNote << "   currentNote   " << absoluteBeat << "   absoluteBeat" << std::endl;

        cvBPM.notify_one();
    }
}