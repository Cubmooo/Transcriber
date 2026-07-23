#include "pch/pch.h"

#include "mainwindow.h"
#include "freqsender.h"
#include "audioinput.h"
#include "beatfinder.h"
#include "bpm.h"
#include "fft.h"

int main(int argc, char *argv[]){
    std::cout << "main";
    std::thread mic(fetchInput);
    std::thread FftThread(FFT);
    std::thread FftAnalyser(secondsToBeats);
    std::thread pulseFinder(magReggression);
    mic.detach();
    FftThread.detach();
    FftAnalyser.detach();
    pulseFinder.detach();
    
    QApplication app(argc, argv);
    MainWindow window;

    Sender sender;
    QObject::connect(
        &sender,
        &Sender::newFreqRecived,
        &window,
        &MainWindow::updateFrequency
    );

    window.show();
    return app.exec();
}
