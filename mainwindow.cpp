#include "pch/pch.h"
#include "mainwindow.h"
#include "layout.h"
#include "buttons.h"
#include "globals.h"

extern std::mutex mtx;
extern std::vector<std::pair<int, double>> BPMTimeList;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Hello Qt");

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
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);

    layout->addWidget(title);

    auto *buttons = new Buttons(this);
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

    stave = new StaveWidget(this);
    //stave->setMinimumHeight(style.staveWidgetHeight);
    layout->addWidget(stave);

    label = new QLabel("Hello World", central);
    layout->addWidget(label);

    showMaximized();
}

void MainWindow::updateFrequency(int note)
{
    label->setText(QString("Frequency: %1 Hz").arg(note));
    label->adjustSize();
    label->repaint();
    return;
}

void MainWindow::updateStave(std::vector<std::pair<int, double>> BPMTimeList)
{
    stave->setNote(BPMTimeList);
}