#include "pch/pch.h"
#include "mainwindow.h"
#include <QString>
#include <QVBoxLayout>

extern std::mutex mtx;
extern std::vector<std::pair<int, int>> BPMTimeList;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showMaximized();
    setWindowTitle("Hello Qt");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);

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

void MainWindow::updateStave(int note)
{
    stave->setNote(note);
}