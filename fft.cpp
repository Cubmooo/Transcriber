#include "fft.h"
#include "globals.h"
#include <fftw3.h>

void FFT(){
    const int NUM_HARMONICS = 5;
    const float MIN_FREQ = 50.0f;
    const float MAX_FREQ = 2000.0f;
    const double SILENCE_THRESHOLD = 0.002;

    while (true) {
        double rms = 0.0;
        std::vector<float> localBuffer = sharedBuffer;
        for (float sample : localBuffer)
            rms += sample * sample;
        rms = sqrt(rms / BUFFER_SIZE);

        if (rms < SILENCE_THRESHOLD) {
            std::unique_lock<std::mutex> lock(mtx);
            sharedNote = 0;
            noteHandOverReady = true;
            cv.notify_one();
            continue;
        }

        int numBins = BUFFER_SIZE / 2 + 1;
        double* in = fftw_alloc_real(BUFFER_SIZE);
        fftw_complex* out = fftw_alloc_complex(numBins);

        for (int i = 0; i < BUFFER_SIZE; i++){
            double w = 0.5 * (1 - cos(2 * PI * i / (BUFFER_SIZE - 1)));
            in[i] = static_cast<double>(localBuffer[i] * w);
        }

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
                if (i * h < numBins){
                    hps[i] = mag[i];
                    for (int h = 2; h <= NUM_HARMONICS; h++)
                        hps[i] += mag[i*h];
                }

        int peakBin = minBin;
        for (int i = minBin + 1; i < maxBin; i++)
            if (hps[i] > hps[peakBin]) peakBin = i;

        double freq = (double)peakBin * SAMPLE_RATE / BUFFER_SIZE;
        /*std::cout << "frequency: " << freq << " Hz\n" << std::flush*/
        int note = std::round(57 + 12 * std::log2(freq / 440.0));

        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        {
            std::unique_lock<std::mutex> lock(mtx);
            sharedNote = note;
            noteHandOverReady = true;
        }
         cv.notify_one();

    }
}