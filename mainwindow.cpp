#include "pch/pch.h"
#include "mainwindow.h"
#include "layout.h"
#include "buttons.h"
#include "globals.h"
#include <QTimer>

extern std::mutex mtx;
extern std::vector<std::pair<int, double>> BPMTimeList;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Transcriber");

    //style = DefineLayoutConstants(height(), width(), 100);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setSpacing(0);

    auto *title = new QLabel("Transcriber");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "background-color: #2596BE;"
        "color: black;"
    );

    QFont titleFont;
    titleFont.setPointSize(42);
    titleFont.setBold(true);
    title->setFixedHeight(120);
    title->setFont(titleFont);

    layout->addWidget(title);

    buttons = new Buttons(this);
    layout->addWidget(buttons);
    connect(buttons, &Buttons::pauseToggled, this, [](bool paused)
    {
        std::lock_guard<std::mutex> lock(mtx);
        transcriptionPaused = paused;
    });

    connect(buttons, &Buttons::clearRequested, this, [this]()
    {
        {
            std::scoped_lock lock(mtx, bpmMtx);
            sharedRealTimeList.clear();
            getBMPReady = false;
            clearData++;
            BPMTimeList.clear();
            BPMTimeList.emplace_back(0, 0.0);
            bpmReady = false;
        }
        updateStave({});
    });

    connect(buttons, &Buttons::bpmScaleRequested, this, [this](double factor)
    {
        tempoScale *= factor;

        std::vector<std::pair<int, double>> current;
        {
            std::lock_guard<std::mutex> lock(bpmMtx);
            current = BPMTimeList;
        }
        if (current.size() <= 1) current.clear();
        updateStave(current);
    });

    connect(buttons, &Buttons::inputDeviceSelected, this, [](int device)
    {
        std::lock_guard<std::mutex> lock(bufferMtx);
        requestedInputDevice = device;
    });

    auto *deviceTimer = new QTimer(this);
    connect(deviceTimer, &QTimer::timeout, this, [this]()
    {
        std::vector<std::pair<int, std::string>> devices;
        int selected;
        {
            std::lock_guard<std::mutex> lock(bufferMtx);
            devices = inputDevices;
            selected = requestedInputDevice;
        }
        buttons->setInputDevices(devices, selected);
    });
    deviceTimer->start(250);

    auto *volumeTimer = new QTimer(this);
    connect(volumeTimer, &QTimer::timeout, this, [this]()
    {
        double rms;
        double freq;
        {
            std::lock_guard<std::mutex> lock(bufferMtx);
            rms = inputRMS;
            freq = inputFreq;
        }
        buttons->setVolume(rms);
        buttons->setPitch(freq);
    });
    volumeTimer->start(50);

    stave = new StaveWidget(this);
    //stave->setMinimumHeight(style.staveWidgetHeight);
    layout->addWidget(stave);
    showMaximized();
}

void MainWindow::updateStave(std::vector<std::pair<int, double>> BPMTimeList)
{
    double bps;{
        std::lock_guard<std::mutex> lock(bpmMtx);
        bps = currentBPS;
    }

    for (auto &n : BPMTimeList)
        n.second *= tempoScale;
    stave->setNote(BPMTimeList);
    buttons->setBPM(60.0 * bps * tempoScale);
}