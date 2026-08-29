#include "pch/pch.h"
#include "audioinput.h"
#include "globals.h"

int fetchInput() {
    // initialize with windows default microphone
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

    // constantly write microphone input into the shared buffer
    while (true) {
        Pa_ReadStream(stream, sharedBuffer.data(), BUFFER_SIZE);

        auto now = std::chrono::steady_clock::now() - START;
        double timeSec = std::chrono::duration<double>(now).count();
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}