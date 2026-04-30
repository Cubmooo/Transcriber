#include "pch.h"
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 512

int FFT(){
    return 0;
}

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
    PaError err = Pa_Initialize();

    PaDeviceIndex inputDevice = Pa_GetDefaultInputDevice();
    std::cout << inputDevice;
    std::cout << "mic";
    if (inputDevice == paNoDevice) {
        fprintf(stderr, "no mic");
        Pa_Terminate();
        return -1;
    }

    int numDevices = Pa_GetDeviceCount();
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxInputChannels > 0) {
            printf("[%d] %s (inputs: %d)\n", i, info->name, info->maxInputChannels);
    }
}

     PaStream* stream;
    Pa_OpenDefaultStream(&stream,
        1,
        0, 
        paInt32,
        SAMPLE_RATE,
        BUFFER_SIZE,
        nullptr,
        nullptr
    );

    Pa_StartStream(stream);

    std::vector<int16_t> buffer(BUFFER_SIZE);

    while (true) {
        Pa_ReadStream(stream, buffer.data(), BUFFER_SIZE);
        
        for (int i = 0; i < BUFFER_SIZE; i++) {
            std::cout << buffer[i] << " ";
        }
        std::cout << "\n";
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "After Pa_Initialize" << std::endl;
    return 0;
}

int main(){
    std::cerr << "MAIN STARTED\n";
    std::cout << "Hello World!"<< std::endl;
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