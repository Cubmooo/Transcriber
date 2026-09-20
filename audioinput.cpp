#include "pch/pch.h"
#include "audioinput.h"
#include "globals.h"

int fetchInput() {
    // initialize with windows default microphone
    Pa_Initialize();


    std::vector<float> localCapture(BUFFER_SIZE);
    const int WINDOW_SIZE = BUFFER_SIZE;

    // hop size is after how many samples the frequency will actually be read
    const int HOP_SIZE = BUFFER_SIZE / 4;
    std::vector<float> window(WINDOW_SIZE, 0.0f);

    //inititaies listening
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
    // sends to ftt.cpp when enough samples have been completed to fill hop size
    bufferCv.notify_one();
    }

    // end audio stream
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}