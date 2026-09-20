#pragma once
#include "pch/pch.h"

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 8192

const double PI = 3.14159265358979323846;

extern std::mutex mtx;
extern std::mutex bpmMtx;
extern std::condition_variable cv;
extern std::condition_variable cvBPM;
extern double sharedNote;
extern bool noteHandOverReady;
extern bool getBMPReady;
extern bool bpmReady;

extern std::mutex bufferMtx;
extern std::condition_variable bufferCv;
extern bool audioBufferReady;
extern std::vector<float> audioBuffer;

extern std::vector<float> sharedBuffer;
extern std::vector<std::pair<int, double>> sharedRealTimeList;
extern std::vector<std::pair<int, double>> BPMTimeList;
extern std::chrono::steady_clock::time_point START;
using TimeDuration = std::chrono::duration<int64_t, std::nano>;
extern QString lelandFamily;

extern bool transcriptionPaused;
extern int  clearData; 
extern double currentBPS;
extern std::vector<std::pair<int, std::string>> inputDevices;   // {PortAudio index, name}
extern int requestedInputDevice;
extern double inputRMS;