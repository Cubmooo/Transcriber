#include "audioinput.h"
#include "globals.h"

#include <portaudio.h>
#include <chrono>

int fetchInput() {
    std::cout << "fetchInputStart" << std::endl;
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