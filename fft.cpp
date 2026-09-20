#include "pch/pch.h"
#include "fft.h"
#include "globals.h"

void FFT()
{
    const int NUM_HARMONICS = 5;
    const float MIN_FREQ = 50.0f;
    const float MAX_FREQ = 2000.0f;
    const double SILENCE_THRESHOLD = 0.002;
    const int REQUIRED_STABLE_FRAMES = 1;
    const int ZERO_PAD_FACTOR = 4;
    const int FFT_SIZE = BUFFER_SIZE * ZERO_PAD_FACTOR;
    int previousNote = 0;
    int candidateNote = 0;
    int stableCount = 0;

    while (true)
    {
        std::vector<float> localBuffer;
        {
            std::unique_lock<std::mutex> lock(bufferMtx);
            bufferCv.wait(lock, [] { return audioBufferReady; });
            localBuffer = audioBuffer;
            audioBufferReady = false;
        }
        /*calculate the  rms effectively "energy"
        in this context it is used as a measure of volume
        */
        double rms = 0.0;
        for (float sample : localBuffer)
            rms += sample * sample;
        rms = sqrt(rms / BUFFER_SIZE);

        //if rms is to low it is likely a rest
        //the pitch therefore doesn't have to be found
        if (rms < SILENCE_THRESHOLD)
        {
            std::unique_lock<std::mutex> lock(mtx);
            sharedNote = 0;
            noteHandOverReady = true;
            cv.notify_one();
            continue;
        }

        
        //initialise the fft compuatutation bins
        int numBins = FFT_SIZE / 2 + 1;
        double *in = fftw_alloc_real(FFT_SIZE);
        fftw_complex *out = fftw_alloc_complex(numBins);

        //fill fft bin with complex cast of audio input
        std::fill(in, in + FFT_SIZE, 0.0);
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            double w = 0.5 * (1 - cos(2 * PI * i / (BUFFER_SIZE - 1)));
            in[i] = static_cast<double>(localBuffer[i] * w);
        }

        // excute FFT
        fftw_plan plan = fftw_plan_dft_r2c_1d(FFT_SIZE, in, out, FFTW_ESTIMATE);
        fftw_execute(plan);

        //load output into bins of each frequency
        std::vector<double> mag(numBins);
        for (int i = 0; i < numBins; i++)
            mag[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);

        int minBin = (int)(MIN_FREQ * FFT_SIZE / SAMPLE_RATE);
        int maxBin = std::min((int)(MAX_FREQ * FFT_SIZE / SAMPLE_RATE), numBins / NUM_HARMONICS);

        // apply harmonic prodcut spectrum to find fundamental pitch
        std::vector<double> hps(maxBin, 0.0);
        for (int i = minBin; i < maxBin; i++)
        {
            double product = mag[i];
            for (int h = 2; h <= NUM_HARMONICS; h++)
            {
                int idx = i * h;
                if (idx < numBins)
                    product *= mag[idx];
                else
                    product *= 1e-9;
            }
            hps[i] = product;
        }

        int peakBin = minBin;
        for (int i = minBin + 1; i < maxBin; i++)
            if (hps[i] > hps[peakBin])
                peakBin = i;

        int halfBin = peakBin / 2;
        if (halfBin >= minBin && hps[halfBin] > 0.0 && hps[peakBin] > 0.0)
        {
            const double OCTAVE_BIAS = 0.4;
            if (hps[halfBin] >= hps[peakBin] * OCTAVE_BIAS)
                peakBin = halfBin;
        }
        
        double interpolatedBin = (double)peakBin;
        if (peakBin > minBin && peakBin < maxBin - 1)
        {
            double alpha = hps[peakBin - 1];
            double beta  = hps[peakBin];
            double gamma = hps[peakBin + 1];
            double denom = (alpha - 2.0 * beta + gamma);
            if (std::abs(denom) > 1e-12)
                interpolatedBin += 0.5 * (alpha - gamma) / denom;
        }

        double freq = interpolatedBin * SAMPLE_RATE / FFT_SIZE;
        int note = std::round(57 + 12 * std::log2(freq / 440.0));

        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        if (note == candidateNote)
        {
            stableCount++;
        }
        else
        {
            candidateNote = note;
            stableCount = 0;
        }


        if (stableCount >= REQUIRED_STABLE_FRAMES)
        {
            {
                std::unique_lock<std::mutex> lock(mtx);
                sharedNote = note;
                noteHandOverReady = true;
            }

            cv.notify_one();

            previousNote = note;
        }
    }
}