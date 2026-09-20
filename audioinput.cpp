#include "pch/pch.h"
#include "audioinput.h"
#include "globals.h"

static PaStream* openInputStream(int device, int hopSize)
{
    const PaDeviceInfo* info = Pa_GetDeviceInfo(device);
    if (!info)
        return nullptr;

    PaStreamParameters params;
    params.device = device;
    params.channelCount = 1;
    params.sampleFormat = paFloat32;
    params.suggestedLatency = info->defaultHighInputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    PaStream* newStream = nullptr;
    if (Pa_OpenStream(&newStream, &params, nullptr, SAMPLE_RATE, hopSize, paNoFlag, nullptr, nullptr) != paNoError)
        return nullptr;

    if (Pa_StartStream(newStream) != paNoError)
    {
        Pa_CloseStream(newStream);
        return nullptr;
    }
    return newStream;
}

int fetchInput() {
    // initialize with windows default microphone
    Pa_Initialize();

    std::vector<std::pair<int, std::string>> devices;
    for (int i = 0; i < Pa_GetDeviceCount(); i++)
    {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info && info->maxInputChannels > 0 && info->hostApi == Pa_GetDefaultHostApi())
            devices.emplace_back(i, info->name);
    }
    int currentDevice = Pa_GetDefaultInputDevice();
    {
        std::lock_guard<std::mutex> lock(bufferMtx);
        inputDevices = devices;
        requestedInputDevice = currentDevice;
    }

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

        int wantedDevice; 
        {
            std::lock_guard<std::mutex> lock(bufferMtx);
            std::rotate(window.begin(), window.begin() + HOP_SIZE, window.end());
            std::copy(chunk.begin(), chunk.end(), window.end() - HOP_SIZE);
            audioBuffer = window;
            audioBufferReady = true;
            wantedDevice = requestedInputDevice;
        }
    bufferCv.notify_one();

        if (wantedDevice != currentDevice)
        {
            PaStream* newStream = openInputStream(wantedDevice, HOP_SIZE);
            if (newStream)
            {
                Pa_StopStream(stream);
                Pa_CloseStream(stream);
                stream = newStream;
                currentDevice = wantedDevice;
            }
            else
            {
                std::lock_guard<std::mutex> lock(bufferMtx);
                requestedInputDevice = currentDevice;
            }
        }
    }

    // end audio stream
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}