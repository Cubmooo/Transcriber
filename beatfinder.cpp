#include "globals.h"
#include "bpm.h"

int secondsToBeats()
{
    int note;
    double lastTimeStamp = 0.0;
    std::vector<std::pair<int, double>> realTimeList;
    bool firstNote = true;
    int previousNote = 0;
    double beatLength = 0.0;
    bool wasPaused = false;
    double pauseStart = 0.0;
    int storedClearedTime = 0; 

    while (true)
    {
        bool isPaused; 
        //safely receive the pitch of the current note from fft.cpp 
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (clearData != storedClearedTime)
            {
                storedClearedTime = clearData;
                realTimeList.clear();
                firstNote = true;
                previousNote = 0;
                lastTimeStamp = 0.0;
                wasPaused = false;
                START = std::chrono::steady_clock::now();
            }
            cv.wait(lock, [] { return noteHandOverReady; });
            note = sharedNote;
            noteHandOverReady = false;
            isPaused = transcriptionPaused;  
        }

        double currentTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();

        if (isPaused)
        {
            if (!wasPaused){
                pauseStart = currentTime;
                wasPaused = true;
            }
            continue;
        }
        if (wasPaused){
            lastTimeStamp += currentTime - pauseStart;
            wasPaused = false;
        }

        double timeDelta = currentTime - lastTimeStamp;

        //needed to avoid comparing to the previous note when previous note doesn't exist 
        if (firstNote){
            realTimeList.emplace_back(note, 0.0);
            lastTimeStamp = currentTime;
            firstNote = false;
            continue;
        }

        int noPlayedNotes = realTimeList.size();
        beatLength = 1/findBPS(noPlayedNotes, realTimeList);

        if ((note != previousNote) || (timeDelta >= beatLength)){
            realTimeList.emplace_back(note, timeDelta);
            lastTimeStamp = currentTime;
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (clearData != storedClearedTime) continue;
                sharedRealTimeList = realTimeList;
                getBMPReady = true;
            }
            cvBPM.notify_one();
            previousNote = note;
        }
    }
}