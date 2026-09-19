#include "pch/pch.h"
#include "audioinput.h"
#include "globals.h"

int fetchInput() {
    // initialize with windows default microphone
    Pa_Initialize();

    std::vector<float> localCapture(BUFFER_SIZE);
    const int WINDOW_SIZE = BUFFER_SIZE;
    const int HOP_SIZE = BUFFER_SIZE / 2;
    std::vector<float> window(WINDOW_SIZE, 0.0f);

    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, HOP_SIZE, nullptr, nullptr);
    Pa_StartStream(stream);

    // constantly write microphone input into the shared buffer
    while (true) {
        std::vector<float> chunk(HOP_SIZE);
        Pa_ReadStream(stream, chunk.data(), HOP_SIZE);

        {
            std::lock_guard<std::mutex> lock(bufferMtx);
            std::rotate(window.begin(), window.begin() + HOP_SIZE, window.end());
            std::copy(chunk.begin(), chunk.end(), window.end() - HOP_SIZE);
            audioBuffer = window;
            audioBufferReady = true;
        }
    bufferCv.notify_one();
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}