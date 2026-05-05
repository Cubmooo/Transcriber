#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <portaudio.h>
#include <chrono>
#include <algorithm> 

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 4096

std::vector<float> sharedBuffer(BUFFER_SIZE, 0.0f);

int secondsToBeats(){
    return 0;
}

void InitialiseUI(){
    return;
}

void UIButtons(){
    return;
}

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
        std::cout << "3\n" << std::flush;
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
        std::cout << "frequency: " << freq << " Hz\n" << std::flush;

        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(){
    std::cout << "main";
    std::thread mic(fetchInput);
    std::thread FftThread(FFT);
    std::thread FftAnalyser(secondsToBeats);
    std::thread UI(InitialiseUI);
    std::thread UIInteraction(UIButtons);

    mic.join();
    FftThread.join();
    FftAnalyser.join();
    UI.join();
    UIInteraction.join();

    return 0;
}