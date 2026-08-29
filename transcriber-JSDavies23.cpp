#include "pch/pch.h"

#include "mainwindow.h"
#include "freqsender.h"
#include "audioinput.h"
#include "beatfinder.h"
#include "bpm.h"
#include "fft.h"

int main(int argc, char *argv[])
{
    // places each computational step and UI into separate threads to improve performance
    std::cout << "main";
    std::thread mic(fetchInput);
    std::thread FftThread(FFT);
    std::thread FftAnalyser(secondsToBeats);
    std::thread pulseFinder(magReggression);
    mic.detach();
    FftThread.detach();
    FftAnalyser.detach();
    pulseFinder.detach();

    //load leland font for displaying music notation
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Leland.otf");
    if (fontId != -1)
    {
        QString lelandFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    }

    QApplication app(argc, argv);
    MainWindow window;

    Sender sender;

    //connects the gui and audio processing threads
    QObject::connect(
        &sender,
        &Sender::newFreqRecived,
        &window,
        &MainWindow::updateFrequency);
    QObject::connect(
        &sender,
        &Sender::staveChangeNeeded,
        &window,
        &MainWindow::updateStave);

    window.show();
    return app.exec();
}