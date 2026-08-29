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
    struct NoteGap
    {
        double gap;
    };

    std::vector<NoteGap> noteGaps;
    double currentGap = 0.0;

    int previousNote = realTimeList[0].first;

    for (int i = 1; i < noPlayedNotes; ++i)
    {
        currentGap += realTimeList[i].second;

        int currentNote = realTimeList[i].first;

        if (currentNote != previousNote)
        {
            noteGaps.push_back({currentGap});

            currentGap = 0.0;
            previousNote = currentNote;
        }
    }

    if (noteGaps.empty())
        return 1.0f;

    if (noteGaps.size() < 3)
    {
        double sum = 0.0;

        for (const auto& gap : noteGaps)
            sum += gap.gap;

        double averageGap = sum / noteGaps.size();

        if (averageGap <= 0.0)
            return 1.0f;

        return static_cast<float>(1.0 / averageGap);
    }

    int n = static_cast<int>(noteGaps.size());

    int recentWindow = std::min(8, n);

    double sumW  = 0.0;
    double sumX  = 0.0;
    double sumY  = 0.0;
    double sumXX = 0.0;
    double sumXY = 0.0;

    for (int i = 0; i < n; ++i)
    {
        double x = static_cast<double>(i);
        double y = noteGaps[i].gap;

        int distanceFromNewest = n - 1 - i;

        double weight;

        if (distanceFromNewest < recentWindow)
        {
            weight = 1.0;
        }
        else
        {
            weight = std::pow(
                0.5,
                static_cast<double>(
                    distanceFromNewest - recentWindow + 1
                ) / 8.0
            );
        }

        sumW  += weight;
        sumX  += weight * x;
        sumY  += weight * y;
        sumXX += weight * x * x;
        sumXY += weight * x * y;
    }

    double denominator =
        sumW * sumXX - sumX * sumX;

    if (std::abs(denominator) < 1e-10)
    {
        double sum = 0.0;

        for (int i = std::max(0, n - recentWindow); i < n; ++i)
            sum += noteGaps[i].gap;

        double averageGap =
            sum / std::min(n, recentWindow);

        return static_cast<float>(1.0 / averageGap);
    }

    double slope =
        (sumW * sumXY - sumX * sumY) /
        denominator;

    double intercept =
        (sumY - slope * sumX) / sumW;

    double x = static_cast<double>(n - 1);

    double estimatedGap =
        intercept + slope * x;


    if (!std::isfinite(estimatedGap) ||
        estimatedGap <= 0.0)
    {
        double sum = 0.0;

        int count = std::min(n, recentWindow);

        for (int i = n - count; i < n; ++i)
            sum += noteGaps[i].gap;

        estimatedGap = sum / count;
    }

    float BPS =
        static_cast<float>(1.0 / estimatedGap);

    while (BPS > 200.0f / 60.0f)
    {
        BPS /= 2.0f;
    }

    return BPS;
}