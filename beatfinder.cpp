#include "globals.h"

int secondsToBeats(){
    double freq;
    double lastTimeStamp = 0.0;
    std::vector<std::pair<double, double>> realTimeList;

    while(true){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return freqHandOverReady; });
            freq = sharedFrequency;
            /*quantFreq = std::round(sharedFrequency / 5.0) * 5.0;*/
            freqHandOverReady = false;
        }
        if (realTimeList.empty()){} 
        else if(realTimeList.back().first != freq){} 
        else{continue;}
        
        double time = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();
        realTimeList.emplace_back(freq, time - lastTimeStamp);
        lastTimeStamp = time;
        {
            std::lock_guard<std::mutex> lock(mtx);
            sharedRealTimeList = realTimeList;
            getBMPReady = true; 
        }
        cvBPM.notify_one();
        /*std::cout << realTimeList.back().first << "Hz\n" << realTimeList.back().second << "s\n\n" << std::flush;*/
    }
}