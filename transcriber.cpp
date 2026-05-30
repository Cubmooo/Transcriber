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
#include <Eigen/Dense>

#define SAMPLE_RATE 16000
//defines sample rate
#define BUFFER_SIZE 2048
#define BPM_BUFFER_SIZE 48000

const double PI = 3.14159265358979323846;

std::mutex mtx;
std::condition_variable cv;
double sharedFrequency;
bool freqHandOverReady = false;
bool getBMPReady = false;
std::condition_variable cvBPM;

std::vector<float> sharedBuffer(BUFFER_SIZE, 0.0f);
std::vector<float> sharedBPMBuffer(BPM_BUFFER_SIZE, 0.0f);

static const auto START = std::chrono::steady_clock::now();

using TimeDuration = std::chrono::duration<int64_t, std::nano>;
std::vector<std::pair<double, TimeDuration>> sharedRealTimeList;

int fetchInput(){
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

    while (true) {
        Pa_ReadStream(stream, sharedBuffer.data(), BUFFER_SIZE);
        Pa_ReadStream(stream, sharedBPMBuffer.data(), BPM_BUFFER_SIZE);
        /*std::cout << "3\n" << std::flush;*/
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
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
    std::vector<std::pair<double, std::chrono::duration<int64_t, std::nano>>> realTimeList;
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
        auto time = std::chrono::steady_clock::now() - START;
        realTimeList.emplace_back(freq, time);
        {
            std::lock_guard<std::mutex> lock(mtx);
            sharedRealTimeList = realTimeList;
            getBMPReady = true;
        }
        cvBPM.notify_one(); 
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(realTimeList.back().second);
        std::cout << realTimeList.back().first << "Hz\n" << std::flush;
        std::cout << ms.count() << "ms\n\n" << std::flush;
    }
}

void magReggression(){
    const int MIN_BPM = 30;
    const int MAX_BPM = 200;
    const int BPM_SAMPLES = 1000;
    const int MAX_WINDOW = 10;

    while (true){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvBPM.wait(lock, []{ return getBMPReady; });
            getBMPReady = false;
        }
        std::vector<std::pair<double,double>> BPMBuffer;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (sharedRealTimeList.empty()) continue;

            double latestSec =
                std::chrono::duration<double>(sharedRealTimeList.back().second).count();
            double cutoffSec = latestSec - MAX_WINDOW;

            for (auto& entry : sharedRealTimeList){
                double tSec = std::chrono::duration<double>(entry.second).count();
                if (tSec >= cutoffSec)
                    BPMBuffer.emplace_back(tSec, entry.first);
            }
        }
        const int N = static_cast<int>(BPMBuffer.size());

        double bestOmega = 0.0;
        double bestResidual = std::numeric_limits<double>::max();
        double bestA = 0.0, bestB = 0.0;

        Eigen::VectorXd y(N);
        for (int i = 0; i < N; ++i)
            y(i) = BPMBuffer[i].second;

        for (int step = 0; step <= BPM_SAMPLES; ++step){
            double omegaHz = (MIN_BPM + (MAX_BPM - MIN_BPM) * step / BPM_SAMPLES) / 60;
            double omega = 2.0 * PI * omegaHz;

            Eigen::MatrixXd Phi(N, 3);
            for (int i = 0; i < N; ++i){
                double t = BPMBuffer[i].first;
                Phi(i, 0) = std::sin(omega * t);
                Phi(i, 1) = std::cos(omega * t);
                Phi(i, 2) = 1.0; 
            }

            Eigen::VectorXd x =
                Phi.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(y);

            double residual = (Phi * x - y).squaredNorm();

            if (residual < bestResidual){
                bestResidual = residual;
                bestOmega    = omega;
                bestA        = x(0);
                bestB        = x(1);
            }
        }
        double bestFreqHz = (bestOmega / (2.0 * PI)) * 60;
        double phase = std::atan2(bestB, bestA);

        std::cout << "Regression freq: " << bestFreqHz << "phase:" << phase << " rad\n" << std::flush;
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