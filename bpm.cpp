#include "globals.h"
#include "bpm.h"

void magReggression()
{
    std::vector<std::pair<int, double>> realTimeList;
    double beat = 0.0;
    double previousNoteLength = 0;
    BPMTimeList.emplace_back(0, 0.0);
    int previousNote = -1;

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
        int currentNote = realTimeList.back().first;
        if (noPlayedNotes > 1){
            if (currentNote == previousNote){beat += 1.0;}
            else{
                double noteLength = (realTimeList.back().second) * findBPS(noPlayedNotes, realTimeList);
                previousNoteLength = noteLength;
                beat +=  std::round(noteLength * 4.0) / 4.0;
            }
        }
        previousNote = currentNote;
        
        {
            std::lock_guard<std::mutex> lock(bpmMtx);
            bpmReady = true;
            cvBPM.notify_one();
            if (!BPMTimeList.empty()){
                BPMTimeList.emplace_back(realTimeList.back().first, beat);
            }
        }
        std::cout << "bps: " << findBPS(noPlayedNotes, realTimeList) << std::endl;
        std::cout << "(" << BPMTimeList.back().first << ", " << BPMTimeList.back().second << ")\n";
    }
}

float findBPS(int noPlayedNotes,
              std::vector<std::pair<int, double>>& realTimeList)
{
    if (noPlayedNotes < 2 || realTimeList.size() < 2)
        return 1.0f;

    std::vector<double> gaps;

    double currentGap = 0.0;
    int previousNote = realTimeList[0].first;

    for (int i = 1; i < noPlayedNotes; ++i)
    {
        currentGap += realTimeList[i].second;

        int currentNote = realTimeList[i].first;

        if (currentNote != previousNote)
        {
            if (currentGap > 0.05)
                gaps.push_back(currentGap);

            currentGap = 0.0;
            previousNote = currentNote;
        }
    }

    if (gaps.empty())
        return 1.0f;

    constexpr int WINDOW = 12;

    int start = std::max(0,
                         static_cast<int>(gaps.size()) - WINDOW);

    std::vector<double> recentGaps;

    for (int i = start; i < static_cast<int>(gaps.size()); ++i)
        recentGaps.push_back(gaps[i]);

    std::sort(recentGaps.begin(), recentGaps.end());

    double medianGap;

    int count = static_cast<int>(recentGaps.size());

    if (count % 2 == 0)
    {
        medianGap =
            (recentGaps[count / 2 - 1] +
             recentGaps[count / 2]) / 2.0;
    }
    else
    {
        medianGap = recentGaps[count / 2];
    }

    if (medianGap <= 0.0)
        return 1.0f;

    static double previousBPS = 100.0 / 60.0;

    double rawBPS = 1.0 / medianGap;

    double candidates[] =
    {
        rawBPS / 4.0,
        rawBPS / 2.0,
        rawBPS,
        rawBPS * 2.0,
        rawBPS * 4.0
    };

    double bestBPS = candidates[0];
    double bestDistance =
        std::abs(std::log(candidates[0] / previousBPS));

    for (double candidate : candidates)
    {
        if (candidate <= 0.0)
            continue;

        double distance =
            std::abs(std::log(candidate / previousBPS));

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestBPS = candidate;
        }
    }

    while (bestBPS < 70.0 / 60.0)
    {
        double doubled = bestBPS * 2.0;

        if (doubled <= 160.0 / 60.0)
            bestBPS = doubled;
        else
            break;
    }

    while (bestBPS > 140.0 / 60.0)
    {
        double halved = bestBPS / 2.0;

        if (halved >= 60.0 / 60.0)
            bestBPS = halved;
        else
            break;
    }

    bestBPS =
        std::clamp(bestBPS,
                   60.0 / 60.0,
                   160.0 / 60.0);

    constexpr double MAX_CHANGE = 0.02;

    double maxIncrease =
        previousBPS * (1.0 + MAX_CHANGE);

    double maxDecrease =
        previousBPS * (1.0 - MAX_CHANGE);

    if (bestBPS > maxIncrease)
        bestBPS = maxIncrease;

    if (bestBPS < maxDecrease)
        bestBPS = maxDecrease;
    previousBPS = bestBPS;

    return static_cast<float>(bestBPS);
}