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

    while (true)
    {
        //safely receive the pitch of the current note from fft.cpp 
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                    { return noteHandOverReady; });
            note = sharedNote;
            noteHandOverReady = false;
        }

        double currentTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - START).count();

        if (transcriptionPaused.load())
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
                sharedRealTimeList = realTimeList;
                getBMPReady = true;
            }
            cvBPM.notify_one();
            previousNote = note;
        }
    }
}