#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <portaudio.h>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <condition_variable>

#define SAMPLE_RATE 16000
//defines sample rate
#define BUFFER_SIZE 2048

const double PI = 3.14159265358979323846;

std::mutex mtx;
std::condition_variable cv;
double sharedFrequency;
bool freqHandOverReady = false;
bool getBMPReady = false;
std::condition_variable cvBPM;
std::vector<float> sharedBuffer(BUFFER_SIZE, 0.0f);

static const auto START = std::chrono::steady_clock::now();

using TimeDuration = std::chrono::duration<int64_t, std::nano>;
std::vector<std::pair<double, double>> sharedRealTimeList;

int fetchInput() {
    Pa_Initialize();

    PaStream* stream;
    Pa_OpenDefaultStream(&stream,
        1,
        0,
        paFloat32,
        SAMPLE_RATE,
        BUFFER_SIZE,
        nullptr,
        nullptr
    );

    Pa_StartStream(stream);

    static double prevEnergy = 0;

    while (true) {
        Pa_ReadStream(stream, sharedBuffer.data(), BUFFER_SIZE);

        auto now = std::chrono::steady_clock::now() - START;
        double timeSec = std::chrono::duration<double>(now).count();
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}

void FFT(){
    const int NUM_HARMONICS = 5;
    const float MIN_FREQ = 50.0f;
    const float MAX_FREQ = 2000.0f;

    while (true) {
        std::vector<float> localBuffer = sharedBuffer;

        int numBins = BUFFER_SIZE / 2 + 1;
        double* in = fftw_alloc_real(BUFFER_SIZE);
        fftw_complex* out = fftw_alloc_complex(numBins);

        for (int i = 0; i < BUFFER_SIZE; i++)
            in[i] = static_cast<double>(localBuffer[i]);

        fftw_plan plan = fftw_plan_dft_r2c_1d(BUFFER_SIZE, in, out, FFTW_ESTIMATE);
        fftw_execute(plan);

        std::vector<double> mag(numBins);
        for (int i = 0; i < numBins; i++)
            mag[i] = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);

        int minBin = (int)(MIN_FREQ * BUFFER_SIZE / SAMPLE_RATE);
        int maxBin = std::min((int)(MAX_FREQ * BUFFER_SIZE / SAMPLE_RATE), numBins / NUM_HARMONICS);

        std::vector<double> hps(maxBin, 1.0);
        for (int i = minBin; i < maxBin; i++)
            for (int h = 1; h <= NUM_HARMONICS; h++)
                if (i * h < numBins)
                    hps[i] *= mag[i * h];

        int peakBin = minBin;
        for (int i = minBin + 1; i < maxBin; i++)
            if (hps[i] > hps[peakBin]) peakBin = i;

        double freq = (double)peakBin * SAMPLE_RATE / BUFFER_SIZE;
        /*std::cout << "frequency: " << freq << " Hz\n" << std::flush;*/

        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        {
            std::unique_lock<std::mutex> lock(mtx);
            sharedFrequency = freq;
            freqHandOverReady = true;
        }
         cv.notify_one();

    }
}

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
        std::cout << realTimeList.back().first << "Hz\n" << realTimeList.back().second << "s\n\n" << std::flush;
    }
}

void magReggression(){
    std::vector<std::pair<double, double>> realTimeList;
    std::vector<std::pair<double, int>> BPMTimeList;
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

        if (!BPMTimeList.empty()){
            BPMTimeList.emplace_back(realTimeList.back().first, BPMTimeList.back().second + previousBeatLength);
        }
        else{BPMTimeList.emplace_back(realTimeList.back().first, previousBeatLength);}
        previousBeatLength = beatLength;
        std::cout << "beatLengthValue:" << beatLengthValue << "\n" <<std::flush;
        std::cout << "BPMlist: " <<BPMTimeList.back().first << "," << BPMTimeList.back().second << "\n" << std::flush;
    }
}   

void InitialiseUI(){
    return;
}

void UIButtons(){
    return;
}

int main(){
    std::cout << START.time_since_epoch().count() << std::flush;
    std::cout << "main";
    std::thread mic(fetchInput);
    std::thread FftThread(FFT);
    std::thread FftAnalyser(secondsToBeats);
    std::thread pulseFinder(magReggression);
    std::thread UI(InitialiseUI);
    std::thread UIInteraction(UIButtons);
    

    mic.join();
    FftThread.join();
    FftAnalyser.join();
    UI.join();
    UIInteraction.join();
    pulseFinder.join();

    return 0;
}