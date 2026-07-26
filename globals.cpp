#include "globals.h"


std::mutex mtx;
std::mutex bpmMtx;
std::condition_variable cv;
std::condition_variable cvBPM;

double sharedNote = 0;
bool noteHandOverReady = false;
bool getBMPReady = false;
bool bpmReady = false;
std::vector<float> sharedBuffer(BUFFER_SIZE, 0.0f);
std::vector<std::pair<int,double>> sharedRealTimeList;
std::vector<std::pair<int,double>> BPMTimeList;

const std::chrono::steady_clock::time_point START = std::chrono::steady_clock::now();
using TimeDuration = std::chrono::duration<int64_t, std::nano>;

QString lelandFamily;