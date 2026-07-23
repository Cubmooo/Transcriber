#include "globals.h"

int secondsToBeats(){
    int note;
    double lastTimeStamp = 0.0;
    std::vector<std::pair<int, double>> realTimeList;

    while(true){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return noteHandOverReady; });
            note = sharedNote;
            /*quantFreq = std::round(sharedFrequency / 5.0) * 5.0;*/
            noteHandOverReady = false;
        }
        if (realTimeList.empty()){} 
        else if(realTimeList.back().first != note){} 
        else{continue;}
        
        double time = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();
        realTimeList.emplace_back(note, time - lastTimeStamp);
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