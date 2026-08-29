#include "pch/pch.h"
#include "mainwindow.h"

extern std::mutex mtx;
extern std::vector<std::pair<int, double>> BPMTimeList;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showMaximized();
    setWindowTitle("Hello Qt");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);

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

    stave = new StaveWidget(this);
    layout->addWidget(stave);

    label = new QLabel("Hello World", central);
    layout->addWidget(label);
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