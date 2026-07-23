#include "globals.h"

void magReggression(){
    std::vector<std::pair<double, double>> realTimeList;
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
        /*std::cout << noPlayedNotes << "\n" << std::flush;*/
        if (noPlayedNotes == 1){
            realTimeList.emplace_back(realTimeList.back().first, realTimeList.back().second);
            continue;
        }
        averageGap = (realTimeList.back().second + (noPlayedNotes - 2) * averageGap) / (noPlayedNotes - 1);
        float BPM = 1 / averageGap;
        
        beatLengthValue = - floor( log2 (BPM * 0.75 * realTimeList.back().second));
        float beatLength = pow(2, beatLengthValue);

        double time = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();

        std::lock_guard<std::mutex> lock(bpmMtx);
        BPMTimeList.emplace_back(BPM, time);
        bpmReady = true;

        cvBPM.notify_one();

        if (!BPMTimeList.empty()){
            BPMTimeList.emplace_back(realTimeList.back().first, BPMTimeList.back().second + previousBeatLength);
        }
        else{BPMTimeList.emplace_back(realTimeList.back().first, previousBeatLength);}
        previousBeatLength = beatLength;
        /*std::cout << "beatLengthValue:" << beatLengthValue << "\n" <<std::flush;
        std::cout << "BPMlist: " <<BPMTimeList.back().first << "," << BPMTimeList.back().second << "\n" << std::flush;*/
    }
}   