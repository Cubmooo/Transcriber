#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <portaudio.h>

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 512

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
    while (true) {
        std::vector<float> localBuffer = sharedBuffer;
        double* in = fftw_alloc_real(BUFFER_SIZE);
        fftw_complex* out = fftw_alloc_complex(BUFFER_SIZE / 2 + 1);

        for (int i = 0; i < BUFFER_SIZE; i++) {
            in[i] = static_cast<double>(localBuffer[i]);
        }

        fftw_plan plan = fftw_plan_dft_r2c_1d(BUFFER_SIZE, in, out, FFTW_ESTIMATE);
        fftw_execute(plan);

        double maxMag = 0;
        int maxBin = 0;
        for (int i = 0; i < BUFFER_SIZE / 2 + 1; i++) {
            double mag = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
            if (mag > maxMag) {
                maxMag = mag;
                maxBin = i;
            }
        }

        double freq = maxBin * (double)SAMPLE_RATE / BUFFER_SIZE;
        std::cout << "frequency: " << freq << " Hz " << std::flush;;

        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
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