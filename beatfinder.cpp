#include "globals.h"

int secondsToBeats() {
    int note;
    double lastTimeStamp = 0.0;
    std::vector<std::pair<int, double>> realTimeList;
    bool firstNote = true;
    int previousNote = 0;

    while(true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return noteHandOverReady; });
            note = sharedNote;
            noteHandOverReady = false;
        }
        
        double currentTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();
        
        if (firstNote) {
            realTimeList.emplace_back(note, 0.0);
            lastTimeStamp = currentTime;
            firstNote = false;
            continue;
        }
        if (note != previousNote){
            double timeDelta = currentTime - lastTimeStamp;
            realTimeList.emplace_back(note, timeDelta);
            lastTimeStamp = currentTime;
            {
                std::lock_guard<std::mutex> lock(mtx);
                sharedRealTimeList = realTimeList;
                getBMPReady = true; 
            }
            cvBPM.notify_one();
        }
        previousNote = note;
    }
}